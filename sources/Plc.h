#pragma once
#include <QObject>
#include "common.h"
#include "HandleSensorDevice.h"

class ConfigHelper;
class QSerialPort;
class SpeedIntegrator;
enum class HardwareAlarmEvent;

//! plc����Ӳ���豸����࣬������������plc io��plc ad
class Plc : public QObject {
	Q_OBJECT
public:
	//! ���봫��configure, const��Ա�����ڳ�ʼ���б��г�ʼ��
	explicit Plc(const ConfigHelper* _configHelper, QObject* parent = nullptr);
	~Plc();
	// ����mainwindow���߳̿��ơ�ͨ���źŲۻ��ƣ�onXxx����plcThread��ִ��
	void start(); //!< usrctrl
	void stop(); //!< usrctrl
	void connect(); //!< ͨ���������ӵ�plc
	//! ��ȡ̨���ٶ�(m/min)���̰߳�ȫ��
	double getTruckSpeed(int devId = 0) const
	{
		QMutexLocker locker(&mutex);
		return truckSpeed * speedCompensationCoeff[devId];
	}
	static const int adInterval = 1000; //!< ��ȡad�����٣��ļ����ms // ��Ӧ̫Ƶ����Ӧ����ͼ�����ʱ��7/25s������������
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
	/* ע�⣺���ζ�д�ĺ�ʱ�����~300ms */
	static const int cio0Interval = 1000; //!<  ��ȡcio0�����������ļ����ms
	static const int serialport_timeout = 800;//��Ӧ��ʱʱ��ms��ӦС�ڶ�ȡ���
	void writePLC(QByteArray plcData);
	QByteArray readPLC(QByteArray plcData);
	//PC��PLC��ͨ�ţ�����HOSTLINKЭ���C-MODEָ��
	//e.g., cmd = '@00RR00020001', fcs code = '0x43', the full code = "@00RR0002000143*\r"
	QByteArray getFCSCode(QByteArray cmd) const; //!< �����������ȡFCS�룬ͨ������²���Ҫֱ�ӵ���
	QByteArray getFullCode(QByteArray cmd) const;//!< �����������ȡ����ָ����
	QByteArray genRRCode(int addr, int num) const; //!< ���ض�ȡcio����Ҫ������ָ����
	QByteArray genWRCode(int addr, QByteArray data) const; //!< ����д��cio����Ҫ������ָ����
	bool checkAnsCode(QByteArray ansCode) const; //!< ���Ӧ���Ƿ���ȷ
	QByteArrayList getRRData(QByteArray ansCode) const; //!< ��Ӧ������ȡn���ֵ�����(hex)

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
	 * \brief ̨��(����)�ٶ��źŶ�ȡ����
	 * errcode 1: disconnect
	 * errcode 2: out of range(4~20ma)
	 * \param int errorCode
	 */
	void truckSpeedError(int);

public slots:
	void sendAlarmToControlCenter(int cio); //!< ���пط�������
};