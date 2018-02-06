#include "stdafx.h"
#include "imageprocess.h"
#include "common.h"
#include "database.h"
#include "hikvideocapture.h"
#include "outlierhelper.h" //used for calc mean, replace opencv lib
#include "plcserial.h"
#include "ocr.h"

using namespace std;
using namespace cv;

ImageProcessParameters::ImageProcessParameters() : angle2Speed(60 * (M_PI * 0.650 / 360) / (8.0 / 25.0)),
alarmRatio(0.05),
warningRatio(0.10),
radius_max(250),
radius_min(200),
gs1(9),
gs2(9),
dp(2),
minDist(180),
param1(200),
param2(100),
sensorTriggered(false),
roiRect(220, 0, 800, 720)
{
}

ImageProcessParameters ImageProcess::g_imgParam;

ImageProcess::ImageProcess(QObject *parent /*= Q_NULLPTR*/) : QObject(parent)
, nImgCount(0)
, nFragments(0)
, bStopProcess(true)
, bIsInArea(false)
, bWheelStopped(false)
, ocr(new OCR)
{
	connect(this, &ImageProcess::initModel, this, &ImageProcess::setupModel);
}

ImageProcess::~ImageProcess()
= default;

void ImageProcess::doImageProcess()
{
	foreverPreProcess();
	mutex.lock(); //prevent reading the frameToShow simultaneously
	calibratedFrame.copyTo(frameToShow); //here we use deep copy, otherwise we will change calibratedFrame when change frameToshow
	cvtColor(frameToShow, frameToShow, CV_GRAY2BGR);
	rectangle(frameToShow, g_imgParam.roiRect, Scalar(0, 0, 255), 2);
	mutex.unlock();
	if (!bStopProcess)
	{
		//when wheel is stop,
		//1. stop the process to avoid meaningless count
		//2. drop the calc results and wait for a come-in signal.
		//this usually happens when wheel timeout, or the speed is detected as zero
		if (bWheelStopped)
		{
			alarmThisWheel();
		}

		else
		{
			/*sensor triggered*/
			//decide if the wheel comes in/out by SENSOR IN/OUT
			if (g_imgParam.sensorTriggered)
			{
				static bool lastIsIn = false;	//the wheel is in(1) or out(0) of the detect area. when 1->0(falling edge), which means the wheel is scrolling out, alarm this wheel.
												//assume wheel is out at initial, won't be set to true until a new wheel comes in.
				if (bIsInArea)						   //in detect area
				{
					static bool lastCore = false;	  //record the last return of core process
					bool nowCore = coreImageProcess(); //0->1, Fragment++ at rise edge. NOTE: it can be 0, which means no circle detected
					if (nowCore&(!lastCore)) //nowCore == true && lastCore == false
					{
						nFragments++;
					}
					lastCore = nowCore;
				}
				else if (lastIsIn) //lastIsIn==true && bIsInArea==false,which means wheel is out of detect area
				{
					//Once a wheel scroll out the detect area, settle it.
					alarmThisWheel();
				}
				lastIsIn = bIsInArea;
			}
			/*image triggered*/
			//for test manually, independent on cameracapsave
			else
			{
				static bool lastCore = false;
				bool nowCore = coreImageProcess();
				if (nowCore && !lastCore) //0->1
				{
					nFragments++;
				}
				else if (!nowCore&&lastCore) //1->0
				{
					alarmThisWheel(); //be careful if it cost too much time(>320ms)
				}
				lastCore = nowCore;
			}
		}
	}
	emit imageProcessReady();
	emit showRealtimeImage();
}

