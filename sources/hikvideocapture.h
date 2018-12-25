/**
 * \file hikvideocapture.h
 *
 * \author cx3386
 * \date ���� 2018
 *
 * \brief �ɼ�¼��
 *
 * �ڹ��캯��ʱ��ʼ��������������ӣ��������ʧ�ܣ���Ҫ�Ų��������������APP��ż���жϣ����Զ�����
 * start()��stop()ֻ���ڿ�����Ƶ���Ķ�ȡ/ֹͣ��ȡ�������ж����ӡ�
 * ��ӣ����õĺ���SDK�Ǿ�̬��������Ҫ���ݶ����deviceIndex�ֱ����Decfunc0,Decfunc1
 *
 */
#pragma once
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include <QObject>
#include <opencv2/opencv.hpp>

class ConfigHelper;
struct CamProfile;
class HikVideoCapture : public QObject {
	Q_OBJECT

public:
	HikVideoCapture(const ConfigHelper* _configHelper, int _deviceIndex, HWND h, QObject* parent = Q_NULLPTR);
	~HikVideoCapture();

	inline QString getVideoRelativeFilePath() const
	{
		QMutexLocker locker(&mutex);
		return videoRelativeFilePath;
	}
	inline int getDeviceIndex() const { return deviceIndex; }
	inline cv::Mat getRawImage() const
	{
		QMutexLocker locker(&mutex);
		return rawImage.clone();
	}
	//static inline cv::Mat getRawImage(HikVideoCapture *p) { return p->getRawImage(); }

	/*called by mainwindow*/
	void syncCameraTime();
	bool start();
	bool stop();
	// ���봫��Ĳ�����δ����飬���û�д��룬�ᷢ������
	//void setRealPlayWND(HWND val) { hPlayWnd = val; } //!< ����ʵʱԤ���Ĵ��ھ��
private:
	// ��ʼ���б������ڹ��캯���ĺ�����ִ�У����ҳ�Ա�ĳ�ʼ���б����Ա������˳����ͬ�� [10/9/2018 cx3386]
	// ������ע������˳�� [10/9/2018 cx3386]
	const ConfigHelper* configHelper;
	int deviceIndex;
	HWND hPlayWnd; //���Ŵ��ھ��
	const CamProfile* camProfile;

	mutable QMutex mutex;

	bool bIsRecording = false; //!< timeoutTimer��plc��sensorout������̸߳ı����������Ҫ�̱߳���//ԭ�Ӳ��������ñ���
	QString videoRelativeFilePath; ///< the fileName of save video.
	QTimer* timeoutTimer; //!< ¼�Ƶĳ�ʱ��ʱ����ֹͣ¼��ʱֹͣ
	const int MAX_RECORD_MSEC = 100000; //!< ¼�Ƶĳ�ʱʱ�����������Զ�ֹͣ¼�ƣ�������timeout���ź�

	cv::Mat rawImage;
	const int nHandling_Start = 3; ///< �ڿ�ʼ��Ƶ�������һ�γ�ʼ��
	volatile int nHandling; ///< the countdown counter ��������
	//bool bIsProcessing = false;
	int nPendingFrame = 0; ///< ͼ���̴߳���ʱ�����ڱ��߳���˵���Ǵ������֡�����ڲ����棬ֻ����0��1. 0 is ready for next cap, 1 must wait for processed
	LONG lUserID;
	LONG nPort = -1;
	LONG lRealPlayHandle_SD = -1; ///< for save video
	LONG lRealPlayHandle_HD = -1; ///< for process and show
	NET_DVR_PREVIEWINFO struPlayInfo_HD; ///< for process and show
	NET_DVR_PREVIEWINFO struPlayInfo_SD; ///< for save
	void DecCBFun(char* pBuf, FRAME_INFO* pFrameInfo);
	static void CALLBACK DecCBFun0(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK DecCBFun1(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize, void* pUser);
	static void CALLBACK ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void* pUser);
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
	void startRecord(); //! �漰��timeoutTimer�����ܿ��̣߳�����ͨ��sg/sl���Ƶ���
	void stopRecord(); //!  �漰��timeoutTimer�����ܿ��̣߳�����ͨ��sg/sl���Ƶ���
	inline void frameProcessed() { nPendingFrame--; }
};