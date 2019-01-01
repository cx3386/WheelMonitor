#include "stdafx.h"

#include "AlarmEvent.h"
#include "Plc.h"
#include "common.h"
#include "confighelper.h"
#include "database.h"
#include "hikvideocapture.h"
#include "imageprocess.h"
#include "ocr.h"
#include "outlierhelper.h" //used for calc mean, replace opencv lib
#include "robustmatcher.h"

using namespace std;
using namespace cv;

ImageProcess::ImageProcess(const ConfigHelper* _configHelper, HikVideoCapture* _capture, Plc* _plcSerial, QObject* parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, configHelper(_configHelper)
	, videoCapture(_capture)
	, plcSerial(_plcSerial)
	, deviceIndex(videoCapture->getDeviceIndex())
	, imProfile(&(configHelper->device[deviceIndex].imProfile))
	, ocr(new OCR(configHelper, deviceIndex, this))
	, rMatcher(new RobustMatcher)
{
	connect(videoCapture, &HikVideoCapture::recordTimeout, this, &ImageProcess::onWheelTimeout);
	connect(plcSerial, &Plc::truckSpeedReady, this, [=]() { rtRefSpeed = plcSerial->getTruckSpeed(deviceIndex); });
	// plc -> imageprocess
	connect(plcSerial->handleSensorDevice[deviceIndex], &HandleSensorDevice::dzIn, this, &ImageProcess::onSensorIN);
	connect(plcSerial->handleSensorDevice[deviceIndex], &HandleSensorDevice::dzOut, this, &ImageProcess::onSensorOUT);

	//videocapture -> imageprocess
	// 图像驱动法(TODO)
	//connect(this, &ImageProcess::_MAIn, videoCapture, &HikVideoCapture::startRecord);
	//connect(this, &ImageProcess::_MAOut, videoCapture, &HikVideoCapture::stopRecord);
	// //双向通知：cap->handle->cap->handle...是一个库存为1的生产者消费者模型，
	connect(this, &ImageProcess::frameHandled, videoCapture, &HikVideoCapture::frameProcessed);
	// videocapute -> imageproecss
	connect(videoCapture, &HikVideoCapture::frameCaptured, this, &ImageProcess::handleFrame);

	// plc -> video capture
	// 传感器驱动法
	connect(plcSerial->handleSensorDevice[deviceIndex], &HandleSensorDevice::dzIn, videoCapture, &HikVideoCapture::startRecord);
	connect(plcSerial->handleSensorDevice[deviceIndex], &HandleSensorDevice::dzOut, videoCapture, &HikVideoCapture::stopRecord);
	// videocapture -> plc // 放弃，用一个专门的报警类来控制报警 [9/20/2018 cx3386]
	//connect(this, &ImageProcess::setAlarmLight, plcSerial, &Plc::Alarm);
}

ImageProcess::~ImageProcess() {
	onStop();
	delete rMatcher;
};

void ImageProcess::handleFrame()
{
	makeFrame4Show(); //只要cap发送信号，则显示到界面上
	if (bUsrCtrl) {
		// 台车停止，丢弃帧，丢弃该车轮，停止处理。
		// 发生在光电开关未在规定时间内检测到离开信号，或者测到台车速度为0
		if (bIsTruckStopped)
			checkoutWheel();
		else {
			//sensor triggered. 通过SENSOR IN/OUT信号开始，注意，1个周期只收到1次in，1次out。/结束本车轮的检测，而无论core的返回值
			if (imProfile->sensorTriggered) {
				//int state = _DZRecorder.state(0);
				// 车轮进入、正在DZ
				//if (state == LevelRecorder::HighLevel || state == LevelRecorder::PositiveEdge) //in detect area
				if (bInDz)
				{
					checkoutAlready = false;
					int nCore = coreImageProcess(); //0->1, Fragment++ at rise edge. NOTE: it can be 0, which means no circle detected
					if (nCore == LocateFail && nCore_pre > RMA) //nowCore == true && lastCore == false
						interrupts++;
					nCore_pre = nCore;
				}
				// 车轮离开DZ，结算
				else {
					if (!checkoutAlready)
					{
						checkoutWheel();
						checkoutAlready = true;
					}
				}
			}
			//image triggered
			//通过LMA变InMA的上升沿开始，InMA变RMA的下降沿结算车轮；
			// 边界反复跳动：开始1(跳过)-->开始2,3..（清零）-->中断-->结束1（结算）-->结束2,3..（跳过）
			else {
				int nCore = coreImageProcess();
				// LMA->InMA上升沿 开始
				if (nCore > RMA && nCore_pre == LMA) {
					// 开始1
					if (_MAState != 0) {
						_MAState = 0;
					}
					// 开始2,3...
					else {
						rtSpeeds.clear();
						refSpeeds.clear();
						interrupts = 0;
						//wheelFrame_pre.release(); // 因为pre是LMA,所以frame_pre必然是空的
					}
				}
				// InMA->RMA下降沿 结算
				else if (nCore == RMA && nCore_pre > RMA) {
					// 结束1
					if (_MAState != 1) {
						_MAState = 1;
						// ncore重置为LocateFail，实际应该为RMA, 但并不影响判断（无pre==RMA或FindFail的判断条件）
						// 此时wheelFrame_pre已经自动release了
						checkoutWheel();
					}
				}
				// InMA->MissWheel 中断
				else if (nCore == LocateFail && nCore_pre > RMA) {
					_MAState = 2;
					interrupts++;
				}
				nCore_pre = nCore;
			}
		}
	}
	emit frameHandled();
	emit showFrame();
}

void ImageProcess::makeFrame4Show()
{
	preprocess();
	mutex.lock(); //prevent reading the frameToShow simultaneously
	undistortedFrame.copyTo(frameToShow); // 深拷贝，因为undistortedFrame将与framToShow的内容不同
	cvtColor(frameToShow, frameToShow, CV_GRAY2BGR); // 转换成彩图
	rectangle(frameToShow, imProfile->roiRect, Scalar(0, 0, 255), 2); // 在图中画彩色线框
	mutex.unlock();
}

int ImageProcess::coreImageProcess()
{
	ocr->core_ocr(srcImg); //card num detect gzy 2017/11/9
	Mat monitorArea = undistortedFrame(imProfile->roiRect); //截取监测区域
	equalizeHist(monitorArea, monitorArea); //equalize the image, this will change calibratedFrame//2017/12/21
	Mat blurImage; //use blur image for hough circles, use source image for matches

	// Reduce the noise to avoid false circle detection
	GaussianBlur(monitorArea, blurImage, Size(imProfile->gs1, imProfile->gs1), 2, 2); //the greater the kernel size is, the less time the hough cost
	// 霍夫匹配，寻找车轮的外圆。注意：该方法获取的圆可能部分在图像外面
	vector<Vec3f> circles;
	int max_r = std::min(monitorArea.rows / 2, imProfile->radius_max);// 防止纵向超出：通常高度比较小
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, imProfile->dp, imProfile->minDist, imProfile->param1, imProfile->param2, imProfile->radius_min, max_r);
	//无霍夫圆
	if (circles.empty()) {
		wheelFrame_pre.release(); // 有效帧将不连续，置空前一帧
		return LocateFail; // 车轮定位丢失
	}

	//取霍夫圆的最大一个，当作车轮外径。取外切矩形用于匹配
	//patch：唯一一个霍夫圆
	Point center0(cvRound(circles[0][0]), cvRound(circles[0][1])); // 圆心
	int radius = cvRound(circles[0][2]); // 半径
	Rect wheelRect((center0.x - radius), (center0.y - radius), 2 * radius, 2 * radius); // 外切矩形的位置

	//判断车轮是否在监测区域内
	Rect monitorRect(0, 0, monitorArea.cols, monitorArea.rows);
	bool bInside = isInside(wheelRect, monitorRect);

	//draw circle to UI
	auto circleColor = bInside ? Scalar(0, 255, 0) : Scalar(0, 255, 255); // 如果在里面则画绿，否则画黄
	Size size;
	Point ofs;
	monitorArea.locateROI(size, ofs); // get the offset of monitorArea from the srcImg
	mutex.lock();
	circle(frameToShow, center0 + ofs, 3, circleColor, -1); // center
	circle(frameToShow, center0 + ofs, radius, circleColor, 1, LINE_AA); // round
	mutex.unlock();

	// 未完全在监测区域内，调用时应注意颤抖引起的在边界的反复横跳
	if (!bInside) {
		//车轮在MA的左边，即未进入
		if (wheelRect.tl().x < monitorRect.tl().x) {
			wheelFrame_pre.release(); // 置空前一帧
			return LMA;
		}
		//车轮在MA的右侧，即已退出
		if (wheelRect.br().x > monitorRect.br().x) {
			wheelFrame_pre.release(); // 置空前一帧
			return RMA; //软件判别法应该结算该车轮
		}
		//车轮超出MA的上下界
		if (wheelRect.tl().y < monitorRect.tl().y || wheelRect.br().y > monitorRect.br().y) {
			wheelRect = wheelRect & monitorRect;//取交集
		}
	}

	// 至此成功锁定（区域内的）车轮，保存
	Mat matchSrc1 = wheelFrame_pre;
	Mat matchSrc2 = monitorArea(wheelRect); //外切矩形图像,注意：wheelrect必须在monitor内
	wheelFrame_pre = matchSrc2;

	// 前一帧为空
	if (matchSrc1.empty()) {
		return NoPre; // 等待匹配帧
	}

	// 开始匹配 //orb matches
	Mat image_matches; // no longer show in UI
	double oneAngle;
	if (!rMatcher->match(matchSrc1, matchSrc2, image_matches, oneAngle)) //200ms
	{
		//qDebug() << "match fail";
		return MatchFail;
	}
	mutex.lock();
	image_matches.copyTo(matchResult);
	mutex.unlock();
	emit showMatch();
	//angle to linear velocity
	double rtSpeed = oneAngle * imProfile->angle2Speed();
	qDebug("dev[%d] Speed: %.2lf; ref: %.2lf", deviceIndex, rtSpeed, rtRefSpeed);
	rtSpeeds.push_back(rtSpeed);
	refSpeeds.push_back(rtRefSpeed);

	// if truck stops, give up this wheel.
	// to avoid the error rtSpeed, this judge should be refSpeed and rtSpeed
	// this won't stop the video cap save, until save timeout(100s)
	if (rtRefSpeed < 0.05 && rtSpeed < 0.05) {
		bIsTruckStopped = true;
		return TruckSpZero;
	}
	return Success;
}

