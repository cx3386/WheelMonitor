#include "stdafx.h"
#include "HikVideoCapture.h"
#include "common.h"	//capsavedir
using namespace std;

//static member init; just like the global variables
//<数据类型><类名>::<静态数据成员名 >= <值>
QString HikVideoCapture::videoRelativeFilePath = "";	//static, to save videoname to database;
cv::Mat HikVideoCapture::pRawImage;	//Because the DecBFun(HIKVISION SDK) must be static/global, so the member in it must be static.
int HikVideoCapture::capInterval = 7;
volatile int HikVideoCapture::gbHandling = HikVideoCapture::capInterval;
volatile bool HikVideoCapture::bIsProcessing = false;
LONG HikVideoCapture::nPort = -1;
QMutex HikVideoCapture::mutex;
HikVideoCapture* HikVideoCapture::pVideoCapture = new HikVideoCapture; //need init

HikVideoCapture::HikVideoCapture(QObject* parent)
	: QObject(parent)
	, bIsSaving(false)
	, bSaveTimeout(false)
	, timer(Q_NULLPTR)
{
	//gbHandling = 3;	//only init once, if two instace/object exist, the gbhanding is unprotected.
}

HikVideoCapture::~HikVideoCapture()
{
	stopSave();	//save video anyway when quit
	if (timer != Q_NULLPTR)
	{
		timer->deleteLater();
	}	//will delete automatically
	//delete timer;
	//timer = nullptr;
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

bool HikVideoCapture::startCap(HWND h)
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
	if (lUserID < 0) {
		qDebug("Login error, %d", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		emit isStartCap(false);
		return false;
	}

	//设置异常消息回调函数
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

	//sycn device time to system time
	if (!syncCameraTime())
	{
		emit isStartCap(false);
		return false;
	}

	if (h == 0) {
		qDebug() << "the handle of hik realplay is null!";
	}

	NET_DVR_PREVIEWINFO struPlayInfo_SD = { 0 };
	struPlayInfo_SD.hPlayWnd = h; //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo_SD.lChannel = 1; //预览通道号
	struPlayInfo_SD.dwStreamType = 1; //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo_SD.dwLinkMode = 0; //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	struPlayInfo_SD.bBlocked = 1; //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

	lRealPlayHandle_SD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_SD, NULL, NULL);	//SD: stand-definition, used to realplay and save

	//draw to the realplay HDC
	//NET_DVR_RigisterDrawFun(lRealPlayHandle_SD, fDrawFun, lUserID);

	NET_DVR_PREVIEWINFO struPlayInfo_HS = { 0 };
	struPlayInfo_HS.hPlayWnd = nullptr; //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo_HS.lChannel = 1; //预览通道号
	struPlayInfo_HS.dwStreamType = 0; //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo_HS.dwLinkMode = 0; //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	struPlayInfo_HS.bBlocked = 1; //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

	lRealPlayHandle_HD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_HS, NULL, NULL);	//HD: high-definition, used to imgprocess

	if (!NET_DVR_SetRealDataCallBack(lRealPlayHandle_HD, fRealDataCallBack, lUserID)) {
		qDebug("NET_DVR_SetRealDataCallBack error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		emit isStartCap(false);
		return false;
	}
	if ((lRealPlayHandle_SD < 0) || (lRealPlayHandle_HD < 0)) {
		qDebug("NET_DVR_RealPlay_V40 error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		emit isStartCap(false);
		return false;
	}
	emit isStartCap(true);
	return true;
}
bool HikVideoCapture::stopCap()
{
	//---------------------------------------
	//关闭预览
	if ((!NET_DVR_StopRealPlay(lRealPlayHandle_SD)) || (!NET_DVR_StopRealPlay(lRealPlayHandle_HD))) {
		emit isStopCap(false);
		return false;
	}
	//注销用户
	if (!NET_DVR_Logout(lUserID)) {
		emit isStopCap(false);
		return false;
	}
	//释放SDK资源
	if (!NET_DVR_Cleanup()) {
		emit isStopCap(false);
		return false;
	}
	emit isStopCap(true);
	return true;
}

bool HikVideoCapture::startSave()
{
	static bool hasTimer = false;
	if (!hasTimer)
	{//timer won't be delete until ~HikVideoCapture()
		timer = new QTimer;
		connect(timer, &QTimer::timeout, [&] {bSaveTimeout = true; stopSave(); });
		timer->setSingleShot(true);	//timer fires only once
		hasTimer = true;
	}
	if (bIsSaving)
		return false;
	bIsSaving = true;
	qDebug() << "save start";
	QString videoFilePath;
	QString nowDate = QDateTime::currentDateTime().toString("yyyyMMdd"); //ddd is weekday
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss"); //ddd is weekday
	mutex.lock();
	videoRelativeFilePath = QStringLiteral("%1/%2.mp4").arg(nowDate).arg(nowTime);
	mutex.unlock();
	videoFilePath = QStringLiteral("%1/%2").arg(videoDirPath).arg(videoRelativeFilePath);
	QByteArray ba = videoFilePath.toLatin1();	//QString to char * 官方转换方法//不可以str.toLatin1().data()这样一步完成，可能会出错。//只支持Latin，不支持中文，
	if (NET_DVR_SaveRealData(lRealPlayHandle_SD, ba.data())) {
		//QTimer::singleShot(100000, this, SLOT(timeoutSave())); //wait 100s. the singleshot cannot be stopped.
		timer->start(100000);	//wait 100s	//If the timer is already running, it will be stopped and restarted.
			//emit isStartSave(true);
		return true;
	}
	else {
		qDebug() << "cap save start error";
		//emit isStartSave(false);
		bIsSaving = false;
		return false;
	}
}
bool HikVideoCapture::stopSave()
{
	if (bIsSaving)
	{
		if (NET_DVR_StopSaveRealData(lRealPlayHandle_SD))
		{
			bIsSaving = false;
			emit saveStopped();
			if (!bSaveTimeout)
			{//stop the timeout timer
				qDebug("save complete");
				timer->stop();
			}
			else
			{//savetimeout
				qDebug("save timeout, cart is stopped");
				emit wheelTimeout();
				bSaveTimeout = false;	//reset the savetimeout
			}
			//if video stopped by sensor correctly, stop the timeout timer;
			return true;
		}
	}
	return false;
}

void HikVideoCapture::imageProcessReady()
{
	//如果图像处理完成，则允许录制下一帧；否则阻塞
	bIsProcessing = false;
}

void CALLBACK HikVideoCapture::DecCBFun(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, long nReserved1, long nReserved2)
{
	if (gbHandling) {
		gbHandling--;
		return;
	}
	if (bIsProcessing) {
		qDebug("Image is waiting for processing! Please increase gbhanding");
		return;
	}
	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_YV12) {
		cv::Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1); //pImg = Gray

		cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
		cvtColor(src, pImg, CV_YUV2GRAY_YV12);
		//cv::imshow("callback", pImg);
		//cv::waitKey(1);
		mutex.lock();
		pRawImage = pImg;
		mutex.unlock();
		bIsProcessing = true; //bool need no mutex, because bool is guaranteed atomic operations
		emit pVideoCapture->imageNeedProcess(); //正向通知,反向阻塞(bStop)
	}
	gbHandling = capInterval; // every 8 frame
}