int ImageProcess::coreImageProcess() //0-no wheel, 1-matches success, 2-wait next srcImg
{
	static Mat imgCache, dstImg[2];
	static Mat maskCache, mask[2];

	ocr->core_ocr(srcImg); //card num detect gzy 2017/11/9
	Mat roiImage = calibratedFrame(g_imgParam.roiRect); //srcImg(rect).copyTo(roiImage);
	equalizeHist(roiImage, roiImage); //equalize the image, this will change calibratedFrame//2017/12/21
	Mat blurImage;							   //use blur image for hough circles, use source image for matches

	/************************************************************************/
	/* hough circles                                                                     */
	/************************************************************************/
	// Reduce the noise so we avoid false circle detection
	GaussianBlur(roiImage, blurImage, Size(g_imgParam.gs1, g_imgParam.gs1), 2, 2); //the greater the kernel size is, the less time the hough cost
	// Canny show
	//Mat detectEdges;
	//Canny(blurImage, detectEdges, g_imgParam.param1 / 2, g_imgParam.param1, 3);
	//imshow("edges", detectEdges);
	// Apply the Hough Transform to find the circles. //cost 40ms
	vector<Vec3f> circles;
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, g_imgParam.dp, g_imgParam.minDist, g_imgParam.param1, g_imgParam.param2, g_imgParam.radius_min, g_imgParam.radius_max);
	if (circles.empty())
	{
		nImgCount = 0;
		return 0;
	}

	Point center0(cvRound(circles[0][0]), cvRound(circles[0][1])); //center of the circle
	int radiusOutside = cvRound(circles[0][2]);					   //Ro
	int radiusInside = radiusOutside / 2;						   //Ri

	//draw circle to ui real video
	auto circleColor = Scalar(0, 255, 0); //green
	auto drawCircle = [&] {
		Size size; Point ofs;
		roiImage.locateROI(size, ofs);
		QMutex mutex;
		mutex.lock();
		circle(frameToShow, center0 + ofs, 3, circleColor, -1);
		circle(frameToShow, center0 + ofs, radiusOutside, circleColor, 1, LINE_AA);
		mutex.unlock();
	};

	//judge if the circle is out of ROI
	if ((center0.x + radiusOutside) > roiImage.cols || (center0.x - radiusOutside) < 0 ||
		(center0.y + radiusOutside) > roiImage.rows || (center0.y - radiusOutside) < 0)
	{
		circleColor = Scalar(0, 255, 255); //yellow
		drawCircle();
		nImgCount = 0;
		return 0;
	}
	drawCircle();
	nImgCount++;

	/************************************************************************/
	/********Ring mask*********/
	Rect ringRect((center0.x - radiusOutside), (center0.y - radiusOutside), 2 * radiusOutside, 2 * radiusOutside);
	Mat imageRing = roiImage(ringRect);
	Mat maskTmp = rMatcher.getMask(imageRing.size(), radiusOutside - 10, radiusInside + 15); //get a ring mask, use to match

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
		nImgCount = 1; //continuous sampling
	}
	/************************************************************************/

	//**************************orb matches**************************//

	Mat image_matches;
	double oneAngle;
	if (!rMatcher.match(dstImg[0], dstImg[1], mask[0], mask[1], image_matches, oneAngle)) //200ms
		return 2;

	//angle to linear velocity
	double rtSpeed = oneAngle * g_imgParam.angle2Speed;
	qDebug("imageprocess Speed: %.2lf; refSpeed: %.2lf", rtSpeed, refSpeed);
	rtSpeeds.push_back(rtSpeed);
	refSpeeds.push_back(refSpeed);

	/************************************************************************/
	/*if wheel stops, end and settle this wheel.*/
	/*to avoid the error rtSpeed, this judge should be refSpeed and rtSpeed*/
	/*this won't stop the video cap save, until save timeout(100s)*/
	/************************************************************************/

	if (refSpeed < 0.05 && rtSpeed < 0.05)
	{
		bWheelStopped = true;
	}

	//save matches
	//QString nowDate = QDate::currentDate().toString("yyyyMMdd");
	//QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
	//QString fullFilePath = QStringLiteral("%1/%2/%3.jpg").arg(matchDirPath).arg(nowDate).arg(nowTime);
	////getUniqueFile(fullFilePath);
	//imwrite(fullFilePath.toStdString(), image_matches);

	//show matches in UI
	mutex.lock();
	imageMatches = image_matches;
	mutex.unlock();
	emit showImageMatches();
	return 1;
}

