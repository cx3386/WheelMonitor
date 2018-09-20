/**
 * \file hikvideocapture.h
 *
 * \author cx3386
 * \date 九月 2018
 *
 * \brief 采集录像
 *
 * 在构造函数时初始化与摄像机的连接，如果连接失败，需要排查连接问题后重启APP。偶尔中断，有自动重连
 * start()和stop()只用于控制视频流的读取/停止读取，并不中断连接。
 * 大坑：调用的海康SDK是静态函数，需要根据对象的deviceIndex分别调用Decfunc0,Decfunc1
 *
 */
#pragma once
#include <QObject>
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include <opencv2/opencv.hpp>

class ConfigHelper;
struct CamProfile;
class HikVideoCapture : public QObject
{
	Q_OBJECT

public:
	HikVideoCapture(const ConfigHelper *_configHelper, int _deviceIndex, HWND h, QObject *parent = Q_NULLPTR);
	~HikVideoCapture();

	inline QString getVideoRelativeFilePath() { QMutexLocker locker(&mutex); return videoRelativeFilePath; }
	inline int getDeviceIndex() const { return deviceIndex; }
	inline cv::Mat getRawImage() { QMutexLocker locker(&mutex); return rawImage.clone(); }
	//static inline cv::Mat getRawImage(HikVideoCapture *p) { return p->getRawImage(); }

	/*called by mainwindow*/
	void syncCameraTime();
	bool start();
	bool stop();
private:
	const ConfigHelper * configHelper;
	int deviceIndex;
	HWND hPlayWnd;
	const CamProfile * camProfile;

	QMutex mutex;

	bool bIsRecording = false;
	QString videoRelativeFilePath; ///< the fileName of save video.
	QTimer *timer;
	const int MAX_RECORD_MSEC = 100000;

	cv::Mat rawImage;
	const int nHandling_Start = 3; ///< 于开始视频流后进行一次初始化
	volatile int nHandling;///< the countdown counter 倒计数器
	//bool bIsProcessing = false;
	int nPendingFrame = 0;///< 图像线程处理时，对于本线程来说就是待处理的帧，由于不缓存，只能是0或1. 0 is ready for next cap, 1 must wait for processed
	LONG lUserID;
	LONG nPort = -1;
	LONG lRealPlayHandle_SD = -1; ///< for save video
	LONG lRealPlayHandle_HD = -1; ///< for process and show
	NET_DVR_PREVIEWINFO struPlayInfo_HD; ///< for process and show
	NET_DVR_PREVIEWINFO struPlayInfo_SD; ///< for save
	void DecCBFun(char* pBuf, FRAME_INFO *pFrameInfo);
	static void CALLBACK DecCBFun0(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK DecCBFun1(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
	static void CALLBACK ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
	static HikVideoCapture *pCap0, *pCap1;

signals:
	/// a frame is captured, need to be processed
	void frameCaptured();
	/// record out of time, will auto stop and save video
	void recordTimeout();
	/// now is recording
	void recordON();
	/// now record stopped
	void recordOFF();

public slots:
	void startRecord();
	void stopRecord();
	inline void frameProcessed() { nPendingFrame--; }
};