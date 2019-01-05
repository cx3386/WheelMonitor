#include "stdafx.h"

#include "AlarmManager.h"
#include <QDebug>

//using namespace std;
AlarmManager::AlarmManager(QObject* parent)
	: QObject(parent)
{
}

AlarmManager::~AlarmManager()
{
}

void AlarmManager::bindPLC(Plc* plc)
{
	m_plc = plc;
	//由PLC的CIO100发出中控报警
	connect(this, &AlarmManager::hardAlarmToPlc, plc, &Plc::sendAlarmToControlCenter);
	//接受PLC的设备故障（断线、传感器故障）信号
	// 修改：断线、传感器故障直接LOG，传感器故障还显示在UI的PLC DOCK中
	//接收PLC的掉轮报警信号
	connect(plc->handleSensorDevice[0], &HandleSensorDevice::wheelFallOff, this, [=](int pos) {
		onHardwareAlarm(HardwareAlarmEvent::Outer_Alarm);
		qCritical() << "wheel fall off! num:" << pos; });
	connect(plc->handleSensorDevice[1], &HandleSensorDevice::wheelFallOff, this, [=](int pos) {
		onHardwareAlarm(HardwareAlarmEvent::Inner_Alarm);
		qCritical() << "wheel fall off! num" << pos; });
}

void AlarmManager::bindImProc(ImageProcess* imp)
{
	// 接受插入数据库信号
	connect(imp, &ImageProcess::wheelNeedHandled, this, &AlarmManager::checkoutWheelToDb);
}

void AlarmManager::bindMainWindow(MainWindow* mainwindow)
{
	m_mainWindow = mainwindow;
	connect(this, &AlarmManager::showSpeed_o, mainwindow, &MainWindow::uiShowSpeed_o);
	connect(this, &AlarmManager::showNum_o, mainwindow, &MainWindow::uiShowNum_o);
	connect(this, &AlarmManager::showError_o, mainwindow, &MainWindow::uiShowError_o);
	connect(this, &AlarmManager::showSpeed_i, mainwindow, &MainWindow::uiShowSpeed_i);
	connect(this, &AlarmManager::showNum_i, mainwindow, &MainWindow::uiShowNum_i);
	connect(this, &AlarmManager::showError_i, mainwindow, &MainWindow::uiShowError_i);
	connect(this, &AlarmManager::showCio100ToUi, mainwindow, &MainWindow::uiShowCio100);
	connect(this, &AlarmManager::showAlarmLightToUi, mainwindow, [=](int cl) { mainwindow->uiShowAlarmLight((AlarmColor)cl); }); // 将状态灯显示到UI
	// 关闭软件时将报警清除-修改：除非手工重置，否则不清除报警
	//connect(mainwindow, MainWindow::closeWindow，this, AlarmManager::clearAllAlarm );
}

void AlarmManager::bindPlayBack(PlayBackWidget* pb)
{
	// 只通过故障处理界面清除报警
	if (m_mainWindow != nullptr) {
		connect(pb, &PlayBackWidget::clearAlarm, this, [=]() { m_mainWindow->uiShowAlarmLight(AlarmColor::Green); });
	}
	else {
		qFatal("no create playbackWidget.");
	}
}

//! 当PLC发出掉轮报警，更新ui界面（DOCK和警报灯），并由PLC向中控发出报警
void AlarmManager::onHardwareAlarm(HardwareAlarmEvent ev)
{
	switch (ev) {
	case HardwareAlarmEvent::Reset:
		currentCio100 = 0;
		break;
	case HardwareAlarmEvent::Outer_Warn:
		currentCio100 |= 1 << 4; //CIO 100.04 out warn
		break;
	case HardwareAlarmEvent::Outer_Alarm:
		currentCio100 |= 1 << 5; //CIO 100.05 out alarm
		break;
	case HardwareAlarmEvent::Inner_Warn:
		currentCio100 |= 1 << 6; //CIO 100.06 in warn
		break;
	case HardwareAlarmEvent::Inner_Alarm:
		currentCio100 |= 1 << 7; //CIO 100.07 in alarm
		break;
	default: // not do anything
		qDebug() << "overflow";
		break;
	}
	emit showCio100ToUi(currentCio100);
	emit hardAlarmToPlc(currentCio100);
	emit showAlarmLightToUi((int)AlarmColor::Red);
}

