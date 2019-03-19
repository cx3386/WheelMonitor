#include "stdafx.h"

#include "AlarmManager.h"
#include <QDebug>
#include <QMessageBox>

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
	//��PLC��CIO100�����пر���
	connect(this, &AlarmManager::hardAlarmToPlc, plc, &Plc::sendAlarmToControlCenter);
	//����PLC���豸���ϣ����ߡ����������ϣ��ź�
	// �޸ģ����ߡ�����������ֱ��LOG�����������ϻ���ʾ��UI��PLC DOCK��
	//����PLC�ĵ��ֱ����ź�
	connect(plc->handleSensorDevice[0], &HandleSensorDevice::wheelFallOff, this, [=](int code) {
		onHardwareAlarm(HardwareAlarmEvent::Outer_Alarm);
		QMessageBox::warning(nullptr, QStringLiteral("����"), QStringLiteral("�������䣡\n���ڹ���ҳ���а�����ʱ��鿴ǰ2��¼��\n����ʱ��:%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")), QStringLiteral("ȷ��"), QStringLiteral("ȡ��"));
		qCritical() << "wheel fall off! code" << code;  });
	connect(plc->handleSensorDevice[1], &HandleSensorDevice::wheelFallOff, this, [=](int code) {
		onHardwareAlarm(HardwareAlarmEvent::Inner_Alarm);
		QMessageBox::warning(nullptr, QStringLiteral("����"), QStringLiteral("�������䣡\n���ڹ���ҳ���а�����ʱ��鿴ǰ2��¼��\n����ʱ��:%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")), QStringLiteral("ȷ��"), QStringLiteral("ȡ��"));
		qCritical() << "wheel fall off! code" << code; });
}

void AlarmManager::bindImProc(ImageProcess* imp)
{
	// ���ܲ������ݿ��ź�
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
	connect(this, &AlarmManager::showAlarmLightToUi, mainwindow, [=](int cl) { mainwindow->uiShowAlarmLight((AlarmColor)cl); }); // ��״̬����ʾ��UI
}

void AlarmManager::bindPlayBack(PlayBackWidget* pb)
{
	// ֻͨ�����ϴ�������������
	connect(pb, &PlayBackWidget::clearAlarm, this, [=]() {
		m_mainWindow->uiShowAlarmLight(AlarmColor::Green);
		//���пصĻ�ɫ��������/�⣩����Ϊ��ɫ
		currentCio100 &= ~(1 << 4);
		currentCio100 &= ~(1 << 6);//����У��趨
		setHardwareAlarm(currentCio100);//plc�趨
	});
}

//! ��PLC�������ֱ���������ui���棨DOCK�;����ƣ�������PLC���пط�������
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
	//�������Ӳ������������/���֣�����Ὣ�����ϵĴ����Ϊ��ɫ
	if (currentCio100 == 0)
	{
		emit showAlarmLightToUi((int)AlarmColor::Green);
	}
	else
	{
		emit showAlarmLightToUi((int)AlarmColor::Red);
	}
}

void AlarmManager::setHardwareAlarm(int cio100)
{
	currentCio100 = cio100;
	emit showCio100ToUi(currentCio100);
	emit hardAlarmToPlc(currentCio100);
	//�������Ӳ������������/���֣�����Ὣ�����ϵĴ����Ϊ��ɫ
	if (currentCio100 == 0)
	{
		emit showAlarmLightToUi((int)AlarmColor::Green);
	}
	else
	{
		emit showAlarmLightToUi((int)AlarmColor::Red);
	}
}

//! ���ݳ��ֵľ����ȼ��򱨾������෢���ź�
void AlarmManager::checkoutWheelToDb(WheelDbInfo info)
{
	//���������Ч
	if (info.validmatch == 0) {
		int lastAlarmLevel = getPreCheckstate(info.num);
		switch (lastAlarmLevel) {
		case -2:
		case -1:
			// �ó�����һ�μ�����޼����
			info.alarmlevel = -2;
			emit showAlarmLightToUi((int)AlarmColor::Yellow);
			qCritical() << QStringLiteral("�������������μ��������Ч�����Ԥ��");
			break;
		case 0:
		case 1:
		case 2:
			//�ó�����һ�μ�����м����
			info.alarmlevel = -1;
			break;
		default:
			break;
		}
	}
	//���������Ч���Խ�����б����ж�
	else {
		double error = fabs(info.error);
		//��Ԥ����Χ��
		if (error <= info.warnRatio * 100) {
			info.alarmlevel = 0;
		}
		//Ԥ����Χ��������Χ֮��
		else if (error <= info.alarmRatio * 100) {
			int lastAlarmLevel = getPreCheckstate(info.num);
			switch (lastAlarmLevel) {
			case -2:
			case -1:
			case 0:
				//��һ�ν������������û����Ч���ݣ���Ԥ��
				info.alarmlevel = 1;
				emit showAlarmLightToUi((int)AlarmColor::Yellow);
				qCritical() << QStringLiteral("�����ֳ���Ԥ��ֵ��Ԥ��");
				break;
			case 1:
			case 2:
				//��һ�ν����Ԥ���򱨾������������γ���Ԥ��ֵ��������
				info.alarmlevel = 2;
				qCritical() << QStringLiteral("�������������μ���г���Ԥ��ֵ��������");
				break;
			default:
				break;
			}
		}
		//��������ֵ
		else {
			info.alarmlevel = 2;
			qCritical() << QStringLiteral("�����ֳ�������ֵ��������");
		}
	}
	//�жϽ��汨���ȼ���ϣ��˵ȼ�ָ������ʾ����ɫ���뱨������ֱ����ء���ɫֱ�����пط���Ԥ��
	if (info.alarmlevel == 2)
	{
		emit showAlarmLightToUi((int)AlarmColor::Red);
		if (info.i_o == 0)
		{
			onHardwareAlarm(HardwareAlarmEvent::Outer_Warn);
		}
		else if (info.i_o == 1)
		{
			onHardwareAlarm(HardwareAlarmEvent::Inner_Warn);
		}
	}
	/* ���ݱ����ȼ�����checkstate�Ĵ��� */
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
	//������ʾ
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
	query.prepare("INSERT INTO wheels (i_o,num,plate,calcspeed,refspeed,error,time,alarmlevel,checkstate,ocrsize,fragment,totalmatch,validmatch,speeds,videopath) VALUES(:i_o,:num,:plate,:calcspeed,:refspeed,:error,:time,:alarmlevel,:checkstate,:ocrsize,:fragment,:totalmatch,:validmatch,:speeds,:videopath);");
	query.bindValue(":i_o", QVariant(info.i_o));
	query.bindValue(":num", QVariant(info.num));
	//QImage to QByteArray
	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::WriteOnly);
	info.plate.save(&buffer, "JPG", 100); // writes image into ba in jpg format
	query.bindValue(":plate", ba, QSql::Binary);
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

//! ��ȡ��ǰ��ŵĳ���ǰһ�μ��Ľ��������ޣ�������ȷ
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