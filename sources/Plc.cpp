#include "stdafx.h"

#include "AlarmEvent.h"
#include "Plc.h"
#include "SpeedIntegrator.h"
#include "common.h"
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

Plc::Plc(const ConfigHelper* _configHelper, QObject* parent)
	: QObject(parent)
	, configHelper(_configHelper)
	, speedIntegrator(new SpeedIntegrator(this))
{
	handleSensorDevice[0] = new HandleSensorDevice(0, this);
	handleSensorDevice[1] = new HandleSensorDevice(1, this);
	QObject::connect(handleSensorDevice[0], &HandleSensorDevice::triInt, speedIntegrator, &SpeedIntegrator::onCkpTri);
	QObject::connect(handleSensorDevice[1], &HandleSensorDevice::triInt, speedIntegrator, &SpeedIntegrator::onCkpTri);
	QObject::connect(speedIntegrator, &SpeedIntegrator::expectTriBegin, this, [=](int ckpId) {
		handleSensorDevice[ckpId >> 1]->newCkp(ckpId);
	});
	QObject::connect(speedIntegrator, &SpeedIntegrator::expectTriEnd, this, [=](int ckpId) {
		handleSensorDevice[ckpId >> 1]->checkoutCkp(ckpId);
	});
	refThread = new QThread(this);
	speedIntegrator->moveToThread(refThread);
	refThread->start();
}

Plc::~Plc()
{
	plcSerialPort->close();
	refThread->quit();
	refThread->wait();
}

void Plc::start()
{
	QTimer::singleShot(0, this, [=]() {
		if (!bConnected)
			return;
		bUsrCtrl = true;
		readSensorPeriodically();
		readSpeedPeriodically(); // sensor置为空,使能第一次.state
		handleSensorDevice[0]->dev->init();
		handleSensorDevice[1]->dev->init();
		// 开始truckref的计数
		speedIntegrator->start();
	});
}

void Plc::stop()
{
	QTimer::singleShot(0, this, [=]() {
		bUsrCtrl = false;
		speedIntegrator->stop();
		emit cio0Update(0);
	}); //用timeevent，避免跨线程操作带来的锁的管理。但是需要get的value，必须通过锁管理
}

//! 连接到PLC，初始化AD041。在plc创建并移到子线程后执行，应在退出程序时断开
void Plc::connect()
{
	QTimer::singleShot(0, this, [=]() {
		/*setup serialport parameters*/
		plcSerialPort = new QSerialPort(this);
		plcSerialPort->setPortName(configHelper->pc2plc_portName); // default "COM3"
		plcSerialPort->setBaudRate(QSerialPort::Baud115200); // 2018.9.17
		plcSerialPort->setParity(QSerialPort::EvenParity);
		plcSerialPort->setDataBits(QSerialPort::Data7);
		plcSerialPort->setStopBits(QSerialPort::TwoStop);
		plcSerialPort->close();
		if (!plcSerialPort->open(QIODevice::ReadWrite)) {
			qWarning() << QStringLiteral("PLC: Can't open serial, error code %1").arg(plcSerialPort->error());
			emit connectError(1);
		}
		else {
			bConnected = true;
			emit connectError(0);
		}

		/*init the AD input module*/
		bool ok;
		int w1 = 1 << 15 | QString("1010").toInt(&ok, 2); //note: int(32) max ~= 10^10
		int w2 = 1 << 15;
		auto word102 = QByteArray::number(w1, 16).rightJustified(4, '0').toUpper();
		auto word103 = QByteArray::number(w2, 16).rightJustified(4, '0').toUpper();
		writePLC(genWRCode(102, word102 + word103));
	});
}

void Plc::readSensorPeriodically()
{
	if (!bUsrCtrl)
		return;
	QTimer::singleShot(cio0Interval, this, [=]() { readSensorPeriodically(); });
	auto ansCode = readPLC(genRRCode(0, 1));
	auto dataList = getRRData(ansCode);
	if (dataList.size() != 1) // the data should be 1 word
	{
		qWarning() << "read cio0 error. anscode " << ansCode;
		return;
	}
	bool ok;
	int cio0 = QString(dataList.at(0)).toInt(&ok, 16); // convert 'FFFF' to int(10) //0x000~0x1ff, in fact cio0~11(16) ranges to 0xffff.
	// PATCH: cioO.02换成cio0.10 //  [12/5/2018 cx3386]
	// 获取第i位的数据
	auto getBit = [](int cio, int loc) -> bool { return cio >> loc & 1; };
	// 将第i位设置为b
	auto setBit = [](int& cio, int loc, bool bit) {
		if (bit)
			cio |= 1 << loc;
		else
			cio &= ~(1 << loc);
	};
	auto patch_swicth = [getBit, setBit](int& cio, int bit1, int bit2) {
		auto bit_tmp = getBit(cio, bit1);
		setBit(cio, bit1, getBit(cio, bit2));
		setBit(cio, bit2, bit_tmp);
	};
	patch_swicth(cio0, 2, 10);
	emit cio0Update(cio0);
	handleSensorDevice[0]->input(cio0);
	handleSensorDevice[1]->input(cio0);
}

