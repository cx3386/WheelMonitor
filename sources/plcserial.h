#pragma once
#include <QObject>
#include "common.h"

class QSerialPort;
class PLCSerial : public QObject
{
	Q_OBJECT
public:
	explicit PLCSerial(QObject *parent = nullptr);
	~PLCSerial();
	inline double getTrolleySpeed() { QMutexLocker locker(&mutex); return trolleySpeed * 0.954; }//the rate optimized by LS method (minimize the error)
	inline double isConnect() { QMutexLocker locker(&mutex); return bIsConnect; }
	enum AlarmColor {
		AlarmColorGreen,
		AlarmColorRed,
		AlarmColorYellow,
		AlarmOFF,
	};

private:
	QMutex mutex;
	QSerialPort *plcSerialPort;
	double trolleySpeed; //linear Velocity read from AD, unit:m/min
	bool sensorRight = false; //out
	bool sensorLeft = false;  //in
	bool stopSensor = false;
	bool bIsConnect = false;
	QTimer *sensorTimer;
	QTimer *trolleyTimer;
	/* one read write period >300ms */
	int sensorSamplingPeriod = 1000; // msec
	int trolleySamplingPeriod = 250;//msec; should > 7/25 s

	void writePLC(QByteArray plcData);
	QByteArray readPLC(QByteArray plcData);

	private slots:
	void readSensor();
	void readTrolley(); //2017.11.9 read trolley reference speed through (PC-PLC-AD-Trolley)

signals:
	void sensorIN();
	void sensorOUT();
	void trolleySpeedReady();
	void trolleySpeedError();

	public slots:
	void init();
	void onAlarmEvent(int);

	void startTimer(); // can't op timer through thread
	void stopTimer();
};