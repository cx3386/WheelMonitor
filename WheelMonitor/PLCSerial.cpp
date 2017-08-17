#include "stdafx.h"
#include "PLCSerial.h"


PLCSerial::PLCSerial(QObject *parent) : QObject(parent)
	, sensorA(false)
	, sensorB(false)
	, stopSensor(false)
	, isConnect(false)
{
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
	if (!plcSerialPort->open(QIODevice::ReadWrite)) {
		qWarning() << QString("Can't connect PLC, error code %2").arg(plcSerialPort->error());
	}
	else
		isConnect = true;
}

void PLCSerial::Alarm(const char* lightcolor)		//Ӧ�ø�ΪAlarm(AlarmColor alarmcolor)����ʽ��ͨ��&Mask�ж�FLag��ֵ,�ٶ�plcData����Ҫ����Ϊȫ�ֱ�������ֵ
{
	plcData = lightcolor;
	
	if (plcData == ALARM_LIGHT_GREEN)
	{
		emit setUiAlarm(AlarmColorGreen);
		currentAlarmColor = AlarmColorGreen;
	}
	else if (plcData == ALARM_LIGHT_RED)
	{
		emit setUiAlarm(AlarmColorRed);
		currentAlarmColor = AlarmColorRed;
	}
	else if (plcData == ALARM_LIGHT_YELLOW)		//yellow light(waring) cannot override red
	{
		if (currentAlarmColor == AlarmColorRed)
		{
			return;
		}
		else
		{
			emit setUiAlarm(AlarmColorYellow);
			currentAlarmColor = AlarmColorYellow;
		}
	}
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			if (responseData != "@00WR0045*\r")
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
	plcData = "@00RR0000000141*\r";
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100)) {
		if (plcSerialPort->waitForReadyRead(100)) {
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();

			if (responseData == "@00RR00000040*\r")
			{
				if (sensorA == true && sensorB == false) 
				{
					emit stopSave();
				}
				sensorA = false;
				sensorB = false;
				//00 10->00����A���½��أ�����ΪA���磬ֹͣ¼��
			}	//���״̬�ı��򷢳��źŷ���return
			else if (responseData == "@00RR00000242*\r")
			{
				sensorA = true;
				sensorB = false;
			}	//01
			else if (responseData == "@00RR00000848*\r")
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
			else if (responseData == "@00RR00000A31*\r")
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