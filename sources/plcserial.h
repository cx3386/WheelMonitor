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
	static double speedAD;	//linear Velocity read from AD, unit:m/min

private:
	QMutex mutex;
	bool sensorRight;	//out
	bool sensorLeft;	//in
	bool stopSensor;
	bool isConnect;
	QTimer* sensorTimer;
	QTimer* ADTimer;
	QSerialPort* plcSerialPort;
	//QByteArray plcData;
	enum AlarmColor currentAlarmColor;

	private slots:
	void readSensor();
	void readAD();	//2017.11.9

signals:
	//void initSignal();
	void sensorIN();
	void sensorOUT();
	void setUiAlarm(AlarmColor alarmColor);
	void isConnectPLC(bool r);
	void isDisconnectPLC(bool r);
	void ADdisconnected();
	void ADSpeedReady(double speed);

	public slots:
	void init();
	void Alarm(AlarmColor alarmcolor);

	bool connectPLC();//0-00 1-01 2-10 3-11 4-error1 5-error2
	bool disconnectPLC();
};