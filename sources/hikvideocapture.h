#pragma once
#include <QObject>
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include <opencv2/opencv.hpp>

class ConfigHelper;
class CamProfile;
class HikVideoCapture : public QObject
{
	Q_OBJECT

public:
	HikVideoCapture(const ConfigHelper *confHelper, int _deviceIndex, HWND h, QObject *parent = Q_NULLPTR);
	~HikVideoCapture();

	inline QString getVideoRelativeFilePath() { QMutexLocker locker(&mutex); return videoRelativeFilePath; }
	inline int getDeviceIndex() const { return deviceIndex; }
	inline cv::Mat getRawImage() { QMutexLocker locker(&mutex); return rawImage.clone(); }
	static inline cv::Mat getRawImage(HikVideoCapture *p) { return p->getRawImage(); }

private:
	const ConfigHelper * configHelper;
	const CamProfile * camProfile;
	int deviceIndex;
	HWND hPlayWnd;

	QMutex mutex;

	bool bIsSaving = false;
	QString videoRelativeFilePath;
	QTimer *timer;
	const int MAX_RECORD_MSEC = 100000;

	cv::Mat rawImage;
	volatile int gbHandling = 3;
	bool bIsProcessing = false;
	LONG lUserID;
	LONG nPort = -1;
	LONG lRealPlayHandle_SD = -1; ///< used to save video
	LONG lRealPlayHandle_HD = -1; ///< used to process and show
	NET_DVR_PREVIEWINFO struPlayInfo_HD;

	static void CALLBACK DecCBFun1(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK DecCBFun2(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
	static void CALLBACK ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
	static HikVideoCapture *pCap1, *pCap2;

signals:
	void captureOneImage();
	void isStartCap(bool result);
	void isStopCap(bool result);
	void recordTimeout();
	void recordON();
	void recordOFF();

	public slots:
	void DecCBFun(char* pBuf, FRAME_INFO *pFrameInfo);
	bool syncCameraTime();
	bool startCapture();
	bool stopCapture();
	void startRecord();
	void stopRecord(bool bRecordTimeout);
	void stopRecord();
	void currentImageProcessReady();
};