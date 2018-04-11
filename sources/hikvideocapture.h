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
	static inline cv::Mat getRawImage(HikVideoCapture *p) { return p->getRawImage(); }

	/*called by mainwindow*/
	void syncCameraTime();
	bool startCapture();
	bool stopCapture();
private:
	const ConfigHelper * configHelper;
	int deviceIndex;
	HWND hPlayWnd;
	const CamProfile * camProfile;

	QMutex mutex;

	bool bIsRecording = false;
	QString videoRelativeFilePath; ///< the fileName of save video. #TODO 目录结构应该分内外圈
	QTimer *timer;
	const int MAX_RECORD_MSEC = 100000;

	cv::Mat rawImage;
	volatile int gbHandling = 3;
	bool bIsProcessing = false;
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
	void captureOneImage();
	void recordTimeout();
	void recordON();
	void recordOFF();

	public slots:
	void startRecord();
	void stopRecord();
	inline void currentImageProcessReady() { bIsProcessing = false; }
};