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
		AlarmUnkown = 0,
		AlarmColorGreen = 1,
		AlarmColorRed = 2,
		AlarmColorYellow = 4,
		AlarmOFF = 8
	};
	//use | to group different color, e.g. AlarmColorRed | AlarmColorYellow = 6, represents showing red & yellow at same time
	//not implement yet

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
	void setUiAlarm(PLCSerial::AlarmColor alarmColor);
	void isConnectPLC(bool r);
	void isDisconnectPLC(bool r);
	void ADdisconnected();
	void ADSpeedReady(double speed);

	public slots:
	void init();
	void Alarm(PLCSerial::AlarmColor alarmcolor);

	bool connectPLC();//0-00 1-01 2-10 3-11 4-error1 5-error2
	bool disconnectPLC();
};