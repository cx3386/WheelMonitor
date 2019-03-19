/**
 * \file HandleSensor.h
 *
 * \author cx3386
 * \date ʮ���� 2018
 *
 * ���������źţ����е��ּ�⡢�豸�����жϺͷ���DZ in/out�ź�
 */
#pragma once

#include <QObject>
#include "SensorDevice.h"

class HandleSensorDevice : public QObject
{
	Q_OBJECT

public:
	HandleSensorDevice(int _devId, QObject *parent);
	~HandleSensorDevice();
	void input(int io); //!< ����һ���µ�io�ź�
	void io2dev(int io); //!< �������io�ź�ѹ��devs��
	void checkTri(); //!< �ö�ȡ��cio���ݼ���Ƿ����㴥��������������/�½��أ�
	void newCkp(int ckpId); //!< expect beginʱ����ʼһ���µ�ckp
	void checkoutCkp(int ckpId); //!< ��expect endʱ������ckp��ע�⣺��ckpIdΪglobal id
	SensorDevice* dev;
private:
	int devId;
	void checkoutDev(); //���뿪���expect endʱ����⵱ǰdev�Ļ����Ĵ������͵���
	void emitDzSignal(int ckpId);
	void updateBrokenSensor();
	void updateGoodSensor();
	int getBreaks();
	int getBrokens();
	int getExpects();
	void updateSensorToUi();
signals:
	void dzIn();
	void dzOut();
	void triInt(int ckpId); //!< ��������,global ckpid
	void wheelFallOff(int); //!< �������䡣����,��λ2���ơ���λΪ�ڣ�1���⣨0��Ȧ����λΪ0ʱΪ��ǰ���֣�1Ϊǰ1������
	void sensorUpdate(int);
};
