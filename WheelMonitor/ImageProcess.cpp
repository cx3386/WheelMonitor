#include "stdafx.h"
#include "ImageProcess.h"
#include "HikVideoCapture.h"
#include "PLCSerial.h"
//#include  <io.h>

using namespace std;
using namespace cv;

double ImageProcess::angle2Speed = 60 * (M_PI * 0.650 / 360) / (8.0 / 25.0);
double ImageProcess::angleHighThreshold = 3.0;
double ImageProcess::angleLowThreshold = 2.0;
int ImageProcess::radius_max = 300;
int ImageProcess::radius_min = 200;
int ImageProcess::gs1 = 7;
double ImageProcess::dp = 1;
double ImageProcess::minDist = 90;
double ImageProcess::param1 = 300;
double ImageProcess::param2 = 60;
bool ImageProcess::sensorTriggered = false;

bool getUniqueFile(QString& fullFileName)
{
	QFileInfo fileInfo(fullFileName);
	QString baseName_src = fileInfo.baseName();
	for (int i = 1; i < 11; i++) //max 10 repeat
	{
		if (!fileInfo.exists()) {
			//if not exists, then unique
			return true;
		}
		else {
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

ImageProcess::ImageProcess(QObject *parent) : QObject(parent)
//, isSameWheel(false)
, angleCount(0)
, angleSum(0)
, iImgCount(0)
, nDetectCount(0)
, bStopProcess(true)
, bLastOUT(true)//假定轮子在外面
, bIsInArea(false)
, bWheelStopped(false)
//, waitTimeout(0)
{
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
}
void ImageProcess::sensorOUT()
{
	//mutex.lock();
	bIsInArea = false;
	//mutex.unlock();
}

void ImageProcess::wheelTimeout()
{
	//mutex.lock();
	bWheelStopped = true;
	//mutex.unlock();
}

void ImageProcess::doImageProcess()
{
	if (!bStopProcess)
	{
		if (bWheelStopped)	//if timeout, which means this wheel is stop, drop the calc and wait for a come-in signal
		{
			angleSum = 0;
			angleCount = 0;
			iImgCount = 0;
			qWarning("cart stay in the detect area");
		}
		else if (!sensorTriggered)
		{
			//if return 0, which means no cycle detected, calc the avgAngle for this wheel
			//否则等待下一帧处理
			if (!coreImageProcess())
			{
				if (angleCount)
					getAvgAngle();
			}
		}
		else  //信号驱动：轮子进来(SensorIN)了，才开始处理数据；
			//一旦圆消失在ROI中，则显示结果,（如果圆侦测有中断，并且又有2帧以上匹配成功的话，有可能显示多个结果）；
			//当轮子出去(SensorOut)，停止处理，显示结果（如果有的话）。
		{
			if (bIsInArea)
			{
				if (!coreImageProcess())
				{
					if (angleCount)
					{
						getAvgAngle();
						nDetectCount++;
					}
				}
			}
			else if (!bLastOUT)	//出去的时刻
			//Once wheel scoll out detect area, calc the avgAngle(if anglecout > 0);
			//if no avgangle ever, emit miss
			{
				bLastOUT = true;
				if (angleCount)
				{
					getAvgAngle();
					nDetectCount++;
				}
				//在轮子离开之前没有测得任何平均速度，报miss
				if (!nDetectCount)
					qWarning() << QStringLiteral("↑Miss test↑");
				//在轮子离开之前测得了2次以上的平均速度，说明断连了
				if (nDetectCount != 1)
					qDebug() << "detect count Error: " << nDetectCount;
				nDetectCount = 0;	//每次将detectcount置零
			}
		}
	}
	emit imageProcessReady();	//emit ready anyway
	return;
}

int ImageProcess::coreImageProcess()	//0-no wheel, 1-matches success, 2-wait next srcImg
{
	static QMutex mutex;
	static Mat srcImg;
	static Mat imgVessel, dstImg[2];	//不断刷新的容器//目标图像
	static Mat maskVessel, mask[2];

	//int debugI = 0;

	mutex.lock();
	srcImg = HikVideoCapture::pRawImage;
	mutex.unlock();

	resize(srcImg, srcImg, Size(1280, 720), 0, 0, CV_INTER_LINEAR);
	Rect rect(220, 0, 800, 720);
	Mat roiImage = srcImg(rect);		  //srcImg(rect).copyTo(roiImage);
	Mat blurImage;//use blurimage for houghcircles, use sourceimage for matches

	//********************霍夫圆检测并分割圆环**************************//
	GaussianBlur(roiImage, blurImage, Size(gs1, gs1), 2, 2);//修改内核大小可更改识别圆的难易，越小越容易
	vector<Vec3f> circles;
	//霍夫圆
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, dp, minDist, param1, param2, radius_min, radius_max); //可修改参数调节捕捉圆的准确度
	//HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, dp, minDist, 110, 40, radius_min, radius_max); //可修改参数调节捕捉圆的准确度
	if (circles.size() < 1)
	{
		iImgCount = 0;
		return 0;
	}
	//qDebug() << ++debugI;

	/***judge if the circle is out of ROI**/
	Point center0(cvRound(circles[0][0]), cvRound(circles[0][1])); //圆环中心
	int radiusOutside = cvRound(circles[0][2]);	//圆环外径
	int radiusInside = radiusOutside / 1.6; //圆环内径
	if ((center0.x + radiusOutside) > roiImage.cols || (center0.x - radiusOutside) < 0 ||
		(center0.y + radiusOutside) > roiImage.rows || (center0.y - radiusOutside) < 0)
	{
		iImgCount = 0;
		return 0;
	}
	iImgCount++;
	//qDebug() << ++debugI;

	/********Ring mask*********/
	Rect ringRect((center0.x - radiusOutside), (center0.y - radiusOutside), 2 * radiusOutside, 2 * radiusOutside);
	Mat imageRing = roiImage(ringRect);
	Mat maskTmp = rMatcher.getMask(imageRing.size(), radiusOutside - 10, radiusInside + 10);	//特征匹配区往圆环内缩小10个像素
	if (iImgCount == 1)
	{
		imgVessel = imageRing;
		maskVessel = maskTmp;
		return 2;
	}
	if (iImgCount == 2)
	{
		dstImg[0] = imgVessel;
		dstImg[1] = imageRing;
		imgVessel = imageRing;
		mask[0] = maskVessel;
		mask[1] = maskTmp;
		maskVessel = maskTmp;
		//iImgCount = 0;//drop the all image//discrete sampling
		iImgCount = 1;	//continius sampling
	}
	//qDebug() << ++debugI;

	//**************************图像特征匹配部分*****************************************//

	Mat image_matches;
	double oneAngle;
	if (!rMatcher.match(dstImg[0], dstImg[1], mask[0], mask[1], image_matches, oneAngle))
		return 2;

	//qDebug() << ++debugI;

	//计算角度
	double realSpeed = oneAngle * angle2Speed;
	qDebug() << "realtime speed: " << realSpeed;
	emit realSpeedReady(realSpeed);
	angleCount++;
	angleSum += oneAngle;
	//qDebug() << ++debugI;

	//保存match图片
	QString nowDate = QDateTime::currentDateTime().toString("yyyyMMdd");
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	QString fullFilePath = QStringLiteral("D:/Capture/%1/%2.jpg").arg(nowDate).arg(nowTime);
	getUniqueFile(fullFilePath);
	imwrite(fullFilePath.toStdString(), image_matches);
	//qDebug() << ++debugI;

	//显示匹配结果
	mutex.lock();
	imageMatches = image_matches;
	mutex.unlock();
	emit showImageMatches();
	//qDebug() << ++debugI;
	return 1;
}

void ImageProcess::getAvgAngle()
{
	//qDebug() << "100";
	//calculate the avg angle
	double avgAngle = angleSum / angleCount;
	double lastAvgSpeed = avgAngle * angle2Speed;
	emit speedClcReady(lastAvgSpeed);
	qWarning("speed: %.2lfm/min", lastAvgSpeed);
	qDebug() << "MatchCount: " << angleCount;
	if (avgAngle < angleLowThreshold)
	{
		emit setAlarmLight(PLCSerial::AlarmColorRed);
		qWarning() << QStringLiteral("↑SLOW↑");
		qCritical("SLOW");
	}
	else if (avgAngle > angleHighThreshold)
	{
		emit setAlarmLight(PLCSerial::AlarmColorYellow);
		qWarning() << QStringLiteral("↑FAST↑");
		qCritical("FAST");
	}
	angleSum = 0;
	angleCount = 0;
	iImgCount = 0;
	//qDebug() << "101";
}