#include "stdafx.h"
#include "hikvideocapture.h"
#include "confighelper.h"
#include "common.h" //capsavedir

using namespace std;
using namespace std::placeholders;
using namespace cv;

HikVideoCapture *HikVideoCapture::pCap0, *HikVideoCapture::pCap1;

HikVideoCapture::HikVideoCapture(const ConfigHelper *_configHelper, int _deviceIndex, HWND h, QObject *parent)
	: QObject(parent)
	, configHelper(_configHelper)
	, deviceIndex(_deviceIndex)
	, hPlayWnd(h)
	, camProfile(&(configHelper->device[deviceIndex].camProfile))
{
	struPlayInfo_HD = {
		1, // LONG lChannel;//ͨ����
		0, // DWORD dwStreamType;    // �������ͣ�0-��������1-��������2-����3��3-����4 ���Դ�����
		0, // DWORD dwLinkMode;// 0��TCP��ʽ,1��UDP��ʽ,2���ಥ��ʽ,3 - RTP��ʽ��4-RTP/RTSP,5-RSTP/HTTP
		hPlayWnd, // HWND hPlayWnd;//���Ŵ��ڵľ��,ΪNULL��ʾ������ͼ��
		1 };	// DWORD bBlocked;  //0-������ȡ��, 1-����ȡ��, �������SDK�ڲ�connectʧ�ܽ�����5s�ĳ�ʱ���ܹ�����,���ʺ�����ѯȡ������.
	struPlayInfo_SD = {
		1, // lChannel
		1, // dwStreamType //������ΪSD
		0, // dwLinkMode
		0, // hPlayWnd
		0 }; // bBlocked //����Ҫ�ص������Է�����
	NET_DVR_Init();
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;

	auto ip = str2charx(camProfile->camIP.toStdString());
	auto user = str2charx(camProfile->camUserName.toStdString());
	auto pwd = str2charx(camProfile->camPassword.toStdString());
	lUserID = NET_DVR_Login_V30(ip, camProfile->camPort, user, pwd, &struDeviceInfo);
	if (lUserID < 0)
	{
		qDebug("HikVideoCapture: IPC Login error, %d", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	NET_DVR_SetExceptionCallBack_V30(0, nullptr, ExceptionCallBack, nullptr);
	syncCameraTime();

	timer = new QTimer;
	timer->setSingleShot(true);
	timer->setInterval(MAX_RECORD_MSEC);
	connect(timer, &QTimer::timeout, this, [&] {stopRecord(); emit recordTimeout(); });
}

HikVideoCapture::~HikVideoCapture()
{
	stopRecord(); //stop record when quit
	//timer->deleteLater();
	NET_DVR_Logout(lUserID);
	NET_DVR_Cleanup();
}
void HikVideoCapture::syncCameraTime()
{
	/*set camera time*/
	NET_DVR_TIME struDVRTime;
	struDVRTime.dwYear = QDateTime::currentDateTime().toString("yyyy").toLong();
	struDVRTime.dwMonth = QDateTime::currentDateTime().toString("MM").toLong();
	struDVRTime.dwDay = QDateTime::currentDateTime().toString("dd").toLong();
	struDVRTime.dwHour = QDateTime::currentDateTime().toString("hh").toLong();
	struDVRTime.dwMinute = QDateTime::currentDateTime().toString("mm").toLong();
	struDVRTime.dwSecond = QDateTime::currentDateTime().toString("ss").toLong();
	if (!NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_TIMECFG, 0xFFFFFFFF, &(struDVRTime), sizeof(NET_DVR_TIME)))
	{
		qDebug("NET_DVR_SET_TIME error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
	}
}

bool HikVideoCapture::startCapture()
{
	if ((lRealPlayHandle_HD != -1) && (lRealPlayHandle_SD != -1))
	{
		qDebug() << "HikVideoCapture: Duplicate start";
		return true;// already started
	}
	lRealPlayHandle_HD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_HD, fRealDataCallBack, this);
	lRealPlayHandle_SD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_SD, nullptr, nullptr);
	if ((lRealPlayHandle_HD == -1) || (lRealPlayHandle_SD == -1))
	{
		qDebug("NET_DVR_RealPlay_V40 error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return false;
	}
	return true;
}

bool HikVideoCapture::stopCapture()
{
	stopRecord();
	bool ret = true;
	if (lRealPlayHandle_HD != -1)
	{
		ret &= NET_DVR_StopRealPlay(lRealPlayHandle_HD);
		lRealPlayHandle_HD = -1;
	}
	if (lRealPlayHandle_SD != -1)
	{
		ret &= NET_DVR_StopRealPlay(lRealPlayHandle_SD);
		lRealPlayHandle_SD = -1;
	}
	//if handle==-1, already stopped, return true
	return ret;
}

void HikVideoCapture::startRecord()
{
	if (bIsRecording) return;
	auto date = QDate::currentDate().toString("yyyyMMdd");
	auto dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	auto mark = getDeviceMark(deviceIndex);
	mutex.lock();
	videoRelativeFilePath = QStringLiteral("%1/%2_%3.mp4").arg(date).arg(mark).arg(dateTime);
	mutex.unlock();
	QString videoFilePath = QStringLiteral("%1/%2").arg(videoDirPath).arg(videoRelativeFilePath);
	QByteArray ba = videoFilePath.toLocal8Bit();
	if (!NET_DVR_SaveRealData(lRealPlayHandle_SD, ba.data()))
	{
		qDebug() << "HikVideoCapture: startRecord error";
		return;
	}
	timer->start(); //wait 100s	//If the timer is already running, it will be stopped and restarted.
	bIsRecording = true;
	emit recordON();
}

void HikVideoCapture::stopRecord()
{
	if (!bIsRecording) return;
	if (!NET_DVR_StopSaveRealData(lRealPlayHandle_SD))
	{
		qDebug() << "HikVideoCapture: stopRecord error";
		return;
	}
	timer->stop();//stop the timeout timer
	bIsRecording = false;
	emit recordOFF();
}

void HikVideoCapture::DecCBFun(char *pBuf, FRAME_INFO *pFrameInfo)
{
	if (--gbHandling)
	{
		return;
	}
	gbHandling = camProfile->frameInterv;
	static int waitcount = 0;
	if (bIsProcessing)
	{
		//every 500 wait, write to log
		if (++waitcount == 500) {
			qDebug("Image is waiting for processing! Please increase gbhanding");
			waitcount = 0;
		}
		return;
	}
	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_YV12)
	{
		cv::Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1); //pImg = Gray
		cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
		cvtColor(src, pImg, CV_YUV2GRAY_YV12);
		mutex.lock();
		rawImage = pImg;
		mutex.unlock();
		bIsProcessing = true; // bool need no mutex, because bool is guaranteed atomic operations
		emit captureOneImage(); // ����֪ͨ,��������(bStop)
	}
}

