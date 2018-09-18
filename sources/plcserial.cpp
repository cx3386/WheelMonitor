#include "stdafx.h"
#include "plcserial.h"
#include "confighelper.h"
#include <QSerialPort>

/* CPU: CP1E - N30S1DR - A*/
/* I/O */
// INPUT:  2WORD in total. CIO0.0~0.11     CIO1.0~1.05     m=1
// OUTPUT: 2WORD in total. CIO100.0~100.07 CIO101.0~101.03 n=101
/*Read sensor state command*/
// "@00RRADDRdataFCS*\r",
// ADDR is register address to read(10based), e.g. 0101, 0001.
// data=FFFF(2B.hex?10based?) is how many word to read, e.g. 0001 means read 1 WORD
// FCS see getFCSCode()
//const char READ_SENSOR_STATE[] = "@00RR0000000141*\r";  ///< read CIO0 for 1 WORD
/*Read sensor state response*/
// "@00RR00dataFCS*\r", data=FFFF(2B)
// NOTE: cio0.00 is NC
// const int msk_sol0 = 1 << 1; //cio0.01
// const int msk_sol1 = 1 << 2; //cio0.02
// const int msk_sor0 = 1 << 3; //cio0.03
// const int msk_sor1 = 1 << 4; //cio0.04
// const int msk_sir0 = 1 << 5; //cio0.05
// const int msk_sir0 = 1 << 6; //cio0.06
// const int msk_sil0 = 1 << 7; //cio0.07
// const int msk_sil1 = 1 << 8; //cio0.08

//const char SENSOR_L_OFF_R_OFF[] = "@00RR00000040*\r"; //00
//const char SENSOR_L_OFF_R_ON[] = "@00RR00000242*\r";  //01
//const char SENSOR_L_ON_R_OFF[] = "@00RR00000848*\r";  //10
//const char SENSOR_L_ON_R_ON[] = "@00RR00000A31*\r";   //11

/*(deprecated)Write alarm light command*/
/*the center control alarm is 0001, which is real useful*/
//const char ALARM_LIGHT_ON[] = "@00WR010000F032*\r";		///< all on
//const char ALARM_LIGHT_OFF[] = "@00WR0100000044*\r";	///< all off
//const char ALARM_LIGHT_RED[] = "@00WR0100001144*\r";	///< red and alarm
//const char ALARM_LIGHT_GREEN[] = "@00WR0100002046*\r";  ///< green
//const char ALARM_LIGHT_YELLOW[] = "@00WR0100004040*\r"; ///< yellow

/* AD041 module */
// INPUT:  4WORD in total. CIO2~CIO5 m+1~m+4,
// OUTPUT: 2WORD in total. CIO102~CIO103) n+1,n+2
// STEPS:
// 1. write setup data to output word
//		- use the port
//		- average calculate
//		- select analog input range(00:-10~10v;01:0~10v;10:1~5v/4~20ma;11:0~5v/0~20ma)
//		- recv WR_CORRECT response
// 2. read A/D result from input word
//		- sent "read CIO2~5" command to PLC
//		- recv data(1WORD) of port01~04 from PLC in "@00RR00dataFCS*\r" data=FFFF(2B)
// NOTE: must init before it can start AD transverse and can read data from it.
// once init complete, cannot change until restart CPU
// const char INIT_AD[] = "@00WR0102800A800037*\r"; ///< ADO41 setup data. 800A 8000. use ad analog input port01, no average, 4-20ma
// const char READ_AD[] = "@00RR0002000143*\r"; ///< AD041. read CIO2 for 1 word

//const char WR_CORRECT_RESPONSE[] = "@00WR0045*\r"; ///< write response: cio correct

PLCSerial::PLCSerial(const ConfigHelper *_configHelper, QObject *parent)
	: QObject(parent), configHelper(_configHelper)
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
	plcSerialPort->setPortName(configHelper->pc2plc_portName); // default "COM3"
	plcSerialPort->setBaudRate(QSerialPort::Baud115200); // 2018.9.17
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

	/*init the AD input module*/
	bool ok;
	int w1 = 1 << 15 | QString("1010").toInt(&ok, 2); //note: int32 max ~= 10^10
	int w2 = 1 << 15;
	auto word102 = QByteArray::number(w1, 16).rightJustified(4, '0').toUpper();
	auto word103 = QByteArray::number(w2, 16).rightJustified(4, '0').toUpper();
	writePLC(genWRCode(102, word102 + word103));
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
	auto ansCode = readPLC(genRRCode(0, 1));
	auto dataList = getRRData(ansCode);
	if (dataList.size() != 1) // the data should be 1 word
	{
		qWarning() << "PLCSerial: readSensor error";
		return;
	}
	bool ok;
	const WORD cioNow = QString(dataList.at(1)).toInt(&ok, 16); // convert 'FFFF' to int(10) //0x000~0x1ff, in fact cio0~11(16) ranges to 0xffff.
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

	if (ansCode == SENSOR_L_OFF_R_OFF)
	{ //00
		if (sensorRight == true && sensorLeft == false)
		{ //01->00 wheel leaves the right sensor
			emit sensorOUT();
		}
		sensorRight = false;
		sensorLeft = false;
	}
	else if (ansCode == SENSOR_L_OFF_R_ON)
	{ //01
		sensorRight = true;
		sensorLeft = false;
	}
	else if (ansCode == SENSOR_L_ON_R_OFF)
	{ //10
		if (sensorRight == false && sensorLeft == false)
		{ //00->10 wheel enters the left sensor
			emit sensorIN();
		}
		sensorRight = false;
		sensorLeft = true;
	}
	else if (ansCode == SENSOR_L_ON_R_ON)
	{ //11
		qWarning() << "PLC(sensor): Both sensors are triggered at the same time, please check if the sensor is broken";
		sensorRight = true;
		sensorLeft = true;
	}
}

