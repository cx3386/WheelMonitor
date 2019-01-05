#pragma once
#include "LevelRecorder.h"
#include "database.h"
#include <QObject>
#include <opencv2/opencv.hpp>

const int IM_PROC_VALID_MIN_COUNT = 10;
const int IM_PROC_INVALID_SPEED = 888;

class Plc;
class RobustMatcher;
class OCR;
struct ImProfile;
class ConfigHelper;
class HikVideoCapture;
enum class AlarmColor;

class ImageProcess : public QObject {
	Q_OBJECT
public:
	explicit ImageProcess(const ConfigHelper* _configHelper, HikVideoCapture* _capture, Plc* _plcSerial, QObject* parent = Q_NULLPTR);
	~ImageProcess();

	cv::Mat getFrameToShow() { QMutexLocker locker(&mutex); return frameToShow; }
	cv::Mat getMatchResult() { QMutexLocker locker(&mutex); return matchResult; }
	// 用于mainwindow跨线程控制
	void start() { QTimer::singleShot(0, this, &ImageProcess::onStart); }
	void stop() { QTimer::singleShot(0, this, &ImageProcess::onStop); }
	//void init() { QTimer::singleShot(0, this, &ImageProcess::setupModel); }
	//int getDevId() { QMutexLocker locker(&mutex); return deviceIndex; }

private:
	const ConfigHelper* configHelper;
	HikVideoCapture* videoCapture;
	Plc* plcSerial;
	int deviceIndex;
	const ImProfile* imProfile;
	OCR* ocr; //!< ocr识别的类，只通过start/stop来控制，常开
	RobustMatcher* rMatcher;

	cv::Mat srcImg;
	cv::Mat undistortedFrame;
	cv::Mat frameToShow;
	cv::Mat matchResult;
	QMutex mutex;

	cv::Mat cameraUndistort(cv::Mat src);
	void checkoutWheel(); //!< 结算该车轮

	QString genSpeeds(std::vector<double> rt, std::vector<double> ref);

	void clearWheel(); //!< 清除当前车轮的相关信息

	//int previousAlarmLevel(const QString& num);
	//QSqlTableModel *previousModel;
	//bool insertRecord(const WheelDbInfo& info);
	//QSqlTableModel *insertModel;
	//void handleAlarmLevel(WheelDbInfo& wheelDbInfo);
	//! 判断rect1是否在rect2
	static bool isInside(cv::Rect rect1, cv::Rect rect2)
	{
		return (rect1 == (rect1 & rect2));
	}
	enum CoreState {
		LocateFail = 0x1, //!< 车轮定位丢失
		OutMA = 0x2, //!< 车轮在MA外
		LMA = OutMA | 0x4, //!< 车轮在MA左
		RMA = OutMA | 0x8, //!< 车轮在MA右
		NoPre = 0x10, //!< 无前帧
		MatchFail = 0x20, //!< 匹配失败
		TruckSpZero = 0x40, //!< 车轮速度为0
		Success = 0x80, //!< 匹配完成，正常退出
	};

	int coreImageProcess(); //!< 图像处理核心代码，返回CoreStates，输入为scrImg和undistortedFrame
	void preprocess();

	std::vector<double> rtSpeeds; //!< 当前车轮的计算速度表
	std::vector<double> refSpeeds; //!< 当前车轮的参考速度表，与rtSpeeds大小相同
	//int nImgCount = 0; // has 0/1/2 src imgs to match. when 2, begin to match
	// process; when 0,1, wait the next src img

	cv::Mat wheelFrame_pre; //!< 上一帧 车轮外切矩形图像
	bool bInDz = false; //!< 车轮处于detect zone里
	bool checkoutAlready = true; //!< 是否需要checkout，防止重复
	//LevelRecorder _DZRecorder; //!< 记录车轮是否进入光电开关的检测范围DZ(detect zone) 用于硬件检测 start初始化posh 0,在stop重置,ss-tri切换后也应重置（因为可能MA已经结算过1此，将导致重结算）
	int _MAState; //!< 0-进 1-出 2中断 start初始化为1，ss-tri状态切换后重置1（TODO）
	//LevelRecorder _MARecorder; //!< 记录车轮是否进入红框的监控区域MA(monitor area) 用于软件检测
	int interrupts = 0; //!< 本车轮MA内连续匹配过程中，中断的次数,stop重置为0
	bool bUsrCtrl = false; ///< ctrl the imageprocecss running state 1 is ON, 0 is OFF
	bool bIsTruckStopped = false;
	int nCore_pre = LocateFail; //!< 存本车轮上一次core的返回值，初始化为findfail
	double rtRefSpeed;
signals:
	void _MAIn();
	void _MAOut();
	void frameHandled();
	void showFrame();
	//void setAlarmLight(int alarmcolor);
	void showMatch();
	void wheelNeedHandled(WheelDbInfo info);
	//void insertRecord();

public slots:
	void handleFrame(); //!< 处理当前帧，每秒钟3次

	void makeFrame4Show();

	void onSensorIN(); //!< 完全进入后（进后）
	void onSensorOUT(); //!< 开始退出前（出前）
	void onWheelTimeout();
private slots:
	//! 开始处理
	void onStart();
	//! 停止处理，仍保留每帧显示的功能
	void onStop();
	//void setupModel();
};
