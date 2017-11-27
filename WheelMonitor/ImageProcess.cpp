#include "stdafx.h"
#include "ImageProcess.h"
#include "HikVideoCapture.h"
#include "PLCSerial.h"
#include "outlierdetection.h"	//used for calc mean, replace opencv lib
#include "datatablewidget.h"
#include "common.h"
//#include  <io.h>

using namespace std;
using namespace cv;

ImageProcessParameters::ImageProcessParameters() :angle2Speed(60 * (M_PI*0.650 / 360) / (8.0 / 25.0)),
angleBigRatio(1.2),
angleSmallRatio(0.8),
radius_max(350),
radius_min(250),
gs1(7),
gs2(7),
dp(1),
minDist(90),
param1(300),
param2(60),
sensorTriggered(false),
roiRect(220, 0, 800, 720)
{
}

ImageProcessParameters ImageProcess::g_imgParam;

bool getUniqueFile(QString &fullFileName)
{
	QFileInfo fileInfo(fullFileName);
	QString baseName_src = fileInfo.baseName();
	for (int i = 1; i < 11; i++) //max 10 repeat
	{
		if (!fileInfo.exists())
		{
			//not existing means current filename is unique
			return true;
		}
		else
		{
			QString newBaseName(QStringLiteral("%1(%2)").arg(baseName_src).arg(i));
			fullFileName = QStringLiteral("%1/%2.%3").arg(fileInfo.absolutePath()).arg(newBaseName).arg(fileInfo.completeSuffix());
			//sure file has suffix, omit this judgement.
			/*if (!fileInfo.completeSuffix().isEmpty())
				fullFileName += "." + fileInfo.completeSuffix();*/
			fileInfo = QFileInfo(fullFileName);
		}
	}
	return false;
}

ImageProcess::ImageProcess(QSqlTableModel *srcmodel, QObject *parent)
	: QObject(parent)
	//, isSameWheel(false)
	//, angleCount(0)
	//, angleSum(0)
	, nImgCount(0)
	//, in_out_time(0)
	, nFragments(0)
	, bStopProcess(true)
	, bLastOUT(true)	//assum ths wheel is out at initial, won't be set to true until a new wheel comes in.
	, bIsInArea(false)
	, bWheelStopped(false)
	//, waitTimeout(0)
{
	model = srcmodel;
}

ImageProcess::~ImageProcess()
{
}
void ImageProcess::startImageProcess()
{
	//mutex.lock();
	bStopProcess = false;
	//mutex.unlock();
}
void ImageProcess::stopImageProcess()
{
	//mutex.lock();
	bStopProcess = true;
	//mutex.unlock();
}
void ImageProcess::sensorIN()
{
	mutex.lock();
	bWheelStopped = false;
	bLastOUT = false;
	bIsInArea = true;
	mutex.unlock();
	//time.start();
}
void ImageProcess::sensorOUT()
{
	//mutex.lock();
	bIsInArea = false;
	//mutex.unlock();
	//in_out_time = time.elapsed();
}

void ImageProcess::wheelTimeout()
{
	mutex.lock();
	if (!bWheelStopped)
	{
		qWarning("Cart is stopped");
		bWheelStopped = true;
	}
	mutex.unlock();
}

void ImageProcess::doImageProcess()
{
	if (!bStopProcess)
	{
		if (bWheelStopped) //if timeout, which means this wheel is stop, drop the calc and wait for a come-in signal
		{//if wheel stop, stop the process to avoid meaningless count
			alarmThisWheel();
		}

		/*sensor triggered*/
		else
		{//decide if the wheel comes in/out by SENSOR IN/OUT
			if (bIsInArea)			//in detect area
			{//inarea(1)/out(1->0)/lastout(0)
				static bool lastCore = false;	//record the last return of core process
				bool nowCore = coreImageProcess();	//0->1, Fragment++ at rise edge, so it can be 0
				if (nowCore == true && lastCore == false)
				{
					nFragments++;
				}
				lastCore = nowCore;
			}
			else if (!bLastOUT)			//out detect area
			{//Once a wheel scoll out the detect area, settle it.
				bLastOUT = true;	//only triggered once, until the next sensorIN set it to false
				alarmThisWheel();
			}
		}
	}
	emit imageProcessReady(); //emit ready anyway
	return;
}

