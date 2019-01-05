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
	// ͼ��������(TODO)
	//connect(this, &ImageProcess::_MAIn, videoCapture, &HikVideoCapture::startRecord);
	//connect(this, &ImageProcess::_MAOut, videoCapture, &HikVideoCapture::stopRecord);
	// //˫��֪ͨ��cap->handle->cap->handle...��һ�����Ϊ1��������������ģ�ͣ�
	connect(this, &ImageProcess::frameHandled, videoCapture, &HikVideoCapture::frameProcessed);
	// videocapute -> imageproecss
	connect(videoCapture, &HikVideoCapture::frameCaptured, this, &ImageProcess::handleFrame);

	// plc -> video capture
	// ������������
	connect(plcSerial->handleSensorDevice[deviceIndex], &HandleSensorDevice::dzIn, videoCapture, &HikVideoCapture::startRecord);
	connect(plcSerial->handleSensorDevice[deviceIndex], &HandleSensorDevice::dzOut, videoCapture, &HikVideoCapture::stopRecord);
	// videocapture -> plc // ��������һ��ר�ŵı����������Ʊ��� [9/20/2018 cx3386]
	//connect(this, &ImageProcess::setAlarmLight, plcSerial, &Plc::Alarm);
}

ImageProcess::~ImageProcess() {
	onStop();
	delete rMatcher;
};