void CALLBACK HikVideoCapture::DecCBFun0(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	pCap0->DecCBFun(pBuf, pFrameInfo);
}

void CALLBACK HikVideoCapture::DecCBFun1(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	pCap1->DecCBFun(pBuf, pFrameInfo);
}

/// ʵʱ���ص�
void CALLBACK HikVideoCapture::fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) //void *pUser
{
	auto *pCap = static_cast<HikVideoCapture*>(pUser);
	LONG &nPort = pCap->nPort;
	auto DecCBFun = [pCap]()->auto {
		int index = pCap->deviceIndex;
		switch (index)
		{
		case 0:
			HikVideoCapture::pCap0 = pCap;
			return HikVideoCapture::DecCBFun0;
		case 1:
			HikVideoCapture::pCap1 = pCap;
			return HikVideoCapture::DecCBFun1;
		default:
			break;
		}
	};
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //ϵͳͷ

		if (!PlayM4_GetPort(&nPort))
		{ //��ȡ���ſ�δʹ�õ�ͨ����
			break;
		}
		//m_iPort = lPort; //��һ�λص�����ϵͳͷ������ȡ�Ĳ��ſ�port�Ÿ�ֵ��ȫ��port���´λص�����ʱ��ʹ�ô�port�Ų���
		if (dwBufSize > 0)
		{
			if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME))
			{ //����ʵʱ������ģʽ
				break;
			}

			if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 10 * 1024 * 1024))
			{ //�����ӿ�
				break;
			}

			if (!PlayM4_Play(nPort, nullptr))
			{ //���룬������
				break;
			}
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun()))
			{
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA: //��������
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				qDebug() << "HikVideoCapture: fRealDataCallBack error" << PlayM4_GetLastError(nPort);
				break;
			}
		}
		break;
	default: //��������
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
		break;
	}
}

void CALLBACK HikVideoCapture::ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	switch (dwType)
	{
	case EXCEPTION_RECONNECT: //Ԥ��ʱ����
		qDebug("---------IPC: reconnect-------%d\n", time(nullptr));
		break;
	default:
		break;
	}
}