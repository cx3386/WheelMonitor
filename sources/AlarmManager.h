/*!
 * \file AlarmManager.h
 * \date 2018/11/26 3:00
 *
 * \author cx3386
 * Contact: cx3386@163.com
 * \par Copyright(c):
 * cx3386.
 * All Rights Reserved
 * \brief 管理所有的报警
 *
 * TODO: long description
 *
 * \note
 *
 * \version 1.0
 */
#pragma once

#include "AlarmEvent.h"
#include "Plc.h"
#include "imageprocess.h"
#include "mainwindow.h"
#include "playbackwidget.h"
#include <QObject>

class AlarmManager : public QObject {
	Q_OBJECT

public:
	AlarmManager(QObject* parent);
	~AlarmManager();
	// 使用前必须初始化
	void bindPLC(Plc* plc);
	void bindImProc(ImageProcess* imp);
	void bindMainWindow(MainWindow* mainwindow);
	void bindPlayBack(PlayBackWidget* pb);;

public slots:
	void onHardwareAlarm(HardwareAlarmEvent event);
	//void alarmWheel(int);

signals:
	//! 掉轮报警信号
	//void wheelFallOffAlarm(int id);
	void showCio100ToUi(int); //!< 中控报警信号（PLC CIO100）在UI中的显示 connect to UI todo
	void hardAlarmToPlc(int); //!< 中控报警信号发送至PLC中进行处理 connect to PLC todo
	void showAlarmLightToUi(int); //!< 设置UI界面的报警灯的颜色 connect to UI
	void showSpeed_i(double);
	void showError_i(double);
	void showNum_i(QString);
	void showSpeed_o(double);
	void showError_o(double);
	void showNum_o(QString);

private:
	Plc* m_plc;
	MainWindow* m_mainWindow;
	//PlayBackWidget* m_pb; //!< 故障处理界面
	int currentCio100;
	bool insertRecord(WheelDbInfo info);
	int getPreCheckstate(QString num);
private slots:
	void checkoutWheelToDb(WheelDbInfo info);
};