void ImageProcess::handleFrame()
{
	makeFrame4Show(); //ֻҪcap�����źţ�����ʾ��������
	if (bUsrCtrl) {
		// ̨��ֹͣ��������㵱ǰ���֣���ֹͣ����
		// �����ڹ�翪��δ�ڹ涨ʱ���ڼ�⵽�뿪�źţ����߲⵽̨���ٶ�Ϊ0
		if (bIsTruckStopped) {
			if (!checkoutAlready)
			{
				checkoutWheel();
				checkoutAlready = true;
			}
		}
		else {
			//sensor triggered. ͨ��SENSOR IN/OUT�źſ�ʼ��ע�⣬1������ֻ�յ�1��in��1��out��/���������ֵļ�⣬������core�ķ���ֵ
			if (imProfile->sensorTriggered) {
				//int state = _DZRecorder.state(0);
				// ���ֽ��롢����DZ
				//if (state == LevelRecorder::HighLevel || state == LevelRecorder::PositiveEdge) //in detect area
				if (bInDz)
				{
					checkoutAlready = false;
					int nCore = coreImageProcess(); //0->1, Fragment++ at rise edge. NOTE: it can be 0, which means no circle detected
					if (nCore == LocateFail && nCore_pre > RMA) //nowCore == true && lastCore == false
						interrupts++;
					nCore_pre = nCore;
				}
				// �����뿪DZ������
				else {
					if (!checkoutAlready)
					{
						checkoutWheel();
						checkoutAlready = true;
					}
				}
			}
			//image triggered
			//ͨ��LMA��InMA�������ؿ�ʼ��InMA��RMA���½��ؽ��㳵�֣�
			// �߽練����������ʼ1(����)-->��ʼ2,3..�����㣩-->�ж�-->����1�����㣩-->����2,3..��������
			else {
				int nCore = coreImageProcess();
				// LMA->InMA������ ��ʼ
				if (nCore > RMA && nCore_pre == LMA) {
					// ��ʼ1
					if (_MAState != 0) {
						_MAState = 0;
					}
					// ��ʼ2,3...
					else {
						rtSpeeds.clear();
						refSpeeds.clear();
						interrupts = 0;
						//wheelFrame_pre.release(); // ��Ϊpre��LMA,����frame_pre��Ȼ�ǿյ�
					}
				}
				// InMA->RMA�½��� ����
				else if (nCore == RMA && nCore_pre > RMA) {
					// ����1
					if (_MAState != 1) {
						_MAState = 1;
						// ncore����ΪLocateFail��ʵ��Ӧ��ΪRMA, ������Ӱ���жϣ���pre==RMA��FindFail���ж�������
						// ��ʱwheelFrame_pre�Ѿ��Զ�release��
						checkoutWheel();
					}
				}
				// InMA->MissWheel �ж�
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
	undistortedFrame.copyTo(frameToShow); // �������ΪundistortedFrame����framToShow�����ݲ�ͬ
	cvtColor(frameToShow, frameToShow, CV_GRAY2BGR); // ת���ɲ�ͼ
	rectangle(frameToShow, imProfile->roiRect, Scalar(0, 0, 255), 2); // ��ͼ�л���ɫ�߿�
	mutex.unlock();
}

int ImageProcess::coreImageProcess()
{
	ocr->core_ocr(srcImg); //card num detect gzy 2017/11/9
	Mat monitorArea = undistortedFrame(imProfile->roiRect); //��ȡ�������
	equalizeHist(monitorArea, monitorArea); //equalize the image, this will change calibratedFrame//2017/12/21
	Mat blurImage; //use blur image for hough circles, use source image for matches

	// Reduce the noise to avoid false circle detection
	GaussianBlur(monitorArea, blurImage, Size(imProfile->gs1, imProfile->gs1), 2, 2); //the greater the kernel size is, the less time the hough cost
	// ����ƥ�䣬Ѱ�ҳ��ֵ���Բ��ע�⣺�÷�����ȡ��Բ���ܲ�����ͼ������
	vector<Vec3f> circles;
	int max_r = std::min(monitorArea.rows / 2, imProfile->radius_max);// ��ֹ���򳬳���ͨ���߶ȱȽ�С
	HoughCircles(blurImage, circles, CV_HOUGH_GRADIENT, imProfile->dp, imProfile->minDist, imProfile->param1, imProfile->param2, imProfile->radius_min, max_r);
	//�޻���Բ
	if (circles.empty()) {
		wheelFrame_pre.release(); // ��Ч֡�����������ÿ�ǰһ֡
		return LocateFail; // ���ֶ�λ��ʧ
	}

	//ȡ����Բ�����һ�������������⾶��ȡ���о�������ƥ��
	//patch��Ψһһ������Բ
	Point center0(cvRound(circles[0][0]), cvRound(circles[0][1])); // Բ��
	int radius = cvRound(circles[0][2]); // �뾶
	Rect wheelRect((center0.x - radius), (center0.y - radius), 2 * radius, 2 * radius); // ���о��ε�λ��

	//�жϳ����Ƿ��ڼ��������
	Rect monitorRect(0, 0, monitorArea.cols, monitorArea.rows);
	bool bInside = isInside(wheelRect, monitorRect);

	//draw circle to UI
	auto circleColor = bInside ? Scalar(0, 255, 0) : Scalar(0, 255, 255); // ������������̣����򻭻�
	Size size;
	Point ofs;
	monitorArea.locateROI(size, ofs); // get the offset of monitorArea from the srcImg
	mutex.lock();
	circle(frameToShow, center0 + ofs, 3, circleColor, -1); // center
	circle(frameToShow, center0 + ofs, radius, circleColor, 1, LINE_AA); // round
	mutex.unlock();

	// δ��ȫ�ڼ�������ڣ�����ʱӦע�����������ڱ߽�ķ�������
	if (!bInside) {
		//������MA����ߣ���δ����
		if (wheelRect.tl().x < monitorRect.tl().x) {
			wheelFrame_pre.release(); // �ÿ�ǰһ֡
			return LMA;
		}
		//������MA���Ҳ࣬�����˳�
		if (wheelRect.br().x > monitorRect.br().x) {
			wheelFrame_pre.release(); // �ÿ�ǰһ֡
			return RMA; //����б�Ӧ�ý���ó���
		}
		//���ֳ���MA�����½�
		if (wheelRect.tl().y < monitorRect.tl().y || wheelRect.br().y > monitorRect.br().y) {
			wheelRect = wheelRect & monitorRect;//ȡ����
		}
	}

	// ���˳ɹ������������ڵģ����֣�����
	Mat matchSrc1 = wheelFrame_pre;
	Mat matchSrc2 = monitorArea(wheelRect); //���о���ͼ��,ע�⣺wheelrect������monitor��
	wheelFrame_pre = matchSrc2;

	// ǰһ֡Ϊ��
	if (matchSrc1.empty()) {
		return NoPre; // �ȴ�ƥ��֡
	}

	// ��ʼƥ�� //orb matches
	Mat image_matches; // show in UI
	double oneAngle;
	if (!rMatcher->match(matchSrc1, matchSrc2, configHelper->matchMaskOuterRatio, configHelper->matchMaskInnerRatio, image_matches, oneAngle)) //200ms
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
	/* δ�ڴ˴���ʼ����wheelDbInfo, ֻ���ض�ʱ��valid��ʱ�����Ч��refspeedΪcoreProcessʱȡ�õ� */
	WheelDbInfo wheelDbInfo;
	wheelDbInfo.i_o = deviceIndex;
	wheelDbInfo.num = QString::fromStdString(ocr->get_final_result());
	// ��¼���е��ٶ�ֵ
	wheelDbInfo.speeds = genSpeeds(rtSpeeds, refSpeeds);
	wheelDbInfo.totalmatch = rtSpeeds.size(); // match size
	//������㹻�����ݣ����м���
	if (refSpeeds.size() >= IM_PROC_VALID_MIN_COUNT) {
		//����ÿһ�����ݵ���rt-ref=diff
		vector<double> absErrors(rtSpeeds.size());
		transform(rtSpeeds.begin(), rtSpeeds.end(), refSpeeds.begin(), absErrors.begin(), minus<double>());
		// �޳��쳣��ע�⣺�ܻ��н��
		OutlierHelper outlier;
		outlier.removeOutliers(absErrors);
		double refspeed = outlier.mean(refSpeeds);
		double absError = outlier.mean(absErrors);
		wheelDbInfo.validmatch = absErrors.size();
		// д�����ݿ��ֵ
		wheelDbInfo.calcspeed = QString::number(absError + refspeed, 'f', 2).toDouble();
		wheelDbInfo.refspeed = QString::number(refspeed, 'f', 2).toDouble();
		wheelDbInfo.error = QString::number(absError / refspeed * 100, 'f', 1).toDouble();
		qDebug() << "Speed: " << wheelDbInfo.calcspeed << ";Ref: " << wheelDbInfo.refspeed;
	}
	//����Ч����
	else
	{
		wheelDbInfo.validmatch = 0;
		wheelDbInfo.calcspeed = IM_PROC_INVALID_SPEED; // ����ֵΪ���
		wheelDbInfo.refspeed = rtRefSpeed; // ��ǰ�ο��ٶ�
		wheelDbInfo.error = 0.0;
		if (wheelDbInfo.totalmatch == 0)
		{
			qCritical() << QStringLiteral("��ƥ����");
		}
		else
		{
			qCritical() << QStringLiteral("ƥ����̫��");
		}
	}
	wheelDbInfo.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	wheelDbInfo.ocrsize = ocr->size();
	wheelDbInfo.fragment = interrupts;

	wheelDbInfo.videopath = videoCapture->getVideoRelativeFilePath();
	wheelDbInfo.warnRatio = imProfile->warningRatio;
	wheelDbInfo.alarmRatio = imProfile->alarmRatio;

	emit wheelNeedHandled(wheelDbInfo);
	/* reset parameters of this wheel */
	clearWheel();
}
//! ����д�����ݿ�speeds�е�����
QString ImageProcess::genSpeeds(vector<double> rt, vector<double> ref)
{
	QStringList rt_str;
	for (auto sp : rt) {
		rt_str << QString::number(sp, 'f', 2);
	}
	QStringList ref_str;
	for (auto sp : ref) {
		ref_str << QString::number(sp, 'f', 2);
	}
	QString speeds = rt_str.join(' ') + ";" + ref_str.join(' ');
	return speeds;
}

void ImageProcess::clearWheel()
{
	rtSpeeds.clear();
	refSpeeds.clear();
	interrupts = 0;
	//��ֹ����һ�����ָ��ţ�ֻ��sensor triggered��Ч,image tri�ᣨ��Ŀǰ���Ƶ������£��Զ������������
	nCore_pre = LocateFail;
	wheelFrame_pre.release();
}

void ImageProcess::onSensorIN()
{
	//QMutexLocker locker(&mutex);// ���±���ȫ��ֻ�б��̲߳��� [10/12/2018 cx3386]
	bIsTruckStopped = false;
	//_DZRecorder.push(true);
	bInDz = true;
	//qDebug() << "dev[" << deviceIndex << "] DZ In";
}
void ImageProcess::onSensorOUT()
{
	//QMutexLocker locker(&mutex);
	//_DZRecorder.push(false);
	bInDz = false;//�´�handleframeʱ���ٴ���coreProc
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
	// �������̲߳���������Ҫ��
	if (bUsrCtrl) {
		qWarning() << "ImageProcess: repeated start";
		return;
	}
	bInDz = false;
	_MAState = 1; //��˲��ᴥ������1
	//nCore_pre = FindFail;
	bUsrCtrl = true;
	//����insert
	//QTimer *timer = new QTimer;
	//connect(timer, &QTimer::timeout, this, [=]() {insertRecord(WheelDbInfo()); });
	//timer->start(1000);
}

void ImageProcess::onStop()
{
	if (!bUsrCtrl) {
		qWarning() << "ImageProcess: repeated stop";
		return;
	}
	// �������жϣ����������Ϊԭʼ״̬
	ocr->resetOcr(); // ocr����
	checkoutAlready = true;
	clearWheel();
	bUsrCtrl = false;
}