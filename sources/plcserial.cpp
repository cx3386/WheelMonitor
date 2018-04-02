#include "stdafx.h"
#include "plcserial.h"
#include <QSerialPort>

/*Write alarm light*/
/*the center control alarm is 0001*/
const char* ALARM_LIGHT_ON = "@00WR010000F032*\r";	//1111
const char* ALARM_LIGHT_OFF = "@00WR0100000044*\r";	//0000
const char* ALARM_LIGHT_RED = "@00WR0100001144*\r";
const char* ALARM_LIGHT_GREEN = "@00WR0100002046*\r";
const char* ALARM_LIGHT_YELLOW = "@00WR0100004040*\r";

/*Read sensor state*/
const char* READ_SENSOR_STATE = "@00RR0000000141*\r";
const char* SENSOR_L_OFF_R_OFF = "@00RR00000040*\r"; //00
const char* SENSOR_L_OFF_R_ON = "@00RR00000242*\r";	//01
const char* SENSOR_L_ON_R_OFF = "@00RR00000848*\r";	//10
const char* SENSOR_L_ON_R_ON = "@00RR00000A31*\r"; //11

/*AD module*/
const char* INIT_AD = "@00WR0102800A800037*\r";//use ad ad input port01, 4-20ma, no average
const char* READ_AD = "@00RR0002000143*\r";

const char* WR_CORRECT_RESPONSE = "@00WR0045*\r";

double PLCSerial::speedAD = 0.0;

PLCSerial::PLCSerial(QObject *parent) : QObject(parent), sensorRight(false), sensorLeft(false), stopSensor(false), isConnect(false), currentAlarmColor(AlarmColorUnkown)
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
	/*setup serialport parameters*/
	plcSerialPort = new QSerialPort;
	plcSerialPort->setPortName("COM3");
	plcSerialPort->setBaudRate(QSerialPort::Baud9600);
	plcSerialPort->setParity(QSerialPort::EvenParity);
	plcSerialPort->setDataBits(QSerialPort::Data7);
	plcSerialPort->setStopBits(QSerialPort::TwoStop);
	plcSerialPort->close();
	if (!plcSerialPort->open(QIODevice::ReadWrite))
	{
		qWarning() << QStringLiteral("PLC: Can't connect PLC, error code %1").arg(plcSerialPort->error());
	}
	else
		isConnect = true;

	/*init the AD input mouduole*/
	QByteArray plcData = INIT_AD;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			if (responseData != WR_CORRECT_RESPONSE)
				qWarning() << "PLC: Init PLC(AD) failed";
		}
	}
}

void PLCSerial::Alarm(AlarmColor alarmcolor)
{
	QMutexLocker locker(&mutex);
	if ((currentAlarmColor == alarmcolor) || ((currentAlarmColor == AlarmColorRed) && (alarmcolor & AlarmColorYellow))) //yellow light(waring) never override red
		return;																											//if alarmcolor is same as currentcolor, or color is now red and to be yellow, return;
	currentAlarmColor = alarmcolor;
	QByteArray plcData;
	if (alarmcolor & AlarmColorGreen)
		plcData = ALARM_LIGHT_GREEN;
	else if (alarmcolor & AlarmColorRed)
		plcData = ALARM_LIGHT_RED;
	else if (alarmcolor & AlarmColorYellow)
		plcData = ALARM_LIGHT_YELLOW;
	else if (alarmcolor & AlarmOFF)
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
				qWarning() << "PLC: Alarm Light wrong return";
		}
	}
}

bool PLCSerial::connectPLC()
{
	if (isConnect)
	{
		sensorTimer = new QTimer;
		connect(sensorTimer, SIGNAL(timeout()), this, SLOT(readSensor()));
		sensorTimer->start(1000); //0 is error, 1 is ok.
		ADTimer = new QTimer;
		connect(ADTimer, SIGNAL(timeout()), this, SLOT(readAD()));
		ADTimer->start(250);	//read ad every 250ms, ensure update more frequent than image process 8/25
		emit isConnectPLC(true);
		return true;
	}
	else
	{
		qWarning() << QStringLiteral("PLC: Can't connect PLC, error code %2").arg(plcSerialPort->error());
		emit isConnectPLC(false);
		return false;
	}
}
bool PLCSerial::disconnectPLC()
{
	sensorTimer->stop();
	delete sensorTimer;
	sensorTimer = nullptr;
	ADTimer->stop();
	delete ADTimer;
	ADTimer = nullptr;
	emit isDisconnectPLC(true);
	return true;
}

void PLCSerial::readSensor()
{
	QMutexLocker locker(&mutex);

	QByteArray plcData = READ_SENSOR_STATE;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();

			if (responseData == SENSOR_L_OFF_R_OFF)
			{//00
				if (sensorRight == true && sensorLeft == false)
				{//01->00 wheel leaves the right sensor
					emit sensorOUT();
				}
				sensorRight = false;
				sensorLeft = false;
			}
			else if (responseData == SENSOR_L_OFF_R_ON)
			{//01
				sensorRight = true;
				sensorLeft = false;
			}
			else if (responseData == SENSOR_L_ON_R_OFF)
			{//10
				if (sensorRight == false && sensorLeft == false)
				{//00->10 wheel enters the left sensor
					emit sensorIN();
				}
				sensorRight = false;
				sensorLeft = true;
			}
			else if (responseData == SENSOR_L_ON_R_ON)
			{//11
				qWarning() << "PLC(sensor): Both sensors are triggered at the same time, please check if the sensor is broken";
				sensorRight = true;
				sensorLeft = true;
			}
			else
			{//wrong response
				qWarning() << "PLC(sensor): Wrong response";
			}
		}
		else
		{
			//QString str(tr("Wait read response timeout %1")
			//.arg(QTime::currentTime().toString()));
			qWarning() << "PLC(sensor): Wait read response timeout";
		}
	}
	else
	{
		qWarning() << "PLC(sensor): Wait write request timeout";
	}
}

void PLCSerial::readAD()	//write and read one time need at least 300ms
{
	QMutexLocker locker(&mutex);
	QByteArray plcData = READ_AD;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			mutex.unlock();
			bool ok;
			int tmpDec = QString(responseData.mid(7, 4)).toInt(&ok, 16);
			if (8000 == tmpDec || 65236 <= tmpDec || 65531 <= tmpDec)//8000 or under -5, think ad module is down
			{
				emit ADdisconnected();
				mutex.lock();
				speedAD = 0;
			}
			else
			{
				mutex.lock();
				speedAD = tmpDec * 3.59 / 6000.0;
				emit ADSpeedReady(speedAD);
			}
		}
		else
		{
			qWarning() << "PLC(AD):Wait read response timeout";
		}
	}
	else
	{
		qWarning() << "PLC(AD): Wait write request timeout";
	}
}