void ImageProcess::preprocess()
{
	srcImg = videoCapture->getRawImage();
	if (deviceIndex == 1) flip(srcImg, srcImg, 1);
	//real time velocity difference between imgprocess and PLC(speedAD)
	//test
	Mat resizedFrame;
	resize(srcImg, resizedFrame, Size(1280, 720), 0, 0, CV_INTER_LINEAR);
	undistortedFrame = cameraUndistort(resizedFrame); //calibrate the srcImg //cost 50ms
}

void ImageProcess::checkoutWheel()
{
	/* 未在此处初始化的wheelDbInfo, 只在特定时候（valid）时候才有效，refspeed为coreProcess时取得的 */
	WheelDbInfo wheelDbInfo;
	wheelDbInfo.i_o = deviceIndex;
	wheelDbInfo.num = QString::fromStdString(ocr->get_final_result());
	//qDebug() << "checkoutWheel. dev" << deviceIndex << "num" << wheelDbInfo.num;
	wheelDbInfo.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	wheelDbInfo.ocrsize = ocr->size();
	wheelDbInfo.fragment = interrupts;
	wheelDbInfo.validmatch = wheelDbInfo.totalmatch = rtSpeeds.size(); // match size
	wheelDbInfo.videopath = videoCapture->getVideoRelativeFilePath();
	wheelDbInfo.refspeed = rtRefSpeed; // if no match result, it will show the current truck speed

	if (wheelDbInfo.totalmatch < 1) { //no result
		wheelDbInfo.alarmlevel = -1;
		qCritical() << "ImageProcess: MISS, no match result";
	}

	else { //has match results
		vector<double> absErrors(rtSpeeds.size()); //a vector saves speed difference between imgprocess and PLC(speedAD)
		transform(rtSpeeds.begin(), rtSpeeds.end(), refSpeeds.begin(), absErrors.begin(), minus<double>()); //rt-ref=diff
		OutlierHelper outlier;
		outlier.removeOutliers(absErrors); //kick out the bad results by grubbs certain
		double refspeed = outlier.mean(refSpeeds);
		double absError = outlier.mean(absErrors);
		wheelDbInfo.validmatch = absErrors.size();

		if (wheelDbInfo.validmatch >= IM_PROC_VALID_MIN_COUNT) { //reliable results
			/* approximate result to write into database */
			wheelDbInfo.calcspeed = QString::number(absError + refspeed, 'f', 2).toDouble();
			wheelDbInfo.refspeed = QString::number(refspeed, 'f', 2).toDouble();
			wheelDbInfo.error = QString::number(absError / refspeed * 100, 'f', 1).toDouble();
			qDebug() << "Speed: " << wheelDbInfo.calcspeed << ";Ref: " << wheelDbInfo.refspeed;
			if (fabs(absError) <= refspeed * imProfile->warningRatio) { //acceptable, good result
				wheelDbInfo.alarmlevel = 0;
			}
			else { //unacceptable error
				if (fabs(absError) > refspeed * imProfile->alarmRatio) //if this wheel is too too slow
				{
					qCritical() << "ImageProcess: Wheel FATAL error(error>alarm ratio )";
					wheelDbInfo.alarmlevel = 2;
				}
				else if (previousAlarmLevel(wheelDbInfo.num) > 0) //if the last wheel is alarmed
				{
					qCritical() << "ImageProcess: Wheel FATAL error(continuous warning))";
					wheelDbInfo.alarmlevel = 2;
				}
				else {
					qCritical() << "ImageProcess: Wheel WARNING error(error>warning ratio)";
					wheelDbInfo.alarmlevel = 1;
				}
			}
		}
		//unreliable, invalid
		else {
			wheelDbInfo.alarmlevel = -1;
		}
	}

	/* wheelspeeds */
	for (auto&& sp : rtSpeeds) {
		wheelDbInfo.speeds += QString(" %1").arg(sp, 0, 'f', 2);
	}
	wheelDbInfo.speeds += ";";
	for (auto&& sp : refSpeeds) {
		wheelDbInfo.speeds += QString(" %1").arg(sp, 0, 'f', 2);
	}

	handleAlarmLevel(wheelDbInfo); //引用
	if (!insertRecord(wheelDbInfo))
		qDebug() << "insert record failed";
	/* reset parameters of this wheel */
	clearWheel();
}