void ImageProcess::alarmThisWheel()
{
	/*ocr*/
	qDebug() << "total result in imageprocess" << myocr.result.size();
	if (myocr.result.empty())
	{
		qDebug() << "no plates ";
	}
	else
	{
		myocr.get_final_result();
		qDebug() << " plate number : " << QString::fromStdString(myocr.final_result);
	}
	/*speed*/
	if (!rtSpeeds.size())
	{//no any matches, coreImageProcess returned no one
		qWarning() << QStringLiteral("Miss test");
	}
	else
	{
		int totalMatchCount = rtSpeeds.size();
		vector<double> rtSpeedDiffs(rtSpeeds.size());//a vector saves speed difference between imgprocess and PLC(speedAD)
		transform(rtSpeeds.begin(), rtSpeeds.end(), refSpeeds.begin(), rtSpeedDiffs.begin(), minus<double>());	//rt-ref=diff
		OutlierDetection outlier;
		outlier.grubbs(rtSpeedDiffs);	//kick out the bad results by grubbs certai
		double meanRef = outlier.mean(refSpeeds);
		double meanDiff = outlier.mean(rtSpeedDiffs);
		double meanSpeed = meanRef + meanDiff;	//mean linear Velocity
		emit speedClcReady(meanSpeed);
		qDebug("Num:%s speed: %.2lfm/min", myocr.final_result, meanSpeed);
		qDebug() << "Total Match: " << totalMatchCount << "; Valid: " << rtSpeedDiffs.size() << "; fragments: " << nFragments;
		if (nFragments != 1)
		{
			qDebug() << QString("detect count error: %1").arg(nFragments);
		}

		double high = (g_imgParam.angleBigRatio - 1) * meanRef;		//high>0 higher deviation
		double low = (g_imgParam.angleSmallRatio - 1) * meanRef;	//low<0 lower deviation
		if (meanDiff < low)
		{
			if (totalMatchCount >= 5)
			{//Only if match enough, the result is reliable, then alarm
				emit setAlarmLight(PLCSerial::AlarmColorRed);
				emit showAlarmNum(QString::fromStdString(myocr.final_result));
			}
			else {
				emit setAlarmLight(PLCSerial::AlarmColorYellow);
			}
			qWarning() << QStringLiteral("SLOW");
			qCritical() << QString::fromStdString(myocr.final_result) << "SLOW";
		}
		else if (meanDiff > high)
		{
			emit setAlarmLight(PLCSerial::AlarmColorYellow);
			qWarning() << QStringLiteral("FAST");
			qCritical() << QString::fromStdString(myocr.final_result) << "FAST";	//show log in the alarmtab
		}
	}
	//when wheel is leaving the detect area
	nFragments = 0;	//1. reset detectcount
	rtSpeeds.clear();	//2. clear rtSpeeds vector
	refSpeeds.clear();
	myocr.resetOcr();
	nImgCount = 0;	//no need	//once settle a wheel, force to zero
}

