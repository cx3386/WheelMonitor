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
    connect(this, &AlarmManager::showAlarmLightToUi, mainwindow, [=](int cl) { mainwindow->uiShowAlarmLight((AlarmColor)cl); }); // ��״̬����ʾ��UI
    // �ر����ʱ���������-�޸ģ������ֹ����ã������������
    //connect(mainwindow, MainWindow::closeWindow��this, AlarmManager::clearAllAlarm );
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
    emit showAlarmLightToUi((int)AlarmColor::Red);
}