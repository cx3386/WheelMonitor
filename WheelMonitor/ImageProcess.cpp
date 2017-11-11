#include "stdafx.h"
#include "ImageProcess.h"
#include "HikVideoCapture.h"
#include "PLCSerial.h"
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
			//if not exists, then unique
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

ImageProcess::ImageProcess(QObject *parent) : QObject(parent)
											  //, isSameWheel(false)
											  ,
											  angleCount(0), angleSum(0), iImgCount(0), bStopProcess(true), bLastOUT(true) //�ٶ�����������
											  ,
											  bIsInArea(false), bWheelStopped(false)
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
	time.start();
}
void ImageProcess::sensorOUT()
{
	//mutex.lock();
	bIsInArea = false;
	//mutex.unlock();
	in_out_time = time.elapsed();
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
		{
			resetCoreProcess();
		}
		else if (!g_imgParam.sensorTriggered) //only for test, haven't done.	// 2017-10-27
		{
			//if return 0, which means no cycle detected, calc the avgAngle for this wheel
			//����ȴ���һ֡����
			if (!coreImageProcess())
			{
				if (angleCount)
				{
					alarm(angleSum / angleCount);
					resetCoreProcess();
				}
			}
		}
		else //�ź����������ӽ���(SensorIN)�ˣ��ſ�ʼ�������ݣ�
			 //һ��Բ��ʧ��ROI�У�����ʾ���,�����Բ������жϣ���������2֡����ƥ��ɹ��Ļ����п�����ʾ����������
			 //�����ӳ�ȥ(SensorOut)��ֹͣ������ʾ���������еĻ�����
		{
			if (bIsInArea)
			{
				static bool lastCore = false;
				bool nowCore = coreImageProcess();
				if (nowCore == false && lastCore == true)
					nDetectCount++;
				lastCore = nowCore;
			}
			else if (!bLastOUT) //��ȥ��ʱ��
								//Once wheel scoll out detect area, calc the avgAngle(if anglecout > 0);
								//if no avgangle ever, emit miss
			{
				bLastOUT = true;
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

				if (angleCount)
				{
					alarm(angleSum / angleCount);
				}
				//�������뿪֮ǰû�в���κ�ƽ���ٶȣ���miss
				else
				{
					qWarning() << QStringLiteral("Miss test");
				}
				if (nDetectCount != 1) 
				{
					qDebug() << QString("detect count error: %1").arg(nDetectCount);
				}
				nDetectCount = 0; //ÿ�ν�detectcount����
				myocr.resetOcr();
			}
		}
	}
	emit imageProcessReady(); //emit ready anyway
	return;
}

void ImageProcess::resetCoreProcess()
{
	angleSum = 0;
	angleCount = 0;
	iImgCount = 0;
}

void ImageProcess::alarm(double avgAngle)
{
	double lastAvgSpeed = avgAngle * g_imgParam.angle2Speed;
	emit speedClcReady(lastAvgSpeed);
	qWarning("Num:%s speed: %.2lfm/min", myocr.final_result, lastAvgSpeed);
	qDebug() << "MatchCount: " << angleCount;
	double length = avgAngle * in_out_time;
	//qDebug("length: %.2lf", length);
	double high = g_imgParam.angleBigRatio * 86000; //��λ��
	double low = g_imgParam.angleSmallRatio * 86000;
	if (length < low)
	{
		emit setAlarmLight(PLCSerial::AlarmColorRed);
		emit showAlarmNum(QString::fromStdString(myocr.final_result));
		qWarning() << QStringLiteral("SLOW");
		qCritical("SLOW");
	}
	else if (length > high)
	{
		emit setAlarmLight(PLCSerial::AlarmColorYellow);
		qWarning() << QStringLiteral("FAST");
		qCritical("FAST");	//show log in the alarmtab
	}
	resetCoreProcess();
}

int ImageProcess::coreImageProcess() //0-no wheel, 1-matches success, 2-wait next srcImg
{
	static QMutex mutex;
	static Mat srcImg;
	static Mat imgVessel, dstImg[2]; //����ˢ�µ�����//Ŀ��ͼ��
	static Mat maskVessel, mask[2];

	//int debugI = 0;

	mutex.lock();
	srcImg = HikVideoCapture::pRawImage;
	mutex.unlock();
	myocr.core_ocr(srcImg); //card num detect gzy 2017/11/9
	resize(srcImg, srcImg, Size(1280, 720), 0, 0, CV_INTER_LINEAR);
	//Rect rect(220, 0, 800, 720);
	Mat roiImage = srcImg(g_imgParam.roiRect); //srcImg(rect).copyTo(roiImage);
	Mat blurImage;					//use blurimage for houghcircles, use sourceimage for matches
	//********************���Բ��Ⲣ�ָ�Բ��**************************//
	GaussianBlur(roiImage, blurImage, Size(g_imgParam.gs1, g_imgParam.gs1), 2, 2); //�޸��ں˴�С�ɸ���ʶ��Բ�����ף�ԽСԽ����
	vector<Vec3f> circles;
	//���Բ
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, g_imgParam.dp, g_imgParam.minDist, g_imgParam.param1, g_imgParam.param2, g_imgParam.radius_min, g_imgParam.radius_max); //���޸Ĳ������ڲ�׽Բ��׼ȷ��
	if (circles.size() < 1)
	{
		iImgCount = 0;
		return 0;
	}
	//qDebug() << ++debugI;

	/***judge if the circle is out of ROI**/
	Point center0(cvRound(circles[0][0]), cvRound(circles[0][1])); //Բ������
	int radiusOutside = cvRound(circles[0][2]);					   //Բ���⾶
	int radiusInside = radiusOutside / 1.6;						   //Բ���ھ�
	if ((center0.x + radiusOutside) > roiImage.cols || (center0.x - radiusOutside) < 0 ||
		(center0.y + radiusOutside) > roiImage.rows || (center0.y - radiusOutside) < 0)
	{
		iImgCount = 0;
		return 0;
	}
	iImgCount++;
	/********Ring mask*********/
	Rect ringRect((center0.x - radiusOutside), (center0.y - radiusOutside), 2 * radiusOutside, 2 * radiusOutside);
	Mat imageRing = roiImage(ringRect);
	Mat maskTmp = rMatcher.getMask(imageRing.size(), radiusOutside - 10, radiusInside + 10); //����ƥ������Բ������С10������
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
		iImgCount = 1; //continius sampling
	}
	//qDebug() << ++debugI;

	//**************************ͼ������ƥ�䲿��*****************************************//

	Mat image_matches;
	double oneAngle;
	if (!rMatcher.match(dstImg[0], dstImg[1], mask[0], mask[1], image_matches, oneAngle))
		return 2;

	//����Ƕ�
	double realSpeed = oneAngle * g_imgParam.angle2Speed;
	qDebug() << "realtime speed: " << realSpeed;
	emit realSpeedReady(realSpeed);
	angleCount++;
	angleSum += oneAngle;

	//����matchͼƬ
	QString nowDate = QDateTime::currentDateTime().toString("yyyyMMdd");
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	QString fullFilePath = QStringLiteral("D:/Capture/%1/%2.jpg").arg(nowDate).arg(nowTime);
	getUniqueFile(fullFilePath);
	imwrite(fullFilePath.toStdString(), image_matches);
	//qDebug() << ++debugI;

	//��ʾƥ����
	mutex.lock();
	imageMatches = image_matches;
	mutex.unlock();
	emit showImageMatches();
	//qDebug() << ++debugI;
	return 1;
}
