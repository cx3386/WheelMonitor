#include "stdafx.h"
#include "HikVideoCapture.h"

using namespace std;

//static member init; just like the global variables
//<数据类型><类名>::<静态数据成员名 >= <值>
int HikVideoCapture::capInterval = 7;
volatile int HikVideoCapture::gbHandling = HikVideoCapture::capInterval;
volatile bool HikVideoCapture::bIsProcessing = false;
LONG HikVideoCapture::nPort = -1;
cv::Mat HikVideoCapture::pRawImage;
QMutex HikVideoCapture::mutex;
HikVideoCapture* HikVideoCapture::pVideoCapture = new HikVideoCapture; //need init

HikVideoCapture::HikVideoCapture(QObject* parent)
	: QObject(parent)
	, bIsSaving(false)
{
	//gbHandling = 3;	//only init once, if two instace/object exist, the gbhanding is unprotected.
}

HikVideoCapture::~HikVideoCapture()
{
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
	// 初始化
	NET_DVR_Init();
	//set connect/reconnect time
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	//LONG lUserID;

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

	NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
	struPlayInfo.hPlayWnd = h; //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo.lChannel = 1; //预览通道号
	struPlayInfo.dwStreamType = 1; //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo.dwLinkMode = 0; //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	struPlayInfo.bBlocked = 1; //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

	lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);

	NET_DVR_PREVIEWINFO struPlayInfo_HS = { 0 };
	struPlayInfo_HS.hPlayWnd = nullptr; //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo_HS.lChannel = 1; //预览通道号
	struPlayInfo_HS.dwStreamType = 0; //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo_HS.dwLinkMode = 0; //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	struPlayInfo_HS.bBlocked = 1; //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

	lRealPlayHandle_HS = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_HS, NULL, NULL);

	if (!NET_DVR_SetRealDataCallBack(lRealPlayHandle_HS, fRealDataCallBack, lUserID)) {
		qDebug("NET_DVR_SetRealDataCallBack error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		emit isStartCap(false);
		return false;
	}
	if ((lRealPlayHandle < 0) || (lRealPlayHandle_HS < 0)) {
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
	if ((!NET_DVR_StopRealPlay(lRealPlayHandle)) || (!NET_DVR_StopRealPlay(lRealPlayHandle_HS))) {
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
	if (bIsSaving)
		return false;
	bIsSaving = true;
	qDebug() << "save start";
	QString saveDir;
	QString nowDate = QDateTime::currentDateTime().toString("yyyyMMdd"); //ddd is weekday
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss"); //ddd is weekday
	saveDir = QStringLiteral("D:/Capture/%1/%2.mp4").arg(nowDate).arg(nowTime);
	mutex.lock();
	//saveDirLink = QStringLiteral("file:///D:/Capture/%1.mp4").arg(dateTime);
	saveDirLink = saveDir;
	mutex.unlock();
	QByteArray ba = saveDir.toLatin1();
	if (NET_DVR_SaveRealData(lRealPlayHandle, ba.data())) {
		QTimer::singleShot(100000, this, SLOT(timeoutSave())); //wait 100s
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
	if (bIsSaving) {
		if (NET_DVR_StopSaveRealData(lRealPlayHandle)) {
			qDebug("save complete");
			bIsSaving = false;
			return true;
		}
		return false;
	}
	return false;
}
bool HikVideoCapture::timeoutSave()
{
	if (bIsSaving) {
		if (NET_DVR_StopSaveRealData(lRealPlayHandle)) {
			qDebug("save timeout, the wheel is stopped");
			bIsSaving = false;
			emit wheelTimeout();
			return true;
		}
		return false;
	}
	return false;
}

void HikVideoCapture::imageProcessReady()
{
	//mutex.lock();
	bIsProcessing = false;
	//如果图像处理完成，则允许录制下一帧；否则阻塞
	//mutex.unlock();
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
		//mutex.lock();
		bIsProcessing = true; //不用锁，因为是automic（原子）操作
		//mutex.unlock();
		emit pVideoCapture->imageNeedProcess(); //单向通知,单向阻塞
	}
	gbHandling = capInterval; // every 8 frame
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