void ImageProcess::foreverPreProcess()
{
	QMutex mutex;
	mutex.lock();
	srcImg = HikVideoCapture::pRawImage.clone();
	//real time velocity difference between imgprocess and PLC(speedAD)
	refSpeed = PLCSerial::speedAD*0.954;	//TODO:the rate should optimize by LS method (minimize the error)
	mutex.unlock();
	//test
	Mat resizedFrame;
	resize(srcImg, resizedFrame, Size(1280, 720), 0, 0, CV_INTER_LINEAR);
	calibratedFrame = cameraUndistort(resizedFrame); //calibrate the srcImg //cost 50ms
}

void ImageProcess::alarmThisWheel()
{
	WheelParam wheel;
	wheel.num = QString::fromStdString(ocr->get_final_result());
	wheel.totalmatch = rtSpeeds.size();
	double meanDiff = 0;
	double meanRef = 0;
	wheel.ocrsize = ocr->size();
	wheel.videopath = HikVideoCapture::videoRelativeFilePath;
	qDebug() << "Num: " << wheel.num;

	///no result
	if (wheel.totalmatch < 1)
	{
		wheel.alarmlevel = -1;
		qCritical() << "ImageProcess: MISS, no match result";
	}
	///has >1 result
	else
	{
		vector<double> rtSpeedDiffs(rtSpeeds.size());														   //a vector saves speed difference between imgprocess and PLC(speedAD)
		transform(rtSpeeds.begin(), rtSpeeds.end(), refSpeeds.begin(), rtSpeedDiffs.begin(), minus<double>()); //rt-ref=diff
		OutlierHelper outlier;
		outlier.reject(rtSpeedDiffs); //kick out the bad results by grubbs certain
		meanRef = outlier.mean(refSpeeds);
		meanDiff = outlier.mean(rtSpeedDiffs);
		wheel.validmatch = rtSpeedDiffs.size();

		///reliable results
		if (wheel.validmatch >= 10)
		{
			double meanRt = meanRef + meanDiff; //mean linear Velocity
			qDebug("Speed: %.2lfm/min", meanRt);
			///unacceptable error
			if (fabs(meanDiff) > meanRef * g_imgParam.warningRatio)
			{
				if (fabs(meanDiff) < meanRef * g_imgParam.alarmRatio) //if this wheel is too too slow
				{
					emit setAlarmLight(AlarmColorRed);
					qCritical() << "ImageProcess: WHEEL FATAL ERROR(single too slow)";
					wheel.alarmlevel = 2;
				}
				else if (previousAlarmLevel(wheel.num) > 0) //if the last wheel is alarmed
				{
					emit setAlarmLight(AlarmColorRed);
					qCritical() << "ImageProcess: WHEEL FATAL ERROR(double slow)";
					wheel.alarmlevel = 2;
				}
				else
				{
					emit setAlarmLight(AlarmColorYellow);
					qCritical() << "ImageProcess: WHEEL WARNING ERROR";
					wheel.alarmlevel = 1;
				}
				emit showAlarmNum(wheel.num);
			}
			///acceptable, good result
			else
			{
				wheel.alarmlevel = 0;
			}
		}
		///unreliable, invalid
		else
		{
			if (previousAlarmLevel(wheel.num) < 0)
			{
				emit setAlarmLight(AlarmColorYellow);
				qCritical() << "ImageProcess: WHEEL FATAL ERROR(double invalid)";
				wheel.alarmlevel = -2;
			}
			else
			{
				emit showWheelSpeed(MISS_TEST_SPEED);
				qCritical() << "ImageProcess: No enough valid results";
				wheel.alarmlevel = -1;
			}
		}
	}
	if (nFragments != 1)
	{
		qWarning() << "ImageProcess: detect count error: " << nFragments;
	}
	for (auto&& sp : rtSpeeds) {
		wheel.speeds += QString(" %1").arg(sp, 0, 'f', 2);
	}
	wheel.speeds += ";";
	for (auto&& sp : refSpeeds) {
		wheel.speeds += QString(" %1").arg(sp, 0, 'f', 2);
	}
	insertRecord(wheel.num, wheel.alarmlevel, meanDiff, meanRef, wheel.ocrsize, nFragments, wheel.totalmatch, wheel.validmatch, wheel.speeds, HikVideoCapture::videoRelativeFilePath);
	insertRecord(wheel);
	/************************************************************************/
	/* reset parameters of this wheel                                       */
	nFragments = 0;
	rtSpeeds.clear();
	refSpeeds.clear();
	nImgCount = 0; //no need	//once settle a wheel, force to zero
	/************************************************************************/
}
void ImageProcess::startImageProcess()
{
	mutex.lock();
	bStopProcess = false;
	mutex.unlock();
}
void ImageProcess::stopImageProcess()
{
	mutex.lock();
	bStopProcess = true;
	mutex.unlock();
}
void ImageProcess::sensorIN()
{
	mutex.lock();
	bWheelStopped = false;
	bIsInArea = true;
	mutex.unlock();
	//time.start();
}
void ImageProcess::sensorOUT()
{
	mutex.lock();
	bIsInArea = false;
	mutex.unlock();
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

cv::Mat ImageProcess::cameraUndistort(cv::Mat src)
{
	double ma[3][3] = {
		5.465101556178353e+02,
		0.650701463159305,
		6.491732555284723e+02,
		0,
		5.460864354608683e+02,
		3.503090793118121e+02,
		0,
		0,
		1 };

	Mat cameraMatrix = Mat(3, 3, CV_64F, ma);
	Mat distCoeffs = (Mat_<double>(5, 1) << -0.126396146351086,
		0.012067785981004,
		-6.004717303426694e-04,
		0.001281258711954,
		0);

	Mat view, rview, map1, map2;
	Size imageSize = src.size();

	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
		getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, nullptr),
		imageSize, CV_16SC2, map1, map2);

	Mat srcCalibration;
	remap(src, srcCalibration, map1, map2, INTER_LINEAR);
	return srcCalibration;
}

