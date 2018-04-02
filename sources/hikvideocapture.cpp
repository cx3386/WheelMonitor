#include "stdafx.h"
#include "hikvideocapture.h"
#include "confighelper.h"
#include "camprofile.h"
#include "common.h" //capsavedir

using namespace std;
using namespace std::placeholders;
using namespace cv;

QString HikVideoCapture::videoRelativeFilePath = ""; //static, to save videoname to database;
cv::Mat HikVideoCapture::pRawImage;					 //Because the DecBFun(HIKVISION SDK) must be static/global, so the member in it must be static.
int HikVideoCapture::capInterval = 7;
volatile int HikVideoCapture::gbHandling = HikVideoCapture::capInterval;
volatile bool HikVideoCapture::bIsProcessing = false;
LONG HikVideoCapture::nPort = -1;
QMutex HikVideoCapture::mutex;
HikVideoCapture *HikVideoCapture::pVideoCapture = new HikVideoCapture; //need init

HikVideoCapture::HikVideoCapture(const ConfigHelper *confHelper, int _deviceIndex, HWND h, QObject *parent)
	: QObject(parent)
	, configHelper(confHelper)
	, deviceIndex(_deviceIndex)
	, hPlayWnd(h)
	, camProfile(&(confHelper->dev[deviceIndex].camProf))
{
	timer = new QTimer;
	connect(timer, &QTimer::timeout, this, static_cast<void (HikVideoCapture::*)()>(&HikVideoCapture::stopRecord));
	timer->setSingleShot(true);
}

HikVideoCapture::~HikVideoCapture()
{
	stopRecord(false); //save video anyway when quit
	timer->deleteLater();
}
bool HikVideoCapture::syncCameraTime()
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
		qDebug("NET_DVR_SET_TIME error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return false;
	}

	/*get camera time*/
	//DWORD dwReturned = 0;
	//memset(&struDVRTime, 0, sizeof(NET_DVR_TIME));
	//if (!NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_TIMECFG, 0xFFFFFFFF, &struDVRTime, sizeof(NET_DVR_TIME),
	//	&dwReturned))
	//{
	//	qDebug("NET_DVR_GET_TIME error, %d", NET_DVR_GetLastError());
	//	NET_DVR_Logout(lUserID);
	//	NET_DVR_Cleanup();
	//	emit isStartCap(false);
	//	return false;
	//}
	//qDebug() << struDVRTime.dwYear << struDVRTime.dwMonth << struDVRTime.dwDay << struDVRTime.dwHour << struDVRTime.dwMinute << struDVRTime.dwSecond;
	return true;
}

