#pragma once
#include <QMutex>
#include <QTime>
#include <QObject>
#include <opencv2/opencv.hpp>
#include "RobustMatcher.h"
#include "PLCSerial.h"
#include <ocr.h>

struct ImageProcessParameters
{
	ImageProcessParameters();
	double angle2Speed;
	double angleSmallRatio;
	double angleBigRatio;
	int radius_min;
	int radius_max;
	int gs1;
	int gs2;
	double dp;
	double minDist;
	double param1;
	double param2;
	bool sensorTriggered;
	cv::Rect roiRect;
};

class ImageProcess : public QObject
{
	Q_OBJECT
public:
	explicit ImageProcess(QObject *parent = nullptr);
	~ImageProcess();

	static ImageProcessParameters g_imgParam;
	cv::Mat imageMatches;
	ocr myocr;

private:
	//int iImgNoCycle;
	int coreImageProcess(); //0-no cycle, 1-matches success
	//bool isSameWheel;
	double angleSum;
	int angleCount;
	int iImgCount;
	int nDetectCount;

	void alarm(double avgAngle);
	void resetCoreProcess();
	bool bIsInArea;
	bool bLastOUT;
	bool bWheelStopped;
	bool bStopProcess;
	QTime time;
	int in_out_time; //ms
	QMutex mutex;
	RobustMatcher rMatcher;

signals:
	//void resultReady(const QString &result);
	void imageProcessReady();
	void showAlarmNum(const QString &s);
	void showImageMatches();
	void setAlarmLight(PLCSerial::AlarmColor alarmcolor);
	void speedClcReady(double speed);
	void realSpeedReady(double speed);
public slots:
	void doImageProcess();
	void startImageProcess();
	void stopImageProcess();
	void sensorIN();
	void sensorOUT();
	void wheelTimeout();
	//{
	//QString result;
	///* ... here is the expensive or blocking operation ... */
	//emit resultReady(result);
};