void ImageProcess::clearWheel()
{
	rtSpeeds.clear();
	refSpeeds.clear();
	interrupts = 0;
	//防止对下一个车轮干扰，只对sensor triggered有效,image tri会（在目前限制的条件下）自动清除以下两项
	nCore_pre = LocateFail;
	wheelFrame_pre.release();
}

void ImageProcess::onSensorIN()
{
	//QMutexLocker locker(&mutex);// 以下变量全部只有本线程操作 [10/12/2018 cx3386]
	bIsTruckStopped = false;
	//_DZRecorder.push(true);
	bInDz = true;
	//qDebug() << "dev[" << deviceIndex << "] DZ In";
}
void ImageProcess::onSensorOUT()
{
	//QMutexLocker locker(&mutex);
	//_DZRecorder.push(false);
	bInDz = false;//下次handleframe时不再触发coreProc
	//qDebug() << "dev[" << deviceIndex << "] DZ Out";
	//checkoutWheel();
}

void ImageProcess::onWheelTimeout()
{
	//QMutexLocker locker(&mutex);
	if (!bIsTruckStopped) {
		qWarning("Cart is stopped");
		bIsTruckStopped = true;
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
		1
	}; ///< for 1280*720

	Mat cameraMatrix = Mat(3, 3, CV_64F, ma);
	Mat distCoeffs = (Mat_<double>(5, 1) << -0.126396146351086,
		0.012067785981004,
		-6.004717303426694e-04,
		0.001281258711954,
		0);

	Mat view, rview, map1, map2;
	Size imageSize = src.size();

	//undistort combine initUndistortRectifyMap and remap
	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
		getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, nullptr),
		imageSize, CV_16SC2, map1, map2);

	Mat srcCalibration;
	remap(src, srcCalibration, map1, map2, INTER_LINEAR);
	return srcCalibration;
}

