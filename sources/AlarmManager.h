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
    void bindPLC(Plc* plc)
    {
        m_plc = plc;
        //由PLC的CIO100发出中控报警
        connect(this, &AlarmManager::hardAlarmToPlc, plc, &Plc::onHardAlarm);
        //接受PLC的设备故障（断线、传感器故障）信号
        // 修改：断线、传感器故障直接LOG，传感器故障还显示在UI的PLC DOCK中
        //接收PLC的掉轮报警信号
        connect(plc, &Plc::wheelFallOff, this, [=]() {
			//假设是最外圈掉轮
			onHardwareAlarm(HardwareAlarmEvent::Outer_Alarm);
			qCritical() << "wheel fall off!"; });
    }

    void bindImProc(ImageProcess* imp)
    {
        //接收转速检查的“转速过慢”信号，只发出UI的灯报警
        connect(imp, &ImageProcess::setAlarmLight, this, [=](int cl) { emit showAlarmLightToUi(cl); });
    }
    void bindMainWindow(MainWindow* mainwindow);
    void bindPlayBack(PlayBackWidget* pb)
    {
        // 只通过故障处理界面清除报警
        if (m_mainWindow != nullptr) {
            connect(pb, &PlayBackWidget::clearAlarm, this, [=]() { m_mainWindow->uiShowAlarmLight(AlarmColor::Green); });
        } else {
            qFatal("no create playbackWidget.");
        }
    };

public slots:
    void onHardwareAlarm(HardwareAlarmEvent event);
    //void alarmWheel(int);

signals:
    //! 掉轮报警信号
    //void wheelFallOffAlarm(int id);
    void showCio100ToUi(int); //!< 中控报警信号（PLC CIO100）在UI中的显示 connect to UI todo
    void hardAlarmToPlc(int); //!< 中控报警信号发送至PLC中进行处理 connect to PLC todo
    void showAlarmLightToUi(int); //!< 设置UI界面的报警灯的颜色 connect to UI

private:
    Plc* m_plc;
    MainWindow* m_mainWindow;
    //PlayBackWidget* m_pb; //!< 故障处理界面
    int currentCio100;
};