int ImageProcess::coreImageProcess() //0-no wheel, 1-matches success, 2-wait next srcImg
{
	static QMutex mutex;
	static Mat srcImg;
	static Mat imgCache, dstImg[2]; //????????????//??????
	static Mat maskCache, mask[2];

	//int debugI = 0;

	mutex.lock();
	srcImg = HikVideoCapture::pRawImage;
	//realtime veloctiy difference between imgprocess and PLC(speedAD)
	double refSpeed = PLCSerial::speedAD;
	mutex.unlock();
	myocr.core_ocr(srcImg); //card num detect gzy 2017/11/9
	resize(srcImg, srcImg, Size(1280, 720), 0, 0, CV_INTER_LINEAR);
	//Rect rect(220, 0, 800, 720);
	Mat roiImage = srcImg(g_imgParam.roiRect); //srcImg(rect).copyTo(roiImage);
	Mat blurImage;					//use blurimage for houghcircles, use sourceimage for matches
	//********************?????????????**************************//
	GaussianBlur(roiImage, blurImage, Size(g_imgParam.gs1, g_imgParam.gs1), 2, 2); //???????§³?????????????????§³?????
	vector<Vec3f> circles;
	//????
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, g_imgParam.dp, g_imgParam.minDist, g_imgParam.param1, g_imgParam.param2, g_imgParam.radius_min, g_imgParam.radius_max); //?????????????????????
	if (circles.size() < 1)
	{
		nImgCount = 0;
		return 0;
	}
	//qDebug() << ++debugI;

	/***judge if the circle is out of ROI**/
	Point center0(cvRound(circles[0][0]), cvRound(circles[0][1])); //center of the circle
	int radiusOutside = cvRound(circles[0][2]);					   //Ro
	int radiusInside = radiusOutside / 1.6;						   //Ri
	if ((center0.x + radiusOutside) > roiImage.cols || (center0.x - radiusOutside) < 0 ||
		(center0.y + radiusOutside) > roiImage.rows || (center0.y - radiusOutside) < 0)
	{
		nImgCount = 0;
		return 0;
	}
	nImgCount++;
	/********Ring mask*********/
	Rect ringRect((center0.x - radiusOutside), (center0.y - radiusOutside), 2 * radiusOutside, 2 * radiusOutside);
	Mat imageRing = roiImage(ringRect);
	Mat maskTmp = rMatcher.getMask(imageRing.size(), radiusOutside - 10, radiusInside + 10); //??????????????????§³10??????

	//test: output all the imageRing //2017/11/23
	//QString nowDate = QDate::currentDate().toString("yyyyMMdd");
	//QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	//QString fullFilePath = QStringLiteral("D:/Capture/%1/S%2.jpg").arg(nowDate).arg(nowTime);
	//getUniqueFile(fullFilePath);
	//imwrite(fullFilePath.toStdString(), imageRing);

	if (nImgCount == 1)
	{
		imgCache = imageRing;
		maskCache = maskTmp;
		return 2;
	}
	if (nImgCount == 2)
	{
		dstImg[0] = imgCache;
		dstImg[1] = imageRing;
		imgCache = imageRing;
		mask[0] = maskCache;
		mask[1] = maskTmp;
		maskCache = maskTmp;
		//nImgCount = 0;//drop the all image//discrete sampling
		nImgCount = 1; //continius sampling
	}
	//qDebug() << ++debugI;

	//**************************orb matches**************************//

	Mat image_matches;
	double oneAngle;
	if (!rMatcher.match(dstImg[0], dstImg[1], mask[0], mask[1], image_matches, oneAngle))
		return 2;

	//angle to linear velocity
	double rtSpeed = oneAngle * g_imgParam.angle2Speed;
	qWarning("imageprocess Speed: %.2lf; refSpeed: %.2lf", rtSpeed, refSpeed);
	emit rtSpeedReady(rtSpeed);
	rtSpeeds.push_back(rtSpeed);
	refSpeeds.push_back(refSpeed);
	if (rtSpeed < 0.05)
	{//if wheelstop, end and settle this wheel.
		//won't stop the videocap save, until 100s
		bWheelStopped = true;
	}

	//save matches
	QString nowDate = QDate::currentDate().toString("yyyyMMdd");
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	QString fullFilePath = QStringLiteral("%1/%2/%3.jpg").arg(captureDirPath).arg(nowDate).arg(nowTime);
	getUniqueFile(fullFilePath);
	imwrite(fullFilePath.toStdString(), image_matches);
	//qDebug() << ++debugI;

	//show matches in UI
	mutex.lock();
	imageMatches = image_matches;
	mutex.unlock();
	emit showImageMatches();
	//qDebug() << ++debugI;
	return 1;
}