#pragma once
#include <QObject>
#include <QSerialPort>
#include <QTimer>

/*The alarm light control*/
#define ALARM_LIGHT_ON		"@00WR010000F032*\r"		//1111
#define ALARM_LIGHT_OFF		"@00WR0100000044*\r"		//0000
#define ALARM_LIGHT_RED		"@00WR0100001045*\r"	
#define ALARM_LIGHT_GREEN	"@00WR0100002046*\r"
#define ALARM_LIGHT_YELLOW	"@00WR0100004040*\r"	


class PLCSerial : public QObject
{
	Q_OBJECT
public:
	explicit PLCSerial(QObject *parent = nullptr);
	~PLCSerial();
	enum AlarmColor {
		AlarmColorGreen = 1,
		AlarmColorRed = 2,
		AlarmColorYellow = 4
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
	void Alarm(const char* lightcolor);

	bool startWheelSensor();//0-00 1-01 2-10 3-11 4-error1 5-error2
	bool stopWheelSensor();
};