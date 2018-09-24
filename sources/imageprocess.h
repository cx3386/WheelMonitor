#pragma once
#include <QObject>
#include <opencv2/opencv.hpp>
#include "database.h"
#include "LevelRecorder.h"

const int IM_PROC_VALID_MIN_COUNT = 10;
const int IM_PROC_INVALID_SPEED = 888;

class PLCSerial;
class RobustMatcher;
class OCR;
struct ImProfile;
class ConfigHelper;
class HikVideoCapture;
enum AlarmColor;

class ImageProcess : public QObject {
	Q_OBJECT
public:
	ImageProcess(const ConfigHelper *_configHelper, HikVideoCapture *_capture, PLCSerial *_plcSerial, QObject *parent = Q_NULLPTR);
	~ImageProcess();

	inline cv::Mat getFrameToShow() { QMutexLocker locker(&mutex); return frameToShow; }
	/// ��ʼ����
	void start();
	/// ֹͣ�����Ա���ÿ֡��ʾ�Ĺ���
	void stop();
private:
	const ConfigHelper *configHelper;
	HikVideoCapture *videoCapture;
	PLCSerial *plcSerial;
	int deviceIndex;
	const ImProfile *imProfile;
	OCR * ocr;  //!< ocrʶ����ֻ࣬ͨ��start/stop�����ƣ�����
	RobustMatcher *rMatcher;

	cv::Mat srcImg;
	cv::Mat undistortedFrame;
	cv::Mat frameToShow;
	QMutex mutex;

	cv::Mat cameraUndistort(cv::Mat src);
	void checkoutWheel(); //!< ����ó���
	void clearWheel(); //!< �����ǰ���ֵ������Ϣ

	int previousAlarmLevel(const QString & num) const;
	bool insertRecord(const WheelDbInfo &info);
	void handleAlarmLevel(WheelDbInfo & wheelDbInfo);
	//! �ж�rect1�Ƿ���rect2
	static bool isInside(cv::Rect rect1, cv::Rect rect2)
	{
		return (rect1 == (rect1&rect2));
	}
	enum CoreState
	{
		LocateFail = 0x1, //!< ���ֶ�λ��ʧ
		OutMA = 0x2,//!< ������MA��
		LMA = OutMA | 0x4, //!< ������MA��
		RMA = OutMA | 0x8, //!< ������MA��
		NoPre = 0x10, //!< ��ǰ֡
		MatchFail = 0x20, //!< ƥ��ʧ��
		TruckSpZero = 0x40, //!< �����ٶ�Ϊ0
		Success = 0x80, //!< ƥ����ɣ������˳�
	};
	/*!
	 * \brief ����ͼ��ĺ��Ĵ���
	 *
	 * \return CoreState
	 */
	int coreImageProcess();
	void preprocess();

	std::vector<double> rtSpeeds; //!< ��ǰ���ֵļ����ٶȱ�
	std::vector<double> refSpeeds; //!< ��ǰ���ֵĲο��ٶȱ���rtSpeeds��С��ͬ
	//int nImgCount = 0; // has 0/1/2 src imgs to match. when 2, begin to match
				   // process; when 0,1, wait the next src img

	cv::Mat wheelFrame_pre;  //!< ��һ֡ �������о���ͼ��
	LevelRecorder _DZRecorder; //!< ��¼�����Ƿ�����翪�صļ�ⷶΧDZ(detect zone) ����Ӳ����� start��ʼ��posh 0,��stop����,ss-tri�л���ҲӦ���ã���Ϊ����MA�Ѿ������1�ˣ��������ؽ��㣩
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
	void initModel();
	void frameHandled();
	void showFrame();
	void setAlarmLight(AlarmColor alarmcolor);

public slots:
	void setupModel();
	void handleFrame();

	void makeFrame4Show();

	void onSensorIN(); //!< ��ȫ����󣨽���
	void onSensorOUT(); //!< ��ʼ�˳�ǰ����ǰ��
	void onWheelTimeout();
};