//! 根据车轮的警报等级向报警处理类发出信号
void AlarmManager::checkoutWheelToDb(WheelDbInfo info)
{
	//检测数据无效
	if (info.validmatch == 0) {
		int lastAlarmLevel = getPreCheckstate(info.num);
		switch (lastAlarmLevel) {
		case -2:
		case -1:
			// 该车轮上一次检测中无检测结果
			info.alarmlevel = -2;
			emit showAlarmLightToUi((int)AlarmColor::Yellow);
			qCritical() << QStringLiteral("本车轮连续两次检测中无有效结果，预警");
			break;
		case 0:
		case 1:
		case 2:
			//该车轮上一次检测中有检测结果
			info.alarmlevel = -1;
			break;
		default:
			break;
		}
	}
	//检测数据有效，对结果进行报警判断
	else {
		double error = fabs(info.error);
		//在预警范围内
		if (error <= info.warnRatio * 100) {
			info.alarmlevel = 0;
		}
		//预警范围到报警范围之间
		else if (error <= info.alarmRatio * 100) {
			int lastAlarmLevel = getPreCheckstate(info.num);
			switch (lastAlarmLevel) {
			case -2:
			case -1:
			case 0:
				//上一次结果正常，或者没有有效数据，则预警
				info.alarmlevel = 1;
				emit showAlarmLightToUi((int)AlarmColor::Yellow);
				qCritical() << QStringLiteral("本车轮超出预警值，预警");
				break;
			case 1:
			case 2:
				//上一次结果是预警或报警，则连续两次超出预警值，报警！
				info.alarmlevel = 2;
				emit showAlarmLightToUi((int)AlarmColor::Red);
				qCritical() << QStringLiteral("本车轮连续两次检测中超出预警值，报警！");
				break;
			default:
				break;
			}
		}
		//超出报警值
		else {
			info.alarmlevel = 2;
			emit showAlarmLightToUi((int)AlarmColor::Red);
			qCritical() << QStringLiteral("本车轮超出报警值，报警！");
		}
	}
	/* checkstate */
	switch (info.alarmlevel) {
	case -2:
	case 1:
	case 2:
		info.checkstate = NeedCheck;
		break;
	case -1:
	case 0:
		info.checkstate = NoNeedCheck;
		break;
	default:
		break;
	}
	//界面显示
	switch (info.i_o) {
	case 0:
		emit showSpeed_o(info.calcspeed);
		emit showNum_o(info.num);
		emit showError_o(info.error);
		break;
	case 1:
		emit showSpeed_i(info.calcspeed);
		emit showNum_i(info.num);
		emit showError_i(info.error);
		break;
	default:
		break;
	}
	if (!insertRecord(info))
		qDebug() << "insert record failed";
}

bool AlarmManager::insertRecord(WheelDbInfo info)
{
	QSqlQuery query;
	query.prepare("INSERT INTO wheels (i_o,num,calcspeed,refspeed,error,time,alarmlevel,checkstate,ocrsize,fragment,totalmatch,validmatch,speeds,videopath) VALUES(:i_o,:num,:calcspeed,:refspeed,:error,:time,:alarmlevel,:checkstate,:ocrsize,:fragment,:totalmatch,:validmatch,:speeds,:videopath);");
	query.bindValue(":i_o", QVariant(info.i_o));
	query.bindValue(":num", QVariant(info.num));
	query.bindValue(":calcspeed", QVariant(info.calcspeed));
	query.bindValue(":refspeed", QVariant(info.refspeed));
	query.bindValue(":error", QVariant(info.error));
	query.bindValue(":time", QVariant(info.time));
	query.bindValue(":alarmlevel", QVariant(info.alarmlevel));
	query.bindValue(":checkstate", QVariant(info.checkstate));
	query.bindValue(":ocrsize", QVariant(info.ocrsize));
	query.bindValue(":fragment", QVariant(info.fragment));
	query.bindValue(":totalmatch", QVariant(info.totalmatch));
	query.bindValue(":validmatch", QVariant(info.validmatch));
	query.bindValue(":speeds", QVariant(info.speeds));
	query.bindValue(":videopath", QVariant(info.videopath));
	if (!query.exec()) {
		auto text = query.lastError().text();
		qDebug() << text;
		return false;
	}
	return true;
}

//! 获取当前序号的车轮前一次检测的结果，如果无，返回正确
int AlarmManager::getPreCheckstate(QString num)
{
	if (num == OCR_MISS)
		return 0;
	QSqlQuery query;
	query.prepare("SELECT alarmlevel FROM wheels WHERE num=? ORDER BY id DESC");
	query.bindValue(0, QVariant(num));
	query.exec();
	int idx = query.record().indexOf("alarmlevel");
	if (query.first()) {
		int alarmlevel = query.value(idx).toInt();
		return alarmlevel;
	}
	return 0;
}