bool HikVideoCapture::startCapture()
{
	//initial
	NET_DVR_Init();
	//set connect/reconnect time
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	//LONG lUserID;	//moved to class definition

	//---------------------------------------
	//register device
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	lUserID = NET_DVR_Login_V30("192.168.2.84", 8000, "admin", "www.cx3386.com", &struDeviceInfo);
	if (lUserID < 0)
	{
		qDebug("Login error, %d", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		emit isStartCap(false);
		return false;
	}

	//设置异常消息回调函数
	NET_DVR_SetExceptionCallBack_V30(0, nullptr, g_ExceptionCallBack, nullptr);

	//sycn device time to system time
	if (!syncCameraTime())
	{
		emit isStartCap(false);
		return false;
	}

	if (h == nullptr)
	{
		qDebug() << "the handle of hik realplay is null!";
	}

	NET_DVR_PREVIEWINFO struPlayInfo_SD = { 0 };
	struPlayInfo_SD.hPlayWnd = h;	 //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo_SD.lChannel = 1;	 //预览通道号
	struPlayInfo_SD.dwStreamType = 1; //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo_SD.dwLinkMode = 0;   //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	struPlayInfo_SD.bBlocked = 1;	 //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

	lRealPlayHandle_SD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_SD, nullptr, nullptr); //SD: stand-definition, used to realplay and save

	//draw to the realplay HDC
	//NET_DVR_RigisterDrawFun(lRealPlayHandle_SD, fDrawFun, lUserID);

	NET_DVR_PREVIEWINFO struPlayInfo_HD = { 0 };
	struPlayInfo_HD.hPlayWnd = nullptr; //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo_HD.lChannel = 1;		//预览通道号
	struPlayInfo_HD.dwStreamType = 0;   //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo_HD.dwLinkMode = 0;		//0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	struPlayInfo_HD.bBlocked = 1;		//0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

	lRealPlayHandle_HD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_HD, nullptr, nullptr); //HD: high-definition, used to imgprocess

	if (!NET_DVR_SetRealDataCallBack(lRealPlayHandle_HD, fRealDataCallBack, lUserID))
	{
		qDebug("NET_DVR_SetRealDataCallBack error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		emit isStartCap(false);
		return false;
	}
	if ((lRealPlayHandle_SD < 0) || (lRealPlayHandle_HD < 0))
	{
		qDebug("NET_DVR_RealPlay_V40 error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		emit isStartCap(false);
		return false;
	}
	emit isStartCap(true);
	return true;
}
bool HikVideoCapture::stopCapture()
{
	//---------------------------------------
	//关闭预览
	if ((!NET_DVR_StopRealPlay(lRealPlayHandle_SD)) || (!NET_DVR_StopRealPlay(lRealPlayHandle_HD)))
	{
		emit isStopCap(false);
		return false;
	}
	//注销用户
	if (!NET_DVR_Logout(lUserID))
	{
		emit isStopCap(false);
		return false;
	}
	//释放SDK资源
	if (!NET_DVR_Cleanup())
	{
		emit isStopCap(false);
		return false;
	}
	emit isStopCap(true);
	return true;
}

void HikVideoCapture::startRecord()
{
	if (bIsSaving) return;
	QString nowDate = QDateTime::currentDateTime().toString("yyyyMMdd");
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	mutex.lock();
	videoRelativeFilePath = QStringLiteral("%1/%2.mp4").arg(nowDate).arg(nowTime);
	mutex.unlock();
	QString videoFilePath = QStringLiteral("%1/%2").arg(videoDirPath).arg(videoRelativeFilePath);
	QByteArray ba = videoFilePath.toLatin1();
	if (!NET_DVR_SaveRealData(lRealPlayHandle_SD, ba.data()))
	{
		qDebug() << "HikVideoCapture: startRecord error";
		return;
	}
	timer->start(MAX_RECORD_MSEC); //wait 100s	//If the timer is already running, it will be stopped and restarted.
	bIsSaving = true;
	emit recordON();
}

void HikVideoCapture::stopRecord(bool bRecordTimeout)
{
	if (!bIsSaving) return;
	if (!NET_DVR_StopSaveRealData(lRealPlayHandle_SD))
	{
		qDebug() << "HikVideoCapture: stopRecord error";
		return;
	}
	if (!bRecordTimeout)
	{
		timer->stop();//stop the timeout timer
	}
	else
	{
		emit recordTimeout();
	}
	bIsSaving = false;
	emit recordOFF();
}

void HikVideoCapture::stopRecord()
{
	stopRecord(false);
}

void HikVideoCapture::currentImageProcessReady()
{
	//如果图像处理完成，则允许录制下一帧；否则阻塞
	bIsProcessing = false;
}

void CALLBACK HikVideoCapture::DecCBFun(long nPort, char *pBuf, long nSize, FRAME_INFO *pFrameInfo, long nReserved1, long nReserved2)
{
	Q_UNUSED(nReserved1);
	Q_UNUSED(nReserved2);
	Q_UNUSED(nSize);
	Q_UNUSED(nPort);

	if (gbHandling)
	{
		gbHandling--;
		return;
	}
	if (bIsProcessing)
	{
		qDebug("Image is waiting for processing! Please increase gbhanding");
		return;
	}
	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_YV12)
	{
		cv::Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1); //pImg = Gray

		cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
		cvtColor(src, pImg, CV_YUV2GRAY_YV12);
		//cv::imshow("callback", pImg);
		//cv::waitKey(1);
		mutex.lock();
		pRawImage = pImg;
		mutex.unlock();
		bIsProcessing = true;					//bool need no mutex, because bool is guaranteed atomic operations
		emit pVideoCapture->captureOneImage(); //正向通知,反向阻塞(bStop)
	}
	gbHandling = capInterval; // every 8 frame
}

//实时流回调
void CALLBACK HikVideoCapture::fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, DWORD dwUser) //void *pUser
{
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //系统头

		if (!PlayM4_GetPort(&nPort))
		{ //获取播放库未使用的通道号
			break;
		}
		//m_iPort = lPort; //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
		if (dwBufSize > 0)
		{
			if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME))
			{ //设置实时流播放模式
				break;
			}

			if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 10 * 1024 * 1024))
			{ //打开流接口
				break;
			}

			if (!PlayM4_Play(nPort, nullptr))
			{ //播放开始
				break;
			}
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun))
			{
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA: //码流数据
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				qDebug() << "error" << PlayM4_GetLastError(nPort);
				break;
			}
		}
		break;
	default: //其他数据
		if (dwBufSize > 0 && nPort != -1)
		{
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
		break;
	}
	//Sleep(1);
}

void CALLBACK HikVideoCapture::g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT: //预览时重连
		qDebug("----------reconnect--------%d\n", time(nullptr));
		break;
	default:
		break;
	}
}