void Plc::readSpeedPeriodically()
{
	if (!bUsrCtrl)
		return;
	QTimer::singleShot(adInterval, this, [=]() { readSpeedPeriodically(); });
	/*	AD分辨率为 1/6000，精度为0.8%满量程（0~55°C），即48单位。
	4~20mA电流范围对应 0000~1770hex(0~6000)。对应的台车（中轴）速度为0~3.59m/min
	可转换的数据范围为FED4~189Chex(-300~6300)。3.2~4mA范围内的电流用二进制补码表示。
	当输入低于指定范围（如低于3.2mA）时，将启动断线检测功能，数据将变为8000hex。
	注意：这个返回的数据是2byte的有符号数，在**64位编译器**中的short（16bit）符合条件 */
	const short AD_min = 0xFED4, AD_max = 0x189c, AD_4ma = 0, AD_20ma = 0x1770, AD_err = 0x8000, AD_res = 48;
	const double scale = 3.59 / 6000; // 1个刻度对应的速度

	auto ansCode = readPLC(genRRCode(2, 1)); //read cio2 (m+1, AD01) for 1 word
	auto dataList = getRRData(ansCode);
	if (dataList.size() != 1) // the data should be 1 word
	{
		qWarning() << "read ad error. anscode " << ansCode;
		return;
	}
	bool ok;
	const short dec = QString(dataList.at(0)).toInt(&ok, 16); // convert 'FFFF' to int(10)
	QMutexLocker locker(&mutex); // protect truckSpeed
	truckSpeed = 0;
	//断线检测
	if (AD_err == dec) {
		emit truckSpeedError(1);
		return;
	}
	// out of range(4~20ma), consider the accuracy/resolution
	if (dec < AD_4ma - AD_res || dec > AD_20ma + AD_res) {
		emit truckSpeedError(2);
		return;
	}
	//正常数据
	else {
		truckSpeed = dec * scale;
		emit truckSpeedReady();
	}
}

QByteArray Plc::readPLC(QByteArray plcData)
{
	QByteArray responseData;
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(serialport_timeout)) {
		if (plcSerialPort->waitForReadyRead(serialport_timeout)) {
			responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
		}
	}
	if (responseData.isNull())
		qWarning() << "response data is null";
	return responseData;
}

void Plc::writePLC(QByteArray plcData)
{
	plcSerialPort->write(plcData);
	if (plcSerialPort->waitForBytesWritten(serialport_timeout)) {
		if (plcSerialPort->waitForReadyRead(serialport_timeout)) {
			QByteArray responseData = plcSerialPort->readAll();
			while (plcSerialPort->waitForReadyRead(100))
				responseData += plcSerialPort->readAll();
			if (!checkAnsCode(responseData))
				qWarning() << "PLC: Wrong response!";
		}
	}
}
QByteArray Plc::getFCSCode(QByteArray cmd) const
{
	int res = 0;
	for (auto c : cmd) {
		res ^= c; // char is auto convert to ASCII
	}
	return QByteArray::number(res, 16).toUpper();
}

QByteArray Plc::getFullCode(QByteArray cmd) const
{
	return cmd + getFCSCode(cmd) + "*\r";
}

/**
 * \brief generate a RR command code
 * PC to PLC ask(command) code
 * "@00RR0001(ADDR)0001(No. of words)FCS*\r"
 * except data and end code is hex, the others is BCD.
 *
 * \param int addr (cio) register address of beginning word to read
 * \param int num how many words to read
 */
QByteArray Plc::genRRCode(int addr, int num) const
{
	if (addr <= 6143 && addr >= 0 && num >= 1 && num <= 6144) {
		return getFullCode(QString("@00RR%1%2").arg(addr, 4, 10, QChar('0')).arg(num, 4, 10, QChar('0')).toUtf8());
	}
	qWarning() << "PLCSerial: addr or No. of words is error";
	return QByteArray();
}

/**
* \brief generate a WR command code
* PC to PLC ask(command) code
* "@00WR0100(ADDR)data(words,hex)FCS*\r"
* except data and end code is hex, the others is BCD.
*
* \param int addr (cio) register address of beginning word to write
* \param QByteArray data how many words to read or the data to write
*/
QByteArray Plc::genWRCode(int addr, QByteArray data) const
{
	if (addr <= 6143 && addr >= 0 && data.size() % 4 == 0) {
		return getFullCode(QString("@00WR%1%2").arg(addr, 4, 10, QChar('0')).arg(QString(data)).toUtf8());
	}
	qWarning() << "PLCSerial: addr or No. of words is error";
	return QByteArray();
}

/**
 * \brief check answer code if it's right.
 * PLC to PC answer(response) code:
 * "@00RR00(endcode,hex)data(words,hex)FCS*\r"
 * "@00WR00(endcode,hex)FCS*\r"
 * except data and end code is hex, the others is BCD.
 *
 * \param QByteArray
 */
bool Plc::checkAnsCode(QByteArray ansCode) const
{
	// null response
	if (ansCode.isNull()) {
		return false;
	}
	//check the format
	if (ansCode.startsWith("@00") && ansCode.endsWith("*\r")) {
		ansCode.chop(2); // chop the terminator
		auto fcs_recv = ansCode.right(2);
		ansCode.chop(2); // chop the fcs
		auto fcs_clc = getFCSCode(ansCode);
		if (fcs_clc == fcs_recv) {
			auto headerCode = ansCode.mid(3, 2);
			auto endCode = ansCode.mid(5, 2);
			if (endCode == "00") {
				if (headerCode == "WR" || headerCode == "RR") {
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * \brief get RR data from response code
 *
 * \param QByteArrayList hex data list
 */
QByteArrayList Plc::getRRData(QByteArray ansCode) const
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
	// 读取的数据以字（FFFF）为单位
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
/**
 * \brief 向中控发出报警
 *
 * 注意：不会覆盖原有状态
 */
void Plc::sendAlarmToControlCenter(int cio)
{
	QByteArray plcData;
	plcData = genWRCode(100, QByteArray::number(cio, 16).rightJustified(4, '0').toUpper());
	writePLC(plcData);
}