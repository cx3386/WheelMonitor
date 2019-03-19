/**
 * \file HandleSensor.h
 *
 * \author cx3386
 * \date 十二月 2018
 *
 * 处理传感器信号，进行掉轮检测、设备故障判断和发出DZ in/out信号
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
	void input(int io); //!< 输入一个新的io信号
	void io2dev(int io); //!< 将输入的io信号压入devs中
	void checkTri(); //!< 用读取的cio数据检测是否满足触发条件（上升沿/下降沿）
	void newCkp(int ckpId); //!< expect begin时，开始一个新的ckp
	void checkoutCkp(int ckpId); //!< 在expect end时，结算ckp。注意：此ckpId为global id
	SensorDevice* dev;
private:
	int devId;
	void checkoutDev(); //在离开侧的expect end时，检测当前dev的坏掉的传感器和掉轮
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
	void triInt(int ckpId); //!< 至积分器,global ckpid
	void wheelFallOff(int); //!< 轮子脱落。参数,两位2进制。高位为内（1）外（0）圈，低位为0时为当前车轮，1为前1个车轮
	void sensorUpdate(int);
};
