#include "stdafx.h"
#include "PLCSerial.h"

/*Write alarm light*/
/*the center control alarm is 0001*/
#define ALARM_LIGHT_ON "@00WR010000F032*\r"  //1111
#define ALARM_LIGHT_OFF "@00WR0100000044*\r" //0000
#define ALARM_LIGHT_RED "@00WR0100001144*\r"
#define ALARM_LIGHT_GREEN "@00WR0100002046*\r"
#define ALARM_LIGHT_YELLOW "@00WR0100004040*\r"

/*Read sensor state*/
#define READ_SENSOR_STATE "@00RR0000000141*\r"
#define SENSOR_A_OFF_B_OFF "@00RR00000040*\r"
#define SENSOR_A_ON_B_OFF "@00RR00000242*\r"
#define SENSOR_A_OFF_B_ON "@00RR00000848*\r"
#define SENSOR_A_ON_B_ON "@00RR00000A31*\r"

/*AD module*/
#define INIT_AD "@00WR0102800A800037*\r" //use ad ad input port01, 4-20ma, no average
#define READ_AD "@00RR0002000143*\r"

#define WR_CORRECT_RESPONSE "@00WR0045*\r"

PLCSerial::PLCSerial(QObject *parent) : QObject(parent), sensorA(false), sensorB(false), stopSensor(false), isConnect(false), currentAlarmColor(ALarmUnkown)
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
		qWarning() << QString("Can't connect PLC, error code %1").arg(plcSerialPort->error());
	}
	else
		isConnect = true;

	/*init the AD input mouduole*/
	plcData = INIT_AD;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			if (responseData != WR_CORRECT_RESPONSE)
				qWarning() << "Error: init PLC(AD) failed";
		}
	}
}

void PLCSerial::Alarm(AlarmColor alarmcolor) //应锟矫革拷为Alarm(AlarmColor alarmcolor)锟斤拷锟斤拷式锟斤拷通锟斤拷&Mask锟叫讹拷FLag锟斤拷值,锟劫讹拷plcData锟斤拷锟斤拷要锟斤拷锟斤拷为全锟街憋拷锟斤拷锟斤拷锟斤拷值
{
	if ((currentAlarmColor == alarmcolor) || ((currentAlarmColor == AlarmColorRed) && (alarmcolor & AlarmColorYellow))) //yellow light(waring) never override red
		return;																											//if alarmcolor is same as currentcolor, or color is now red and to be yellow, return;
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
		sensorTimer->start(1000); //0 is error, 1 is ok.
		//ADTimer = new QTimer;
		//connect(ADTimer, SIGNAL(timeout()), this, SLOT(loopAD()));
		//ADTimer->start(100);
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
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
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
				//00 10->00锟斤拷锟斤拷A锟斤拷锟铰斤拷锟截ｏ拷锟斤拷锟斤拷为A锟斤拷锟界，停止录锟斤拷
			} //锟斤拷锟阶刺拷谋锟斤拷蚍⒊锟斤拷藕欧锟斤拷锟絩eturn
			else if (responseData == SENSOR_A_ON_B_OFF)
			{
				sensorA = true;
				sensorB = false;
			} //01
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
			} //10 00->01,锟斤拷B锟斤拷锟斤拷锟斤拷锟截ｏ拷锟斤拷为B锟斤拷锟诫，锟斤拷始录锟斤拷
			//planA锟斤拷锟斤拷幕锟斤拷锟絩eturn锟斤拷锟杰ｏ拷一锟斤拷锟斤拷锟斤拷锟斤拷诘锟斤拷藕牛锟斤拷锟斤拷鄞锟斤拷母锟阶刺拷浠拷锟斤拷模锟斤拷锟斤拷卸锟皆拷锟斤拷锟街达拷卸锟斤拷锟斤拷锟絧lanB锟斤拷锟斤拷锟斤拷锟斤拷状锟斤拷锟襟，讹拷锟斤拷一锟斤拷状态锟斤拷锟斤拷筛选锟斤拷锟斤拷锟斤拷锟较ｏ拷锟斤拷锟阶刺拷锟轿ㄒ伙拷锟斤拷锟斤拷锟街达拷卸锟斤拷锟�
			//planB锟斤拷执锟叫革拷锟斤拷锟较革拷锟斤拷为锟斤拷要锟斤拷锟斤拷一锟斤拷状态锟斤拷锟斤拷欠锟斤拷锟揭拷锟侥ｏ拷锟斤拷锟斤拷要却锟斤拷锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
			else if (responseData == SENSOR_A_ON_B_ON)
			{
				qWarning() << "Error: the wheel sensor: A&B";
			} //11
			else
				qWarning() << "Error: the wheel sensor wrong response";
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
		//QString str(tr("Wait write request timeout %1")
		//.arg(QTime::currentTime().toString()));
		qWarning() << "PLC(sensor): Wait write request timeout";
	}
	//qDebug() << "SensorA: " << sensorA << "SensorB: " << sensorB;
}

void PLCSerial::readFromAD()
{
	plcData = READ_AD;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			bool ok;
			int tmpDec = QString(responseData.mid(7, 4)).toInt(&ok, 16);
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