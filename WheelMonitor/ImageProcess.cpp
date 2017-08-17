#include "stdafx.h"
#include "ImageProcess.h"
#include "HikVideoCapture.h"
#include "PLCSerial.h"
#include  <io.h>

using namespace std;
using namespace cv;

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

bool GenerateUniqueFileName(string& strRetName, string& strRetFullPath, const string& strFileFullPath)
{
	strRetFullPath = strFileFullPath;
	string::size_type stPos = strRetFullPath.rfind('/');
	if (stPos == strRetFullPath.npos)
		return false;
	strRetName = strRetFullPath.substr(stPos + 1);
	if (_access(strRetFullPath.c_str(), 0) != 0)
		return true;
	string strDir = strRetFullPath.substr(0, stPos + 1);
	string strFileName = strRetFullPath.substr(stPos + 1);
	string strExt;
	string::size_type pos = strFileName.rfind('.');
	if (pos != strRetFullPath.npos)
	{
		strExt = strFileName.substr(pos + 1);
		strFileName = strFileName.substr(0, pos);
	}
	int i = 1;
	char szBuff[11] = { 0 };
	do
	{
		sprintf_s(szBuff, 11, "(%d)", i++);
		strRetName = strFileName;
		if (!strExt.empty())
		{
			strRetName += szBuff;
			strRetName += ".";
			strRetName += strExt;
		}
		else
		{
			strRetName += szBuff;
		}
		strRetFullPath = strDir + strRetName;
	} while (_access(strRetFullPath.c_str(), 0) == 0);

	return true;
}



ImageProcess::ImageProcess(QObject *parent) : QObject(parent)
//, isSameWheel(false)
, angleCount(0)
, angleSum(0)
, iImgCount(0)
, stop(true)
, isSensorSameWheel(false)
, isWheelStop(false)
//50~80����ʶ�𲻵���С���ҵ��ܶ�Բ��Խ��Խ��//���׶�Բ���ۼ�������ֵ��ֵԽ�󣬼�⵽��ԲԽ������ֵԽС�����Լ�⵽������������ڵ�Բ
//, waitTimeout(0)
{

}


ImageProcess::~ImageProcess()
{
}
void ImageProcess::startImageProcess()
{
	mutex.lock();
	stop = false;
	mutex.unlock();
}
void ImageProcess::stopImageProcess()
{
	mutex.lock();
	stop = true;
	mutex.unlock();
}
void ImageProcess::sensorIN()
{
	isWheelStop = false;
	isSensorSameWheel = true;
}
void ImageProcess::sensorOUT()
{
	isSensorSameWheel = false;
}

void ImageProcess::wheelTimeout()
{
	isWheelStop = true;
}

void ImageProcess::doImageProcess()
{
	if (!stop)
	{
		if (!sensorTriggered)
		{
			//if timeout, which means this wheel is stop, drop the calc and wait for a come-in signal
			if (isWheelStop)
			{
				angleSum = 0;
				angleCount = 0;
				iImgCount = 0;

			}
			//if return 0, which means no cycle detected, calc the avgAngle for this wheel
			if (!coreImageProcess())
			{
				if (angleCount)
					getAvgAngle();
			}
		}
		else
		{
			//if sensor stop this wheel, calc the avgAngle
			if (!isSensorSameWheel)
			{
				if (!angleCount)
					qWarning("Wheel sensor off triggered without any wheels detected");
				else
					getAvgAngle();
			}
			coreImageProcess();
		}
	}
	emit imageProcessReady();
}

