#pragma once
#include <QObject>
#include "common.h"
#include "HandleSensorDevice.h"

class ConfigHelper;
class QSerialPort;
class SpeedIntegrator;
enum class HardwareAlarmEvent;

//! plc其他硬件设备相关类，包括传感器、plc io、plc ad
class Plc : public QObject {
	Q_OBJECT
public:
	//! 必须传入configure, const成员变量在初始化列表中初始化
	explicit Plc(const ConfigHelper* _configHelper, QObject* parent = nullptr);
	~Plc();
	// 用于mainwindow跨线程控制。通过信号槽机制，onXxx将在plcThread中执行
	void start(); //!< usrctrl
	void stop(); //!< usrctrl
	void connect(); //!< 通过串口连接到plc
	//! 获取台车速度(m/min)，线程安全的
	double getTruckSpeed(int devId = 0) const
	{
		QMutexLocker locker(&mutex);
		return truckSpeed * speedCompensationCoeff[devId];
	}
	static const int adInterval = 1000; //!< 读取ad（车速）的间隔，ms // 不应太频繁，应大于图像处理的时间7/25s，否则无意义
	HandleSensorDevice* handleSensorDevice[2];

private:
	const ConfigHelper* configHelper;
	SpeedIntegrator* speedIntegrator;
	mutable QMutex mutex;
	QSerialPort* plcSerialPort;
	QThread* refThread;
	double truckSpeed; //linear Velocity read from AD, unit:m/min
	// the rate between inner calc speed and measured speed, of which value is optimized by LS method (minimize the error)
	const double speedCompensationCoeff[2] = { 0.954, 0.802 }; // [out, in]
	bool bUsrCtrl = false;
	bool bConnected = false;
	/* 注意：单次读写的耗时大概在~300ms */
	static const int cio0Interval = 1000; //!<  读取cio0（传感器）的间隔，ms
	static const int serialport_timeout = 800;//响应超时时间ms，应小于读取间隔
	void writePLC(QByteArray plcData);
	QByteArray readPLC(QByteArray plcData);
	//PC与PLC的通信，采用HOSTLINK协议的C-MODE指令
	//e.g., cmd = '@00RR00020001', fcs code = '0x43', the full code = "@00RR0002000143*\r"
	QByteArray getFCSCode(QByteArray cmd) const; //!< 根据命令码获取FCS码，通常情况下不需要直接调用
	QByteArray getFullCode(QByteArray cmd) const;//!< 根据命令码获取完整指令码
	QByteArray genRRCode(int addr, int num) const; //!< 返回读取cio所需要的完整指令码
	QByteArray genWRCode(int addr, QByteArray data) const; //!< 返回写入cio所需要的完整指令码
	bool checkAnsCode(QByteArray ansCode) const; //!< 检查应答是否正确
	QByteArrayList getRRData(QByteArray ansCode) const; //!< 从应答中提取n个字的数据(hex)

private slots:
	void readSensorPeriodically();
	//! read truck reference speed from AD module. a cycle includes sending cmd and recv response, costs ~300ms
	void readSpeedPeriodically();

signals:
	//to UI
	void cio0Update(int cio);
	void connectError(int errorId);

	void truckSpeedReady();

	// to alarmManager
	/**
	 * \brief 台车(中轴)速度信号读取错误
	 * errcode 1: disconnect
	 * errcode 2: out of range(4~20ma)
	 * \param int errorCode
	 */
	void truckSpeedError(int);

public slots:
	void sendAlarmToControlCenter(int cio); //!< 向中控发出报警
};