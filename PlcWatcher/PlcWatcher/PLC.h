#pragma once

#include <QObject>
class QSerialPort;

class PLC : public QObject
{
	Q_OBJECT

public:
	PLC(QObject *parent = Q_NULLPTR);
	~PLC();
	void start();
	void stop();
	void connect();
private:
	QSerialPort* plcSerialPort;
	bool bUsrCtrl = false;
	bool bConnected = false;
	/* 注意：单次读写的耗时大概在~300ms */
	static const int cio0Interval = 1000; //!<  读取cio0（传感器）的间隔，ms
	static const int serialport_timeout = 800;//响应超时时间ms，应小于读取间隔
	void writePLC(QByteArray plcData);
	QByteArray readPLC(QByteArray plcData);
	//PC与PLC的通信，采用HOSTLINK协议的C-MODE指令
	//e.g., cmd = '@00RR00020001', fcs code = '0x43', the full code = "@00RR0002000143*\r"
	QByteArray getFCSCode(QByteArray cmd) const; //!< 根据命令码获取FCS码，通常情况下不需要直接调用
	QByteArray getFullCode(QByteArray cmd) const;//!< 根据命令码获取完整指令码
	QByteArray genRRCode(int addr, int num) const; //!< 返回读取cio所需要的完整指令码
	QByteArray genWRCode(int addr, QByteArray data) const; //!< 返回写入cio所需要的完整指令码
	bool checkAnsCode(QByteArray ansCode) const; //!< 检查应答是否正确
	QByteArrayList getRRData(QByteArray ansCode) const; //!< 从应答中提取n个字的数据(hex)
private slots:
	void readSensorPeriodically();
signals:
	void cio0Update(int cio);
	void connectErr(int code);
};