void PLCSerial::readTrolley()
{
	/*	AD分辨率为 1/6000，精度为0.8%满量程（0~55°C），即48单位。
	4~20mA电流范围对应 0000~1770hex(0~6000)。对应的台车（中轴）速度为0~3.59m/min
	可转换的数据范围为FED4~189Chex(-300~6300)。3.2~4mA范围内的电流用二进制补码表示。
	当输入低于指定范围（如低于3.2mA）时，将启动断线检测功能，数据将变为8000hex。
	注意：这个返回的数据是2byte的有符号数，在**64位编译器**中的short符合条件 */
	const short AD_min = 0xFED4, AD_max = 0x189c, AD_4ma = 0, AD_20ma = 0x1770, AD_err = 0x8000, AD_res = 48;
	const double scale = 3.59 / 6000; // 1个刻度对应的速度

	auto ansCode = readPLC(genRRCode(2, 1));//read cio2 (in AD01) for 1 word
	auto dataList = getRRData(ansCode);
	if (dataList.size() != 1) // the data should be 1 word
	{
		qWarning() << "PLCSerial: readTrolley error";
		return;
	}
	bool ok;
	const short dec = QString(dataList.at(1)).toInt(&ok, 16); // convert 'FFFF' to int(10)
	QMutexLocker locker(&mutex);// protect trolleySpeed
	trolleySpeed = 0;
	//断线检测
	if (AD_err == dec)
	{
		emit trolleySpeedError(1);
		return;
	}
	// out of range(4~20ma), consider the accuracy/resolution
	if (dec < AD_4ma - AD_res || dec > AD_20ma + AD_res)
	{
		emit trolleySpeedError(2);
		return;
	}
	//正常数据
	else
	{
		trolleySpeed = dec * scale;
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
			if (!checkAnsCode(responseData))
				qWarning() << "PLC: Wrong response!";
		}
	}
}
QByteArray PLCSerial::getFCSCode(QByteArray cmd)
{
	int res = 0;
	for (auto c : cmd) {
		res ^= c; // char is auto convert to ASCII
	}
	return QByteArray::number(res, 16).toUpper();
}

QByteArray PLCSerial::genRRCode(int addr, int num)
{
	if (addr <= 6143 && addr >= 0 && num >= 1 && num <= 6144)
	{
		return getFullCode(QString("@00RR%1%2").arg(addr, 4, 10, QChar('0')).arg(num, 4, 10, QChar('0')).toUtf8());
	}
	qWarning() << "PLCSerial: addr or No. of words is error";
	return QByteArray();
}

QByteArray PLCSerial::genWRCode(int addr, QByteArray data)
{
	if (addr <= 6143 && addr >= 0 && data.size() % 4 == 0)
	{
		return getFullCode(QString("@00WR%1%2").arg(addr, 4, 10, QChar('0')).arg(QString(data)).toUtf8());
	}
	qWarning() << "PLCSerial: addr or No. of words is error";
	return QByteArray();
}

bool PLCSerial::checkAnsCode(QByteArray code)
{
	// null response
	if (code.isNull())
	{
		return false;
	}
	//check the format
	if (code.startsWith("@00") && code.endsWith("*\r"))
	{
		code.chop(2); // chop the terminator
		auto fcs_recv = code.right(2);
		code.chop(2); // chop the fcs
		auto fcs_clc = getFCSCode(code);
		if (fcs_clc == fcs_recv)
		{
			auto headerCode = code.mid(3, 2);
			auto endCode = code.mid(5, 2);
			if (endCode == "00")
			{
				if (headerCode == "WR" || headerCode == "RR")
				{
					return true;
				}
			}
		}
	}
	return false;
}

QByteArrayList PLCSerial::getRRData(QByteArray ansCode)
{
	QByteArrayList words;
	if (!checkAnsCode(ansCode)) {
		qWarning() << "PLCSerial: plc response data err!";
		return words;
	}
	if (ansCode.at(3) != 'R') {
		qWarning() << "PLCSerial: try to read data from write cmd!";
		return words;
	}
	auto data = ansCode.mid(7, ansCode.size() - 7 - 4);
	if (data.size() % 4 != 0) {
		qWarning() << "PLCSerial: plc response data err!";
		return words;
	}
	QByteArray word;
	for (size_t i = 0; i < data.size(); i++) {
		word += data.at(i);
		if ((i + 1) % 4 == 0) {
			words << word;
			word.clear();
		}
	}
	return words;
}
void PLCSerial::onAlarmEvent(int id)
{
	// 此id已经根据调用的deviceIndex进行过处理,外0内1
	//001 010 101 110
	//bug2: 如何关闭警报呢-直接清零
	QByteArray plcData;

	//static防止会覆盖原有状态
	static WORD a = 0;
	switch (id)
	{
	case 0:
		a = 0;// will send 0000, cut off all alarm
		break;
	case 1:
		a |= 1 << 4;//CIO 100.04 out warn
		break;
	case 2:
		a |= 1 << 5;//CIO 100.05 out alarm
		break;
	case 5:
		a |= 1 << 6;//CIO 100.06 in warn
		break;
	case 6:
		a |= 1 << 7;//CIO 100.07 in alarm
		break;

	default: // not do anything
		break;
	}
	plcData = genWRCode(100, QByteArray::number(a, 16).rightJustified(4, '0').toUpper());
	writePLC(plcData);
}