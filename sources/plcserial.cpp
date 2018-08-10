#include "stdafx.h"
#include "plcserial.h"
#include <QSerialPort>

/*Write alarm light command*/
/*the center control alarm is 0001*/
const char ALARM_LIGHT_ON[] = "@00WR010000F032*\r";		///< all on
const char ALARM_LIGHT_OFF[] = "@00WR0100000044*\r";	///< all off
const char ALARM_LIGHT_RED[] = "@00WR0100001144*\r";	///< red and alarm
const char ALARM_LIGHT_GREEN[] = "@00WR0100002046*\r";  ///< green
const char ALARM_LIGHT_YELLOW[] = "@00WR0100004040*\r"; ///< yellow

/* CPU: CP1E - N30S1DR - A*/
/* I/O */
// INPUT:  2WORD CIO0.0~0.11     CIO1.0~1.05     m=1
// OUTPUT: 2WORD CIO100.0~100.07 CIO101.0~101.03 n=101
/*Read sensor state command*/
const char READ_SENSOR_STATE[] = "@00RR0000000141*\r";  ///< read CIO0 for 1 word
/*Read sensor state response*/
// data=FFFF(2B)
// NOTE: cio0.00 is NC
const int msk_sol0 = 1 << 1; //cio0.01
const int msk_sol1 = 1 << 2; //cio0.02
const int msk_sor0 = 1 << 3; //cio0.03
const int msk_sor1 = 1 << 4; //cio0.04
const int msk_sir0 = 1 << 5; //cio0.05
const int msk_sir0 = 1 << 6; //cio0.06
const int msk_sil0 = 1 << 7; //cio0.07
const int msk_sil1 = 1 << 8; //cio0.08

const char SENSOR_L_OFF_R_OFF[] = "@00RR00000040*\r"; //00
const char SENSOR_L_OFF_R_ON[] = "@00RR00000242*\r";  //01
const char SENSOR_L_ON_R_OFF[] = "@00RR00000848*\r";  //10
const char SENSOR_L_ON_R_ON[] = "@00RR00000A31*\r";   //11

/* AD041 module */
// INPUT:  4WORD CIO2~CIO5 m+1~m+4,
// OUTPUT: 2WORD CIO102~CIO103) n+1,n+2
// STEPS:
// 1. write setup data to output word
//		- use the port
//		- average calculate
//		- select analog input range(00:-10~10v;01:0~10v;10:1~5v/4~20ma;11:0~5v/0~20ma)
//		- recv WR_CORRECT response
// 2. read A/D result from input word
//		- sent "read CIO2~5" command to PLC
//		- recv data(1word) of port01~04 from PLC in "@00RR00dataFCS*\r" data=FFFF(2B)
// NOTE: must init before it can start AD transverse and can read data from it.
// once init complete, cannot change until restart CPU
const char INIT_AD[] = "@00WR0102800A800037*\r"; ///< ADO41 setup data. 800A 8000. use ad analog input port01, no average, 4-20ma
const char READ_AD[] = "@00RR0002000143*\r"; ///< AD041. read CIO2 for 1 word

const char WR_CORRECT_RESPONSE[] = "@00WR0045*\r"; ///< write response: cio correct

PLCSerial::PLCSerial(QObject *parent)
	: QObject(parent)
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
		qWarning() << QStringLiteral("PLC: Can't open serial, error code %1").arg(plcSerialPort->error());
	}
	else
	{
		QMutexLocker locker(&mutex);
		bIsConnect = true;
	}

	/*init the AD input mouduole*/
	writePLC(INIT_AD);
}

void PLCSerial::onAlarmEvent(int id)
{
	QByteArray plcData;
	switch ((AlarmColor)id)
	{
	case AlarmColorGreen:
		plcData = ALARM_LIGHT_GREEN;
		break;
	case AlarmColorRed:
		plcData = ALARM_LIGHT_RED;
		break;
	case AlarmColorYellow:
		plcData = ALARM_LIGHT_YELLOW;
		break;
	case AlarmOFF:
		plcData = ALARM_LIGHT_OFF;
		break;
	default:
		break;
	}

	writePLC(plcData);
}

