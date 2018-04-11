#include "stdafx.h"
#include "imageprocess.h"
#include "common.h"
#include "database.h"
#include "hikvideocapture.h"
#include "outlierhelper.h" //used for calc mean, replace opencv lib
#include "plcserial.h"
#include "ocr.h"
#include "confighelper.h"
#include "robustmatcher.h"

using namespace std;
using namespace cv;

ImageProcess::ImageProcess(const ConfigHelper *_configHelper, HikVideoCapture *_capture, PLCSerial *_plcSerial, QObject *parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, configHelper(_configHelper)
	, videoCapture(_capture)
	, plcSerial(_plcSerial)
	, deviceIndex(videoCapture->getDeviceIndex())
	, imProfile(&(configHelper->device[deviceIndex].imProfile))
	, ocr(new OCR(configHelper, deviceIndex, this))
	, rMatcher(new RobustMatcher)
{
	connect(this, &ImageProcess::initModel, this, &ImageProcess::setupModel);

	connect(videoCapture, &HikVideoCapture::captureOneImage, this, &ImageProcess::doImageProcess);
	connect(this, &ImageProcess::imageProcessReady, videoCapture, &HikVideoCapture::currentImageProcessReady);

	connect(videoCapture, &HikVideoCapture::recordTimeout, this, &ImageProcess::onWheelTimeout);
	connect(plcSerial, &PLCSerial::trolleySpeedReady, this, [&]() {rtRefSpeed = plcSerial->getTrolleySpeed(); });
	connect(plcSerial, &PLCSerial::sensorIN, this, &ImageProcess::onSensorIN);
	connect(plcSerial, &PLCSerial::sensorOUT, this, &ImageProcess::onSensorOUT);

	connect(this, &ImageProcess::setAlarmLight, plcSerial, &PLCSerial::Alarm);
}

ImageProcess::~ImageProcess()
= default;

void ImageProcess::doImageProcess()
{
	foreverPreProcess();
	mutex.lock(); //prevent reading the frameToShow simultaneously
	calibratedFrame.copyTo(frameToShow); //here we use deep copy, otherwise we will change calibratedFrame when change frameToshow
	cvtColor(frameToShow, frameToShow, CV_GRAY2BGR);
	rectangle(frameToShow, imProfile->roiRect, Scalar(0, 0, 255), 2);
	mutex.unlock();
	if (bIsProcessing)
	{
		//when wheel is stop,
		//1. stop the process to avoid meaningless count
		//2. drop the calc results and wait for a come-in signal.
		//this usually happens when wheel timeout, or the speed is detected as zero
		if (bIsTrolleyStopped)
		{
			alarmThisWheel();
		}

		else
		{
			/*sensor triggered*/
			//decide if the wheel comes in/out by SENSOR IN/OUT
			if (imProfile->sensorTriggered)
			{
				static bool lastIsIn = false;	//the wheel is in(1) or out(0) of the detect area. when 1->0(falling edge), which means the wheel is scrolling out, alarm this wheel.
												//assume wheel is out at initial, won't be set to true until a new wheel comes in.
				if (bIsTrolleyInSensors)						   //in detect area
				{
					static bool lastCore = false;	  //record the last return of core process
					bool nowCore = coreImageProcess(); //0->1, Fragment++ at rise edge. NOTE: it can be 0, which means no circle detected
					if (nowCore&(!lastCore)) //nowCore == true && lastCore == false
					{
						wheelDbInfo.fragment++;
					}
					lastCore = nowCore;
				}
				else if (lastIsIn) //lastIsIn==true && bIsInArea==false,which means wheel is out of detect area
				{
					//Once a wheel scroll out the detect area, settle it.
					alarmThisWheel();
				}
				lastIsIn = bIsTrolleyInSensors;
			}
			/*image triggered*/
			//for test manually, independent on cameracapsave
			else
			{
				static bool lastCore = false;
				bool nowCore = coreImageProcess();
				if (nowCore && !lastCore) //0->1
				{
					wheelDbInfo.fragment++;
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
	Mat roiImage = calibratedFrame(imProfile->roiRect); //srcImg(rect).copyTo(roiImage);
	equalizeHist(roiImage, roiImage); //equalize the image, this will change calibratedFrame//2017/12/21
	Mat blurImage;							   //use blur image for hough circles, use source image for matches

	/************************************************************************/
	/* hough circles                                                                     */
	/************************************************************************/
	// Reduce the noise so we avoid false circle detection
	GaussianBlur(roiImage, blurImage, Size(imProfile->gs1, imProfile->gs1), 2, 2); //the greater the kernel size is, the less time the hough cost
	// Canny show
	//Mat detectEdges;
	//Canny(blurImage, detectEdges,imProfile->param1 / 2,imProfile->param1, 3);
	//imshow("edges", detectEdges);
	// Apply the Hough Transform to find the circles. //cost 40ms
	vector<Vec3f> circles;
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, imProfile->dp, imProfile->minDist, imProfile->param1, imProfile->param2, imProfile->radius_min, imProfile->radius_max);
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
	Mat maskTmp = rMatcher->getMask(imageRing.size(), radiusOutside - 10, radiusInside + 15); //get a ring mask, use to match

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
	if (!rMatcher->match(dstImg[0], dstImg[1], mask[0], mask[1], image_matches, oneAngle)) //200ms
		return 2;

	//angle to linear velocity
	double rtSpeed = oneAngle * imProfile->angle2Speed();
	qDebug("imageprocess Speed: %.2lf; refSpeed: %.2lf", rtSpeed, rtRefSpeed);
	rtSpeeds.push_back(rtSpeed);
	refSpeeds.push_back(rtRefSpeed);

	/************************************************************************/
	/*if wheel stops, end and settle this wheel.*/
	/*to avoid the error rtSpeed, this judge should be refSpeed and rtSpeed*/
	/*this won't stop the video cap save, until save timeout(100s)*/
	/************************************************************************/

	if (rtRefSpeed < 0.05 && rtSpeed < 0.05)
	{
		bIsTrolleyStopped = true;
	}
	return 1;
}

void ImageProcess::foreverPreProcess()
{
	srcImg = videoCapture->getRawImage();
	//real time velocity difference between imgprocess and PLC(speedAD)
	//test
	Mat resizedFrame;
	resize(srcImg, resizedFrame, Size(1280, 720), 0, 0, CV_INTER_LINEAR);
	calibratedFrame = cameraUndistort(resizedFrame); //calibrate the srcImg //cost 50ms
}

void ImageProcess::alarmThisWheel()
{
	/* 未出现在此处的wheelDbInfo, 只在特定时候（valid）时候才有效，refspeed为coreProcess时取得的 */
	if (0 == deviceIndex) { wheelDbInfo.i_o = QStringLiteral("外"); }
	else if (1 == deviceIndex) { wheelDbInfo.i_o = QStringLiteral("内"); }
	wheelDbInfo.num = QString::fromStdString(ocr->get_final_result());
	qDebug() << "ImageProcess: One wheel ready.";
	qDebug() << "Wheel Info:" << wheelDbInfo.i_o << wheelDbInfo.num;
	wheelDbInfo.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	wheelDbInfo.ocrsize = ocr->size();
	wheelDbInfo.validmatch = wheelDbInfo.totalmatch = rtSpeeds.size(); // match size
	wheelDbInfo.videopath = videoCapture->getVideoRelativeFilePath();
	wheelDbInfo.refspeed = rtRefSpeed; // if no match result, it will show the current trolley speed

	if (wheelDbInfo.totalmatch < 1)
	{//no result
		wheelDbInfo.alarmlevel = -1;
		qCritical() << "ImageProcess: MISS, no match result";
	}

	else
	{//has match results
		vector<double> absErrors(rtSpeeds.size());														   //a vector saves speed difference between imgprocess and PLC(speedAD)
		transform(rtSpeeds.begin(), rtSpeeds.end(), refSpeeds.begin(), absErrors.begin(), minus<double>()); //rt-ref=diff
		OutlierHelper outlier;
		outlier.removeOutliers(absErrors); //kick out the bad results by grubbs certain
		double refspeed = outlier.mean(refSpeeds);
		double absError = outlier.mean(absErrors);
		wheelDbInfo.validmatch = absErrors.size();

		if (wheelDbInfo.validmatch >= IM_PROC_VALID_MIN_COUNT)
		{//reliable results
		 /* approximate result to write into database */
			wheelDbInfo.calcspeed = QString::number(absError + refspeed, 'f', 1).toDouble();
			wheelDbInfo.refspeed = QString::number(refspeed, 'f', 1).toDouble();
			wheelDbInfo.error = QString::number(absError / refspeed * 100, 'f', 0).toDouble();

			if (fabs(absError) <= refspeed * imProfile->warningRatio)
			{//acceptable, good result
				wheelDbInfo.alarmlevel = 0;
			}
			else
			{//unacceptable error
				if (fabs(absError) > refspeed *imProfile->alarmRatio) //if this wheel is too too slow
				{
					qCritical() << "ImageProcess: Wheel FATAL error(error>alarm ratio )";
					wheelDbInfo.alarmlevel = 2;
				}
				else if (previousAlarmLevel(wheelDbInfo.num) > 0) //if the last wheel is alarmed
				{
					qCritical() << "ImageProcess: Wheel FATAL error(continuous warning))";
					wheelDbInfo.alarmlevel = 2;
				}
				else
				{
					qCritical() << "ImageProcess: Wheel WARNING error(error>warning ratio)";
					wheelDbInfo.alarmlevel = 1;
				}
			}
		}
		///unreliable, invalid
		else { wheelDbInfo.alarmlevel = -1; }
	}

	/* wheelspeeds */
	for (auto&& sp : rtSpeeds) {
		wheelDbInfo.speeds += QString(" %1").arg(sp, 0, 'f', 2);
	}
	wheelDbInfo.speeds += ";";
	for (auto&& sp : refSpeeds) {
		wheelDbInfo.speeds += QString(" %1").arg(sp, 0, 'f', 2);
	}

	handleAlarmLevel(wheelDbInfo.alarmlevel);
	insertRecord(wheelDbInfo);
	/************************************************************************/
	/* reset parameters of this wheel                                       */
	rtSpeeds.clear();
	refSpeeds.clear();
	wheelDbInfo = { 0 };
	nImgCount = 0; //no need	//once settle a wheel, force to zero
	/************************************************************************/
}

void ImageProcess::onSensorIN()
{
	mutex.lock();
	bIsTrolleyStopped = false;
	bIsTrolleyInSensors = true;
	mutex.unlock();
	//time.start();
}
void ImageProcess::onSensorOUT()
{
	mutex.lock();
	bIsTrolleyInSensors = false;
	mutex.unlock();
}

void ImageProcess::onWheelTimeout()
{
	QMutexLocker locker(&mutex);
	if (!bIsTrolleyStopped)
	{
		qWarning("Cart is stopped");
		bIsTrolleyStopped = true;
	}
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
	initThreadDb(deviceIndex);
}

bool ImageProcess::insertRecord(const WheelDbInfo &info)
{
	QSqlTableModel *model = new QSqlTableModel(nullptr, QSqlDatabase::database(deviceIndex ? THEAD1_CONNECTION_NAME : THEAD0_CONNECTION_NAME));
	model->setTable("wheels");
	model->select();
	QSqlRecord record = model->record();

	record.setValue(Wheel_I_O, QVariant(info.i_o));
	record.setValue(Wheel_Num, QVariant(info.num));
	record.setValue(Wheel_CalcSpeed, QVariant(info.calcspeed));
	record.setValue(Wheel_RefSpeed, QVariant(info.refspeed));
	record.setValue(Wheel_Error, QVariant(info.error));
	record.setValue(Wheel_Time, QVariant(info.time));
	record.setValue(Wheel_AlarmLevel, QVariant(info.alarmlevel));
	record.setValue(Wheel_CheckState, QVariant(info.checkstate));
	record.setValue(Wheel_OcrSize, QVariant(info.ocrsize));
	record.setValue(Wheel_Fragment, QVariant(info.fragment));
	record.setValue(Wheel_TotalMatch, QVariant(info.totalmatch));
	record.setValue(Wheel_ValidMatch, QVariant(info.validmatch));
	record.setValue(Wheel_Speeds, QVariant(info.speeds));
	record.setValue(Wheel_VideoPath, QVariant(info.videopath));
	bool r = model->insertRecord(-1, record);
	r &= model->submitAll(); //on manualsubmit, use submitAll()
	return r;
}

int ImageProcess::previousAlarmLevel(const QString & num) const
{
	if (num == OCR_MISS)
	{//号码牌输出MISS, 直接返回正常
		return 0;
	}
	//every query is up to data using select();
	QSqlTableModel *model = new QSqlTableModel(nullptr, QSqlDatabase::database(deviceIndex ? THEAD1_CONNECTION_NAME : THEAD0_CONNECTION_NAME));
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

void ImageProcess::handleAlarmLevel(int lv)
{
	switch (lv)
	{
	case -2:
		emit setAlarmLight(AlarmColorYellow);
		qCritical() << "ImageProcess: Wheel FATAL error(continuous invalid)";
		wheelDbInfo.calcspeed = IM_PROC_INVALID_SPEED;
		break;
	case -1:
		if (previousAlarmLevel(wheelDbInfo.num) < 0)
		{
			wheelDbInfo.alarmlevel = -2;
			handleAlarmLevel(-2);
			return;
		}
		else
		{
			wheelDbInfo.calcspeed = IM_PROC_INVALID_SPEED;
			qCritical() << "ImageProcess: Miss, No enough valid results";
		}
		break;
	case 0:
		break;
	case 1:
		emit setAlarmLight(AlarmColorYellow);
		break;
	case 2:
		emit setAlarmLight(AlarmColorRed);
		break;
	default:
		break;
	}

	/* checkstate */
	switch (lv)
	{
	case -2:
	case 1:
	case 2:
		wheelDbInfo.checkstate = NeedCheck;
		break;
	case -1:
	case 0:
	default:
		wheelDbInfo.checkstate = NoNeedCheck;
		break;
	}
}