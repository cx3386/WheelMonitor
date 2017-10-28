#include "stdafx.h"
#include "PLCSerial.h"

/*Write alarm light*/
/*the center control alarm is 0001*/
#define ALARM_LIGHT_ON		    "@00WR010000F032*\r"		//1111
#define ALARM_LIGHT_OFF		    "@00WR0100000044*\r"		//0000
#define ALARM_LIGHT_RED		    "@00WR0100001144*\r"
#define ALARM_LIGHT_GREEN	    "@00WR0100002046*\r"
#define ALARM_LIGHT_YELLOW		"@00WR0100004040*\r"

/*Read sensor state*/
#define READ_SENSOR_STATE       "@00RR0000000141*\r"
#define SENSOR_A_OFF_B_OFF	    "@00RR00000040*\r"
#define SENSOR_A_ON_B_OFF	    "@00RR00000242*\r"
#define SENSOR_A_OFF_B_ON	    "@00RR00000848*\r"
#define SENSOR_A_ON_B_ON	    "@00RR00000A31*\r"

#define WR_CORRECT_RESPONSE       "@00WR0045*\r"

PLCSerial::PLCSerial(QObject *parent) : QObject(parent)
, sensorA(false)
, sensorB(false)
, stopSensor(false)
, isConnect(false)
, currentAlarmColor(ALarmUnkown)
{
	//qRegisterMetaType<PLCSerial::AlarmColor>("AlarmColor");	//2017/10/26
}

PLCSerial::~PLCSerial()
{
	//sensorTimer->deleteLater();
	plcSerialPort->deleteLater();
	plcSerialPort->close();
}

void PLCSerial::init()
{
	plcSerialPort = new QSerialPort;
	plcSerialPort->setPortName("COM3");
	plcSerialPort->setBaudRate(QSerialPort::Baud9600);
	plcSerialPort->setParity(QSerialPort::EvenParity);
	plcSerialPort->setDataBits(QSerialPort::Data7);
	plcSerialPort->setStopBits(QSerialPort::TwoStop);
	plcSerialPort->close();
	if (!plcSerialPort->open(QIODevice::ReadWrite))
	{
		qWarning() << QString("Can't connect PLC, error code %1").arg(plcSerialPort->error());
	}
	else
		isConnect = true;
}

void PLCSerial::Alarm(AlarmColor alarmcolor)		//Ӧ�ø�ΪAlarm(AlarmColor alarmcolor)����ʽ��ͨ��&Mask�ж�FLag��ֵ,�ٶ�plcData����Ҫ����Ϊȫ�ֱ�������ֵ
{
	if ((currentAlarmColor == alarmcolor) || ((currentAlarmColor == AlarmColorRed) && (alarmcolor & AlarmColorYellow)))	//yellow light(waring) never override red
		return;	//if alarmcolor is same as currentcolor, or color is now red and to be yellow, return;
	currentAlarmColor = alarmcolor;
	if (alarmcolor & AlarmColorGreen)
		plcData = ALARM_LIGHT_GREEN;
	else if (alarmcolor & AlarmColorRed)
		plcData = ALARM_LIGHT_RED;
	else if (alarmcolor & AlarmColorYellow)
		plcData = ALARM_LIGHT_YELLOW;
	else if (alarmcolor & ALarmOFF)
		plcData = ALARM_LIGHT_OFF;
	emit setUiAlarm(alarmcolor);
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			if (responseData != WR_CORRECT_RESPONSE)
				qWarning() << "Error: Alarm Light wrong return";
		}
	}
}

bool PLCSerial::startWheelSensor()
{
	if (isConnect)
	{
		sensorTimer = new QTimer;
		connect(sensorTimer, SIGNAL(timeout()), this, SLOT(loopWheelSensor()));
		sensorTimer->start(1000);		//0 is error, 1 is ok.
		emit isStartWheelSensor(true);
		return true;
	}
	else
	{
		qWarning() << QString("Can't connect PLC, error code %2").arg(plcSerialPort->error());
		emit isStartWheelSensor(false);
		return false;
	}
}
bool PLCSerial::stopWheelSensor()
{
	sensorTimer->stop();
	delete sensorTimer;
	sensorTimer = nullptr;
	emit isStopWheelSensor(true);
	return true;
}

void PLCSerial::loopWheelSensor()
{
	plcData = READ_SENSOR_STATE;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100)) {
		if (plcSerialPort->waitForReadyRead(100)) {
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();

			if (responseData == SENSOR_A_OFF_B_OFF)
			{
				if (sensorA == true && sensorB == false)
				{
					emit stopSave();
				}
				sensorA = false;
				sensorB = false;
				//00 10->00����A���½��أ�����ΪA���磬ֹͣ¼��
			}	//���״̬�ı��򷢳��źŷ���return
			else if (responseData == SENSOR_A_ON_B_OFF)
			{
				sensorA = true;
				sensorB = false;
			}	//01
			else if (responseData == SENSOR_A_OFF_B_ON)
			{
				//if (sensorA == false && sensorB == true)
				//	return;
				//else
				//{
				//	sensorA = false;
				//	sensorB = true;
				//	emit stopSave();
				//}
				if (sensorA == false && sensorB == false)
				{
					emit startSave();
				}
				sensorA = false;
				sensorB = true;
			}	//10 00->01,��B�������أ���ΪB���룬��ʼ¼��
			//planA����Ļ���return���ܣ�һ��������ڵ��źţ����۴��ĸ�״̬�仯���ģ����ж�ԭ����ִ�ж�����planB��������״���󣬶���һ��״̬����ɸѡ�������ϣ����״̬��Ψһ������ִ�ж���
			//planB��ִ�и����ϸ���Ϊ��Ҫ����һ��״̬����Ƿ���Ҫ��ģ�����Ҫȴ������һ����������
			else if (responseData == SENSOR_A_ON_B_ON)
			{
				qWarning() << "Error: the wheel sensor: A&B";
			}	//11
			else qWarning() << "Error: the wheel sensor wrong response";
		}
		else {
			//QString str(tr("Wait read response timeout %1")
			//.arg(QTime::currentTime().toString()));
			qWarning() << "Wait read response timeout";
		}
	}
	else {
		//QString str(tr("Wait write request timeout %1")
		//.arg(QTime::currentTime().toString()));
		qWarning() << "Wait write request timeout";
	}
	//qDebug() << "SensorA: " << sensorA << "SensorB: " << sensorB;
}