void ImageProcess::setupModel()
{
	initThreadDb();
}

bool ImageProcess::insertRecord(const QString &num, int alarmLevel, double absError, double refSpeed, int ocrsize, int fragment, int totalMatch, int validMatch, const QString &savedSpeeds, const QString &videoPath)
{
	QSqlTableModel *model = new QSqlTableModel(nullptr, QSqlDatabase::database(THEAD_CONNECTION_NAME));
	model->setTable("wheels");
	model->select();
	QSqlRecord record = model->record();
	QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	auto calSpeed_rough = QString::number(absError + refSpeed, 'f', 1).toDouble();
	auto refSpeed_rough = QString::number(refSpeed, 'f', 1).toDouble();
	auto relError = QString::number(absError / refSpeed * 100, 'f', 0).toDouble();
	record.setValue(Wheel_Num, QVariant(num));
	record.setValue(Wheel_Time, QVariant(time));
	record.setValue(Wheel_CalcSpeed, QVariant(calSpeed_rough));
	record.setValue(Wheel_RefSpeed, QVariant(refSpeed_rough));
	record.setValue(Wheel_Error, QVariant(relError));
	record.setValue(Wheel_AlarmLevel, QVariant(alarmLevel));
	record.setValue(Wheel_OcrSize, QVariant(ocrsize));
	int checkState;
	switch (alarmLevel)
	{
	case -2:
	case 1:
	case 2:
		checkState = NeedCheck;
		break;
	case -1:
	case 0:
	default:
		checkState = NoNeedCheck;
		break;
	}
	record.setValue(Wheel_CheckState, QVariant(checkState));
	record.setValue(Wheel_Speeds, QVariant(savedSpeeds));
	record.setValue(Wheel_Fragment, QVariant(fragment));
	record.setValue(Wheel_TotalMatch, QVariant(totalMatch));
	record.setValue(Wheel_ValidMatch, QVariant(validMatch));
	record.setValue(Wheel_VideoPath, QVariant(videoPath)); //relative path, e.g. 20170912/241343.mp4
	bool r = model->insertRecord(-1, record);
	r &= model->submitAll(); //on manualsubmit, use submitAll()
	return r;
}