int ImageProcess::coreImageProcess()	//0-no wheel, 1-matches success, 2-wait next srcImg
{
	static QMutex mutex;
	static Mat srcImg;
	static Mat imgVessel;	//��������
	static Mat dstImg[2];		//Ŀ��ͼ��
	// wait until a image need process
	mutex.lock();
	srcImg = HikVideoCapture::pRawImage;
	mutex.unlock();
	//QTime time;
	//time.start();
	resize(srcImg, srcImg, Size(1280, 720), 0, 0, CV_INTER_LINEAR);

	Rect rect(200, 0, 800, 720);
	Mat roiImage = srcImg(rect);		  //srcImg(rect).copyTo(roiImage);
	Mat blurImage;//use blurimage to houghcircles


	//********************����Բ��Ⲣ�ָ�Բ��**************************//
	GaussianBlur(roiImage, blurImage, Size(gs1, gs1), 2, 2);//�޸��ں˴�С�ɸ���ʶ��Բ�����ף�ԽСԽ����
	vector<Vec3f> circles;
	//����Բ
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, dp, minDist, param1, param2, radius_min, radius_max); //���޸Ĳ������ڲ�׽Բ��׼ȷ��
	if (circles.size() < 1)
	{
		//	qDebug("No cycle:%d\n", ++iImgNoCycle);
		iImgCount = 0;
		return 0;
	}
	/*judge if the circle is out of ROI*/
	Point center0(cvRound(circles[0][0]), cvRound(circles[0][1])); //Բ������
	int radiusOutside = cvRound(circles[0][2]) - 10;	//Բ���⾶
	int radiusInside = radiusOutside / 1.5; //Բ���ھ�

											//if the circle is out of ROI, return
	if ((center0.x + radiusOutside) > roiImage.cols - 1 ||
		(center0.x - radiusOutside) < 0 ||
		(center0.y + radiusOutside) > roiImage.rows - 1 ||
		(center0.y - radiusOutside) < 0)
	{
		//	qDebug("Cycle out:%d\n", ++iImgNoCycle);
		iImgCount = 0;
		return 0;
	}
	iImgCount++;

	//qDebug() << circles.size;		//������Բ
	//qDebug() << QString("(x,y,r):%1 %2 %3").arg(circles[0][0]).arg(circles[0][1]).arg(circles[0][2]);	//Բ�뾶

	/********Ring split*********/
	Rect ringRect((center0.x - radiusOutside), (center0.y - radiusOutside),
		(2 * radiusOutside + 1), (2 * radiusOutside + 1));
	Mat imageRing = roiImage(ringRect);
	Mat mask = Mat::zeros(imageRing.size(), CV_8UC1);
	circle(mask, Point(mask.rows / 2, mask.cols / 2), radiusOutside, Scalar(255), -1, 8);
	bitwise_not(mask, mask);
	circle(mask, Point(mask.rows / 2, mask.cols / 2), radiusInside, Scalar(255), -1, 8);
	imageRing.setTo(0, mask);
	//imshow("mask", mask);
	//imshow("imageRing", imageRing);

	if (iImgCount == 1)
	{
		imgVessel = imageRing;
		return 2;
	}
	if (iImgCount == 2)
	{
		dstImg[0] = imgVessel;
		dstImg[1] = imageRing;
		imgVessel = imageRing;
		//iImgCount = 0;//drop the all image//discrete sampling
		iImgCount = 1;	//continius sampling
	}
	//�����εõ���Բ��ͼ�����Ϊͬ����С
	//resize the first dstImg same as the second
	//so that radius is right

	//resize(dstImg[0], dstImg[0], Size(dstImg[1].cols, dstImg[1].rows), 0, 0, CV_INTER_LINEAR);

	//**************************ͼ������ƥ�䲿��*****************************************//

	/******************ORB�㷨*********************/
	Ptr<ORB> orb = ORB::create(600);
	vector<KeyPoint> allkeypoints1, allkeypoints2;
	orb->detect(dstImg[0], allkeypoints1);
	orb->detect(dstImg[1], allkeypoints2);
	vector<KeyPoint> keypoints1, keypoints2;	//��allkeypoints�з���Բ����Χ��ɸѡ����

	for (int i = 0; i < allkeypoints1.size(); i++)
	{
		int temp = (allkeypoints1[i].pt.x - dstImg[0].cols / 2) * (allkeypoints1[i].pt.x - dstImg[0].cols / 2)
			+ (allkeypoints1[i].pt.y - dstImg[0].rows / 2) * (allkeypoints1[i].pt.y - dstImg[0].rows / 2);
		if (temp > ((radiusInside + 10) * (radiusInside + 10)) && temp < ((radiusOutside - 10) * (radiusOutside - 10)))
		{
			keypoints1.push_back(allkeypoints1[i]);
			//circle(dstImg[0], allkeypoints1[i].pt, 2, Scalar(255, 255, 255));
		}
	}
	for (int i = 0; i < allkeypoints2.size(); i++)
	{
		int temp = (allkeypoints2[i].pt.x - dstImg[1].cols / 2) * (allkeypoints2[i].pt.x - dstImg[1].cols / 2)
			+ (allkeypoints2[i].pt.y - dstImg[1].rows / 2) * (allkeypoints2[i].pt.y - dstImg[1].rows / 2);
		if (temp > ((radiusInside + 10) * (radiusInside + 10)) && temp < ((radiusOutside - 10) * (radiusOutside - 10)))
		{
			keypoints2.push_back(allkeypoints2[i]);
			//circle(dstImg[1], allkeypoints2[i].pt, 2, Scalar(255, 255, 255));
		}
	}
	//imshow("dstImg[0]", dstImg[0]);
	//imshow("dstImg[1]", dstImg[1]);

	Mat descriptors1, descriptors2;
	orb->compute(dstImg[0], keypoints1, descriptors1);
	orb->compute(dstImg[1], keypoints2, descriptors2);

	//����ƥ��
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");

	//Lowe's algorithm
	//�����㷨ѡ�������ƥ��
	vector< vector<DMatch> > matches;
	matcher->knnMatch(descriptors1, descriptors2, matches, 2);
	vector<DMatch> good_matches;
	//������ؼ���֮���������ֵ����Сֵ
	/*
	double max_dist = 0;
	double min_dist = 50;
	for (int i = 0; i < matches.size(); i++) {
	double dist = matches[i][0].distance;
	if (dist < min_dist)
	min_dist = dist;
	if (dist > max_dist)
	max_dist = dist;
	}
	*/
	//qDebug() << "min_dist" << min_dist;
	for (unsigned i = 0; i < matches.size(); i++) {
		if (matches[i][0].distance < 0.6 * matches[i][1].distance//0.6
			) {
			//&&matches[i][0].distance < s * min_dist
			good_matches.push_back(matches[i][0]);
		}
	}
	//�ж���Чƥ�����Ƿ���ڵ���10��
	if (good_matches.size() < 3)
	{
		//qDebug("the match point less than 3");
		return 2;
	}


	//���Ƴ�ƥ�䵽�Ĺؼ���
	Mat image_matches;
	drawMatches(dstImg[0], keypoints1, dstImg[1], keypoints2,
		good_matches, image_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");//ddd is weekday
	string fullpath = QStringLiteral("D:/Capture/%1.jpg").arg(dateTime).toStdString();
	string retname, retpath;
	GenerateUniqueFileName(retname, retpath, fullpath);
	imwrite(retpath, image_matches);
	//��ʾ���ս��
	mutex.lock();
	imageMatches = image_matches;
	emit showImageMatches();
	mutex.unlock();
	//imshow("������ƥ��ͼ", image_matches);
	//qDebug() << "match" << time.elapsed() / 1000.0 << "s";
	//waitKey(30);



	//***********************�������任���󣬼�����ת�Ƕ�**************************//
	//���������ֲ�����
	vector<Point2f> obj;
	vector<Point2f> scene;

	//��ƥ��ɹ���ƥ����л�ȡ�ؼ���
	for (unsigned int i = 0; i < good_matches.size(); i++) {
		obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
	}

	//�������ŷ���任����
	Mat M = estimateRigidTransform(obj, scene, false);
	if (M.data == NULL)
		return 2;

	//������ת�Ƕ�
	//�����1�е�2����
	double oneAngle = -asin(M.at<double>(0, 1)) / M_PI * 180;
	double realSpeed = oneAngle * angle2Speed;
	qDebug() << "realtime speed: " << realSpeed;
	emit realSpeedReady(realSpeed);
	angleCount++;
	angleSum += oneAngle;
	//isSameWheel = true;
	/********compute finish*********/
	return 1;
}

void ImageProcess::getAvgAngle()
{
	//calculate the avg angle
	double avgAngle = angleSum / angleCount;
	double lastAvgSpeed = avgAngle * angle2Speed;
	emit speedClcReady(lastAvgSpeed);
	qWarning("speed: %.2lfm/min", lastAvgSpeed);
	qDebug() << "MatchCount: " << angleCount;
	if (avgAngle < angleLowThreshold)
	{
		emit setAlarmLight(ALARM_LIGHT_RED);
		qWarning() << QStringLiteral("��SLOW��");
		qCritical("SLOW");
	}
	else if (avgAngle > angleHighThreshold)
	{
		emit setAlarmLight(ALARM_LIGHT_YELLOW);
		qWarning() << QStringLiteral("��FAST��");
		qCritical("FAST");
	}
	angleSum = 0;
	angleCount = 0;
	iImgCount = 0;
}