void CALLBACK HikVideoCapture::fDrawFun(LONG lRealHandle, HDC hDc, DWORD dwUser)
{
	HDC hdcSrc = CreateCompatibleDC(hDc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDc, 500, 600);
	SelectObject(hdcSrc, hBitmap);
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hDc, hPen);
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
	Rectangle(hdcSrc, 100, 100, 100, 200);
	BitBlt(hDc, 300, 500, 100, 250, hdcSrc, 100, 100, SRCCOPY);
	//SelectObject(hDc, &hOldBitmap);
}

//实时流回调
void CALLBACK HikVideoCapture::fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize, DWORD dwUser) //void *pUser
{
	switch (dwDataType) {
	case NET_DVR_SYSHEAD: //系统头

		if (!PlayM4_GetPort(&nPort)) { //获取播放库未使用的通道号
			break;
		}
		//m_iPort = lPort; //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
		if (dwBufSize > 0) {
			if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME)) { //设置实时流播放模式
				break;
			}

			if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 10 * 1024 * 1024)) { //打开流接口
				break;
			}

			if (!PlayM4_Play(nPort, NULL)) { //播放开始
				break;
			}
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun)) {
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA: //码流数据
		if (dwBufSize > 0 && nPort != -1) {
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize)) {
				qDebug() << "error" << PlayM4_GetLastError(nPort);
				break;
			}
		}
		break;
	default: //其他数据
		if (dwBufSize > 0 && nPort != -1) {
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize)) {
				break;
			}
		}
		break;
	}
	//Sleep(1);
}

void CALLBACK HikVideoCapture::g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void* pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType) {
	case EXCEPTION_RECONNECT: //预览时重连
		qDebug("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}