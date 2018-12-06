#include "stdafx.h"

#include "AlarmManager.h"
#include <QDebug>

AlarmManager::AlarmManager(QObject* parent)
    : QObject(parent)
{
}

AlarmManager::~AlarmManager()
{
}

void AlarmManager::bindMainWindow(MainWindow* mainwindow)
{
    m_mainWindow = mainwindow;
    connect(this, &AlarmManager::showCio100ToUi, mainwindow, &MainWindow::uiShowCio100);
    connect(this, &AlarmManager::showAlarmLightToUi, mainwindow, [=](int cl) { mainwindow->uiShowAlarmLight((AlarmColor)cl); }); // 将状态灯显示到UI
    // 关闭软件时将报警清除-修改：除非手工重置，否则不清除报警
    //connect(mainwindow, MainWindow::closeWindow，this, AlarmManager::clearAllAlarm );
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