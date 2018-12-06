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
    void bindPLC(Plc* plc)
    {
        m_plc = plc;
        //��PLC��CIO100�����пر���
        connect(this, &AlarmManager::hardAlarmToPlc, plc, &Plc::onHardAlarm);
        //����PLC���豸���ϣ����ߡ����������ϣ��ź�
        // �޸ģ����ߡ�����������ֱ��LOG�����������ϻ���ʾ��UI��PLC DOCK��
        //����PLC�ĵ��ֱ����ź�
        connect(plc, &Plc::wheelFallOff, this, [=]() {
			//����������Ȧ����
			onHardwareAlarm(HardwareAlarmEvent::Outer_Alarm);
			qCritical() << "wheel fall off!"; });
    }

    void bindImProc(ImageProcess* imp)
    {
        //����ת�ټ��ġ�ת�ٹ������źţ�ֻ����UI�ĵƱ���
        connect(imp, &ImageProcess::setAlarmLight, this, [=](int cl) { emit showAlarmLightToUi(cl); });
    }
    void bindMainWindow(MainWindow* mainwindow);
    void bindPlayBack(PlayBackWidget* pb)
    {
        // ֻͨ�����ϴ�������������
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
    //! ���ֱ����ź�
    //void wheelFallOffAlarm(int id);
    void showCio100ToUi(int); //!< �пر����źţ�PLC CIO100����UI�е���ʾ connect to UI todo
    void hardAlarmToPlc(int); //!< �пر����źŷ�����PLC�н��д��� connect to PLC todo
    void showAlarmLightToUi(int); //!< ����UI����ı����Ƶ���ɫ connect to UI

private:
    Plc* m_plc;
    MainWindow* m_mainWindow;
    //PlayBackWidget* m_pb; //!< ���ϴ������
    int currentCio100;
};
