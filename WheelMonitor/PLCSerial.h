#pragma once
#include <QObject>
#include <QSerialPort>
#include <QTimer>

class PLCSerial : public QObject
{
	Q_OBJECT
public:
	explicit PLCSerial(QObject *parent = nullptr);
	~PLCSerial();
	enum AlarmColor {
		ALarmUnkown = 0,
		AlarmColorGreen = 1,
		AlarmColorRed = 2,
		AlarmColorYellow = 4,
		ALarmOFF = 8
	};		//可以组合，用|

private:
	bool sensorA;
	bool sensorB;
	bool stopSensor;
	bool isConnect;
	QTimer* sensorTimer;
	QSerialPort* plcSerialPort;
	QByteArray plcData;
	enum AlarmColor currentAlarmColor;

	private slots:
	void loopWheelSensor();

signals:
	//void initSignal();
	void startSave();
	void stopSave();
	void setUiAlarm(AlarmColor alarmColor);
	void isStartWheelSensor(bool r);
	void isStopWheelSensor(bool r);

	public slots :
	void init();
	void Alarm(AlarmColor alarmcolor);

	bool startWheelSensor();//0-00 1-01 2-10 3-11 4-error1 5-error2
	bool stopWheelSensor();
};