void PLCSerial::startTimer()
{
	sensorTimer = new QTimer;
	connect(sensorTimer, SIGNAL(timeout()), this, SLOT(readSensor()));
	sensorTimer->start(sensorSamplingPeriod);
	trolleyTimer = new QTimer;
	connect(trolleyTimer, SIGNAL(timeout()), this, SLOT(readTrolley()));
	trolleyTimer->start(trolleySamplingPeriod);
}
void PLCSerial::stopTimer()
{
	sensorTimer->stop();
	sensorTimer->deleteLater();
	trolleyTimer->stop();
	trolleyTimer->deleteLater();
}

void PLCSerial::readSensor()
{
	auto data = readPLC(READ_SENSOR_STATE); // like "@00RR00000040*\r"
	if (data.isNull() || data.size() != 15)
	{
		qWarning() << "PLC: Sensor wrong response";
		return;
	}
	bool ok;
	WORD cioNow = WORD((data.mid(8, 3)).toInt(&ok, 16)); //0x000~0x1ff, in fact cio0~11(16) ranges to 0xffff.
	if (cio0 != cioNow)
	{
		cio0 = cioNow;
		emit showCio2Ui(cio0);
	}
	sol0 = cio0 & msk_sol0;
	sol1 = cio0 & msk_sol1;
	sor0 = cio0 & msk_sor0;
	sor1 = cio0 & msk_sor1;
	sir0 = cio0 & msk_sir0;
	sir1 = cio0 & msk_sir0;
	sil0 = cio0 & msk_sil0;
	sil1 = cio0 & msk_sil1;

	if (data == SENSOR_L_OFF_R_OFF)
	{ //00
		if (sensorRight == true && sensorLeft == false)
		{ //01->00 wheel leaves the right sensor
			emit sensorOUT();
		}
		sensorRight = false;
		sensorLeft = false;
	}
	else if (data == SENSOR_L_OFF_R_ON)
	{ //01
		sensorRight = true;
		sensorLeft = false;
	}
	else if (data == SENSOR_L_ON_R_OFF)
	{ //10
		if (sensorRight == false && sensorLeft == false)
		{ //00->10 wheel enters the left sensor
			emit sensorIN();
		}
		sensorRight = false;
		sensorLeft = true;
	}
	else if (data == SENSOR_L_ON_R_ON)
	{ //11
		qWarning() << "PLC(sensor): Both sensors are triggered at the same time, please check if the sensor is broken";
		sensorRight = true;
		sensorLeft = true;
	}
}

void PLCSerial::readTrolley() //write and read one time need at least 300ms
{
	auto data = readPLC(READ_AD);
	if (data.isNull())
		return;
	bool ok;
	int tmpDec = QString(data.mid(7, 4)).toInt(&ok, 16);
	QMutexLocker locker(&mutex);
	if (8000 == tmpDec || 65236 <= tmpDec || 65531 <= tmpDec) //8000 or under -5, think ad module is down
	{
		trolleySpeed = 0;
		emit trolleySpeedError();
	}
	else
	{
		trolleySpeed = tmpDec * 3.59 / 6000.0; //3.59 is max speed
		emit trolleySpeedReady();
	}
}

QByteArray PLCSerial::readPLC(QByteArray plcData)
{
	QByteArray responseData;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
		}
	}
	if (responseData.isNull())
		qWarning() << "PLC: readPLC() error";
	return responseData;
}

void PLCSerial::writePLC(QByteArray plcData)
{
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(100))
	{
		if (plcSerialPort->waitForReadyRead(100))
		{
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			if (responseData != WR_CORRECT_RESPONSE)
				qWarning() << "PLC: Wrong response!";
		}
	}
}