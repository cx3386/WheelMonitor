#include "stdafx.h"

#include <QSerialPort>
#include <QMessageBox>
#include "PLC.h"

PLC::PLC(QObject *parent)
	: QObject(parent)
{
}

PLC::~PLC()
{
	plcSerialPort->close();
}

void PLC::start()
{
	QTimer::singleShot(0, this, [=]() {
		if (!bConnected)
			return;
		bUsrCtrl = true;
		readSensorPeriodically();
	});
}

void PLC::stop()
{
	QTimer::singleShot(0, this, [=]() {
		bUsrCtrl = false;
		emit cio0Update(0);
	}); //用timeevent，避免跨线程操作带来的锁的管理。但是需要get的value，必须通过锁管理
}

//! 连接到PLC，初始化AD041。在plc创建并移到子线程后执行，应在退出程序时断开
void PLC::connect()
{
	QTimer::singleShot(0, this, [=]() {
		plcSerialPort = new QSerialPort(this);
		plcSerialPort->setPortName("COM3"); // default "COM3"
		plcSerialPort->setBaudRate(QSerialPort::Baud115200); // 2018.9.17
		plcSerialPort->setParity(QSerialPort::EvenParity);
		plcSerialPort->setDataBits(QSerialPort::Data7);
		plcSerialPort->setStopBits(QSerialPort::TwoStop);
		plcSerialPort->close();
		if (!plcSerialPort->open(QIODevice::ReadWrite)) {
			emit connectErr(plcSerialPort->error());
		}
		else {
			bConnected = true;
		}
	});
}

void PLC::readSensorPeriodically()
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
}

QByteArray PLC::readPLC(QByteArray plcData)
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

void PLC::writePLC(QByteArray plcData)
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
QByteArray PLC::getFCSCode(QByteArray cmd) const
{
	int res = 0;
	for (auto c : cmd) {
		res ^= c; // char is auto convert to ASCII
	}
	return QByteArray::number(res, 16).toUpper();
}

QByteArray PLC::getFullCode(QByteArray cmd) const
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
QByteArray PLC::genRRCode(int addr, int num) const
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
QByteArray PLC::genWRCode(int addr, QByteArray data) const
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
bool PLC::checkAnsCode(QByteArray ansCode) const
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
QByteArrayList PLC::getRRData(QByteArray ansCode) const
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