#pragma once
#include <QObject>
#include <opencv2/opencv.hpp>
#include "database.h"

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
	void start();
	void stop();
private:
	const ConfigHelper *configHelper;
	HikVideoCapture *videoCapture;
	PLCSerial *plcSerial;
	int deviceIndex;
	const ImProfile *imProfile;
	OCR * ocr;
	RobustMatcher *rMatcher;

	WheelDbInfo wheelDbInfo;
	cv::Mat srcImg;
	cv::Mat calibratedFrame;
	cv::Mat frameToShow;
	QMutex mutex;

	cv::Mat cameraUndistort(cv::Mat src);
	void alarmThisWheel();
	int previousAlarmLevel(const QString & num) const;
	bool insertRecord(const WheelDbInfo &info);
	void handleAlarmLevel(int lv);
	int coreImageProcess(); // 0-no cycle, 1-matches success
	void foreverPreProcess();

	std::vector<double> rtSpeeds;
	std::vector<double> refSpeeds;
	int nImgCount = 0; // has 0/1/2 src imgs to match. when 2, begin to match
				   // process; when 0,1, wait the next src img

	bool bIsTrolleyInSensors = false;
	bool bWheelStopped = false;
	bool bIsProcessing = false;
	bool bIsTrolleyStopped = false;
	double rtRefSpeed;
signals:
	void initModel();
	void imageProcessReady();
	void showRealtimeImage();
	void setAlarmLight(AlarmColor alarmcolor);

	public slots:
	void setupModel();
	void doImageProcess();
	void onSensorIN();
	void onSensorOUT();
	void onWheelTimeout();
};
