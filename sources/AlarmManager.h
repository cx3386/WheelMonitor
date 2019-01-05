/*!
 * \file AlarmManager.h
 * \date 2018/11/26 3:00
 *
 * \author cx3386
 * Contact: cx3386@163.com
 * \par Copyright(c):
 * cx3386.
 * All Rights Reserved
 * \brief �������еı���
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
	// ʹ��ǰ�����ʼ��
	void bindPLC(Plc* plc);
	void bindImProc(ImageProcess* imp);
	void bindMainWindow(MainWindow* mainwindow);
	void bindPlayBack(PlayBackWidget* pb);;

public slots:
	void onHardwareAlarm(HardwareAlarmEvent event);
	//void alarmWheel(int);

signals:
	//! ���ֱ����ź�
	//void wheelFallOffAlarm(int id);
	void showCio100ToUi(int); //!< �пر����źţ�PLC CIO100����UI�е���ʾ connect to UI todo
	void hardAlarmToPlc(int); //!< �пر����źŷ�����PLC�н��д��� connect to PLC todo
	void showAlarmLightToUi(int); //!< ����UI����ı����Ƶ���ɫ connect to UI
	void showSpeed_i(double);
	void showError_i(double);
	void showNum_i(QString);
	void showSpeed_o(double);
	void showError_o(double);
	void showNum_o(QString);

private:
	Plc* m_plc;
	MainWindow* m_mainWindow;
	//PlayBackWidget* m_pb; //!< ���ϴ������
	int currentCio100;
	bool insertRecord(WheelDbInfo info);
	int getPreCheckstate(QString num);
private slots:
	void checkoutWheelToDb(WheelDbInfo info);
};