void ImageProcess::onStart()
{
	// 总是子线程操作，不需要锁
	if (bUsrCtrl) {
		qWarning() << "ImageProcess: repeated start";
		return;
	}
	bInDz = false;
	_MAState = 1; //因此不会触发结束1
	//nCore_pre = FindFail;
	bUsrCtrl = true;
}

void ImageProcess::onStop()
{
	if (!bUsrCtrl) {
		qWarning() << "ImageProcess: repeated stop";
		return;
	}
	// 连续性中断，重置相关量为原始状态
	ocr->resetOcr(); // ocr重置
	checkoutAlready = true;
	clearWheel();
	bUsrCtrl = false;
}

void ImageProcess::setupModel()
{
	initThreadDb(deviceIndex);
}

bool ImageProcess::insertRecord(const WheelDbInfo& info)
{
	QSqlTableModel model{ nullptr, QSqlDatabase::database(deviceIndex ? THREAD1_CONNECTION_NAME : THREAD0_CONNECTION_NAME) };
	model.setTable("wheels");
	model.select();
	QSqlRecord record = model.record();

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
	bool r = model.insertRecord(-1, record);
	r &= model.submitAll(); //on manualsubmit, use submitAll()
	return r;
}

int ImageProcess::previousAlarmLevel(const QString& num) const
{
	if (num == OCR_MISS) { //号码牌输出MISS, 直接返回正常
		return 0;
	}
	//every query is up to data using select();
	QSqlTableModel* model = new QSqlTableModel(nullptr, QSqlDatabase::database(deviceIndex ? THREAD1_CONNECTION_NAME : THREAD0_CONNECTION_NAME));
	model->setTable("wheels");
	model->setFilter(QString("num='%1'").arg(num));
	model->select();
	int row = model->rowCount();
	if (row > 0) {
		return model->data(model->index(row - 1, Wheel_AlarmLevel)).toInt();
	}
	return 0; //if no previous result, regard it as a good wheel
}

//! 根据车轮的警报等级向报警处理类发出信号
void ImageProcess::handleAlarmLevel(WheelDbInfo& wheelDbInfo)
{
	int lv = wheelDbInfo.alarmlevel;
	switch (lv) {
	case -2:
		emit setAlarmLight((int)AlarmColor::Yellow);
		qCritical() << "handleAlarmLevel: Wheel FATAL error(continuous invalid)";
		wheelDbInfo.calcspeed = IM_PROC_INVALID_SPEED;
		break;
	case -1:
		if (previousAlarmLevel(wheelDbInfo.num) < 0) {
			wheelDbInfo.alarmlevel = -2;
			handleAlarmLevel(wheelDbInfo);
			return;
		}
		else {
			wheelDbInfo.calcspeed = IM_PROC_INVALID_SPEED;
			qCritical() << "ImageProcess: Miss, No enough valid results";
		}
		break;
	case 0:
		break;
	case 1:
		emit setAlarmLight((int)AlarmColor::Yellow);
		break;
	case 2:
		emit setAlarmLight((int)AlarmColor::Red);
		break;
	default:
		break;
	}

	/* checkstate */
	switch (lv) {
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