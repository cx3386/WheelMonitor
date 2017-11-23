#pragma once
#include <QObject>
//#include <Windows.h>
//#include <cstdlib>
//#include <cstring>
//#include <iostream>
#include "HCNetSDK.h"
#include "plaympeg4.h"

#include <QMutex>
//#include <time.h>
#include <opencv2/opencv.hpp>

class HikVideoCapture : public QObject
{
	Q_OBJECT
		static void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
	static void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, DWORD dwUser);
	static void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK fDrawFun(LONG lRealHandle, HDC hDc, DWORD dwUser);

public:
	explicit HikVideoCapture(QObject *parent = nullptr);
	~HikVideoCapture();

	static cv::Mat pRawImage;
	static HikVideoCapture *pVideoCapture;
	static int capInterval;
	QString saveDirLink;

private:
	static volatile int gbHandling;
	static LONG nPort;
	static volatile bool bIsProcessing;
	static QMutex mutex;
	LONG lUserID;
	LONG lRealPlayHandle_SD;
	LONG lRealPlayHandle_HD;
	bool bIsSaving;
	bool bSaveTimeout;
	QTimer *timer;

signals:
	void imageNeedProcess();
	void isStartCap(bool result);
	void isStopCap(bool result);
	void wheelTimeout();
	void saveStopped();
	//void isStartSave(bool result);
	//void isStopSave(bool result);

	public slots :
	bool syncCameraTime();
	bool startCap(HWND h);
	bool stopCap();
	bool startSave();
	bool stopSave();
	//bool timeoutSave();
	void imageProcessReady();
};