bool ImageProcess::insertRecord(WheelParam wheel)
{
	QSqlTableModel *model = new QSqlTableModel(nullptr, QSqlDatabase::database(THEAD_CONNECTION_NAME));
	model->setTable("wheels");
	model->select();
	QSqlRecord record = model->record();
	//QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	//auto calSpeed_rough = QString::number(absError + refSpeed, 'f', 1).toDouble();
	//auto refSpeed_rough = QString::number(refSpeed, 'f', 1).toDouble();
	//auto relError = QString::number(absError / refSpeed * 100, 'f', 0).toDouble();
	//record.setValue(Wheel_Num, QVariant(num));
	//record.setValue(Wheel_Time, QVariant(time));
	//record.setValue(Wheel_CalcSpeed, QVariant(calSpeed_rough));
	//record.setValue(Wheel_RefSpeed, QVariant(refSpeed_rough));
	//record.setValue(Wheel_Error, QVariant(relError));
	//record.setValue(Wheel_AlarmLevel, QVariant(alarmLevel));
	//record.setValue(Wheel_OcrSize, QVariant(ocrsize));
	//int checkState;
	//switch (alarmLevel)
	//{
	//case -2:
	//case 1:
	//case 2:
	//	checkState = NeedCheck;
	//	break;
	//case -1:
	//case 0:
	//default:
	//	checkState = NoNeedCheck;
	//	break;
	//}
	//record.setValue(Wheel_CheckState, QVariant(checkState));
	//record.setValue(Wheel_Speeds, QVariant(savedSpeeds));
	//record.setValue(Wheel_Fragment, QVariant(fragment));
	//record.setValue(Wheel_TotalMatch, QVariant(totalMatch));
	//record.setValue(Wheel_ValidMatch, QVariant(validMatch));
	//record.setValue(Wheel_VideoPath, QVariant(videoPath)); //relative path, e.g. 20170912/241343.mp4

	record.setValue(Wheel_Num, QVariant(wheel.num));
	record.setValue(Wheel_CalcSpeed, QVariant(wheel.calcspeed));
	record.setValue(Wheel_RefSpeed, QVariant(wheel.refspeed));
	record.setValue(Wheel_Error, QVariant(wheel.error));
	record.setValue(Wheel_Time, QVariant(wheel.time));
	record.setValue(Wheel_AlarmLevel, QVariant(wheel.alarmlevel));
	record.setValue(Wheel_CheckState, QVariant(wheel.checkstate));
	record.setValue(Wheel_OcrSize, QVariant(wheel.ocrsize));
	record.setValue(Wheel_Fragment, QVariant(wheel.fragment));
	record.setValue(Wheel_TotalMatch, QVariant(wheel.totalmatch));
	record.setValue(Wheel_ValidMatch, QVariant(wheel.validmatch));
	record.setValue(Wheel_Speeds, QVariant(wheel.speeds));
	record.setValue(Wheel_VideoPath, QVariant(wheel.videopath));
	bool r = model->insertRecord(-1, record);
	r &= model->submitAll(); //on manualsubmit, use submitAll()
	return r;
}

int ImageProcess::previousAlarmLevel(const QString & num) const
{
	//every query is up to data using select();
	QSqlTableModel *model = new QSqlTableModel(nullptr, QSqlDatabase::database(THEAD_CONNECTION_NAME));
	model->setTable("wheels");
	model->setFilter(QString("num='%1'").arg(num));
	model->select();
	int row = model->rowCount();
	if (row > 0)
	{
		return model->data(model->index(row - 1, Wheel_AlarmLevel)).toInt();
	}
	return 0;  //if no previous result, regard it as a good wheel
}