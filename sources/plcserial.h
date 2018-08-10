#pragma once
#include <QObject>
#include "common.h"

class QSerialPort;
class PLCSerial : public QObject
{
	Q_OBJECT
public:
	explicit PLCSerial(QObject *parent = nullptr);
	~PLCSerial();
	inline double getTrolleySpeed()
	{
		QMutexLocker locker(&mutex);
		return trolleySpeed * 0.954;
	} //the rate optimized by LS method (minimize the error)
	inline double isConnect()
	{
		QMutexLocker locker(&mutex);
		return bIsConnect;
	}
	enum AlarmColor
	{
		AlarmColorGreen,
		AlarmColorRed,
		AlarmColorYellow,
		AlarmOFF,
	};

private:
	QMutex mutex;
	QSerialPort *plcSerialPort;
	double trolleySpeed; //linear Velocity read from AD, unit:m/min
	// bool sensorRight = false; //out
	// bool sensorLeft = false;  //in
	bool sol = false; ///< sensor out left ×ó²à½øÈë
	bool sor = false; ///< sensor out right ÓÒ²àÀë¿ª
	bool sil = false; ///< sensor in left ×ó²àÀë¿ª
	bool sir = false; ///< sensor in right ÓÒ²à½øÈë
	// for unsigned type like word, >> is logic move right.
	// for signed type like int, >> is arithmetic move right
	WORD cio0 = 0;	 ///< cio0 used0.1~0.8
	bool sol0 = false;
	bool sol1 = false;
	bool sor0 = false;
	bool sor1 = false;
	bool sir0 = false;
	bool sir1 = false;
	bool sil0 = false;
	bool sil1 = false;
	bool stopSensor = false;
	bool bIsConnect = false;
	QTimer *sensorTimer;
	QTimer *trolleyTimer;
	/* one read write period >300ms */
	int sensorSamplingPeriod = 1000; // msec
	int trolleySamplingPeriod = 250; //msec; should > 7/25 s

	void writePLC(QByteArray plcData);
	QByteArray readPLC(QByteArray plcData);

	private slots:
	void readSensor();
	void readTrolley(); //2017.11.9 read trolley reference speed through (PC-PLC-AD-Trolley)

signals:
	void showCio2Ui(WORD);
	void sensorIN();
	void sensorOUT();
	void trolleySpeedReady();
	void trolleySpeedError();

	public slots:
	void init();
	void onAlarmEvent(int);

	void startTimer(); // can't op timer through thread
	void stopTimer();
};