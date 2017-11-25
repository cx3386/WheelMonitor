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

class QSqlTableModel;

class ImageProcess : public QObject
{
	Q_OBJECT
public:
	explicit ImageProcess(QSqlTableModel *srcmodel, QObject *parent = Q_NULLPTR);
	~ImageProcess();

	static ImageProcessParameters g_imgParam;
	cv::Mat imageMatches;
	ocr myocr;

private:
	QSqlTableModel *model;
	//int iImgNoCycle;
	int coreImageProcess(); //0-no cycle, 1-matches success
	//bool isSameWheel;
	std::vector<double> rtSpeeds;
	std::vector<double>	refSpeeds;
	//std::vector<double>	corePeriodSpeeds;	//used in statistics in a imageCoreProcess period, clear when core process is interrupted	//2017/11/20
	//double angleSum;
	//int angleCount;	//coreProcess period angle count, replace by rtSpeeds.size();	//2017/11/20
	int nImgCount;	//has 0/1/2 src imgs to match. when 2, begin to match process; when 0,1, wait the next src img
	int nFragments;

	void alarmThisWheel();
	//void resetCoreProcess();
	bool bIsInArea;
	bool bLastOUT;
	bool bWheelStopped;
	bool bStopProcess;
	//QTime time;
	//int in_out_time; //ms
	QMutex mutex;
	RobustMatcher rMatcher;

signals:
	//void resultReady(const QString &result);
	void imageProcessReady();
	void showAlarmNum(const QString &s);
	void showImageMatches();
	void setAlarmLight(PLCSerial::AlarmColor alarmcolor);
	void speedClcReady(double speed);
	void rtSpeedReady(double speed);
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
