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
	// ����mainwindow���߳̿���
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
	OCR* ocr; //!< ocrʶ����ֻ࣬ͨ��start/stop�����ƣ�����
	RobustMatcher* rMatcher;

	cv::Mat srcImg;
	cv::Mat undistortedFrame;
	cv::Mat frameToShow;
	cv::Mat matchResult;
	QMutex mutex;

	cv::Mat cameraUndistort(cv::Mat src);
	void checkoutWheel(); //!< ����ó���

	QString genSpeeds(std::vector<double> rt, std::vector<double> ref);

	void clearWheel(); //!< �����ǰ���ֵ������Ϣ

	//int previousAlarmLevel(const QString& num);
	//QSqlTableModel *previousModel;
	//bool insertRecord(const WheelDbInfo& info);
	//QSqlTableModel *insertModel;
	//void handleAlarmLevel(WheelDbInfo& wheelDbInfo);
	//! �ж�rect1�Ƿ���rect2
	static bool isInside(cv::Rect rect1, cv::Rect rect2)
	{
		return (rect1 == (rect1 & rect2));
	}
	enum CoreState {
		LocateFail = 0x1, //!< ���ֶ�λ��ʧ
		OutMA = 0x2, //!< ������MA��
		LMA = OutMA | 0x4, //!< ������MA��
		RMA = OutMA | 0x8, //!< ������MA��
		NoPre = 0x10, //!< ��ǰ֡
		MatchFail = 0x20, //!< ƥ��ʧ��
		TruckSpZero = 0x40, //!< �����ٶ�Ϊ0
		Success = 0x80, //!< ƥ����ɣ������˳�
	};

	int coreImageProcess(); //!< ͼ������Ĵ��룬����CoreStates������ΪscrImg��undistortedFrame
	void preprocess();

	std::vector<double> rtSpeeds; //!< ��ǰ���ֵļ����ٶȱ�
	std::vector<double> refSpeeds; //!< ��ǰ���ֵĲο��ٶȱ���rtSpeeds��С��ͬ
	//int nImgCount = 0; // has 0/1/2 src imgs to match. when 2, begin to match
	// process; when 0,1, wait the next src img

	cv::Mat wheelFrame_pre; //!< ��һ֡ �������о���ͼ��
	bool bInDz = false; //!< ���ִ���detect zone��
	bool checkoutAlready = true; //!< �Ƿ���Ҫcheckout����ֹ�ظ�
	//LevelRecorder _DZRecorder; //!< ��¼�����Ƿ�����翪�صļ�ⷶΧDZ(detect zone) ����Ӳ����� start��ʼ��posh 0,��stop����,ss-tri�л���ҲӦ���ã���Ϊ����MA�Ѿ������1�ˣ��������ؽ��㣩
	int _MAState; //!< 0-�� 1-�� 2�ж� start��ʼ��Ϊ1��ss-tri״̬�л�������1��TODO��
	//LevelRecorder _MARecorder; //!< ��¼�����Ƿ������ļ������MA(monitor area) ����������
	int interrupts = 0; //!< ������MA������ƥ������У��жϵĴ���,stop����Ϊ0
	bool bUsrCtrl = false; ///< ctrl the imageprocecss running state 1 is ON, 0 is OFF
	bool bIsTruckStopped = false;
	int nCore_pre = LocateFail; //!< �汾������һ��core�ķ���ֵ����ʼ��Ϊfindfail
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
	void handleFrame(); //!< ����ǰ֡��ÿ����3��

	void makeFrame4Show();

	void onSensorIN(); //!< ��ȫ����󣨽���
	void onSensorOUT(); //!< ��ʼ�˳�ǰ����ǰ��
	void onWheelTimeout();
private slots:
	//! ��ʼ����
	void onStart();
	//! ֹͣ�����Ա���ÿ֡��ʾ�Ĺ���
	void onStop();
	//void setupModel();
};
