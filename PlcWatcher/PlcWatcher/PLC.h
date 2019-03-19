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
	/* ע�⣺���ζ�д�ĺ�ʱ�����~300ms */
	static const int cio0Interval = 1000; //!<  ��ȡcio0�����������ļ����ms
	static const int serialport_timeout = 800;//��Ӧ��ʱʱ��ms��ӦС�ڶ�ȡ���
	void writePLC(QByteArray plcData);
	QByteArray readPLC(QByteArray plcData);
	//PC��PLC��ͨ�ţ�����HOSTLINKЭ���C-MODEָ��
	//e.g., cmd = '@00RR00020001', fcs code = '0x43', the full code = "@00RR0002000143*\r"
	QByteArray getFCSCode(QByteArray cmd) const; //!< �����������ȡFCS�룬ͨ������²���Ҫֱ�ӵ���
	QByteArray getFullCode(QByteArray cmd) const;//!< �����������ȡ����ָ����
	QByteArray genRRCode(int addr, int num) const; //!< ���ض�ȡcio����Ҫ������ָ����
	QByteArray genWRCode(int addr, QByteArray data) const; //!< ����д��cio����Ҫ������ָ����
	bool checkAnsCode(QByteArray ansCode) const; //!< ���Ӧ���Ƿ���ȷ
	QByteArrayList getRRData(QByteArray ansCode) const; //!< ��Ӧ������ȡn���ֵ�����(hex)
private slots:
	void readSensorPeriodically();
signals:
	void cio0Update(int cio);
	void connectErr(int code);
};
