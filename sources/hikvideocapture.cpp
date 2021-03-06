#include "stdafx.h"

#include "common.h" //capsavedir
#include "confighelper.h"
#include "hikvideocapture.h"

using namespace std;
using namespace std::placeholders;
using namespace cv;

HikVideoCapture *HikVideoCapture::pCap0, *HikVideoCapture::pCap1;

HikVideoCapture::HikVideoCapture(const ConfigHelper* _configHelper, int _deviceIndex, HWND h, QObject* parent)
	: QObject(parent)
	, configHelper(_configHelper)
	, deviceIndex(_deviceIndex)
	, hPlayWnd(h)
	, camProfile(&(configHelper->device[deviceIndex].camProfile))
{
	//构造时连接到摄像头，此后不再中断
	struPlayInfo_HD = {
		1, // LONG lChannel;//通道号
		0, // DWORD dwStreamType;    // 码流类型，0-主码流，1-子码流，2-码流3，3-码流4 等以此类推
		0, // DWORD dwLinkMode;// 0：TCP方式,1：UDP方式,2：多播方式,3 - RTP方式，4-RTP/RTSP,5-RSTP/HTTP
		hPlayWnd, //播放窗口的句柄,为NULL表示不播放图象
		1
	}; // DWORD bBlocked;  //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.
	struPlayInfo_SD = {
		1, // lChannel
		1, // dwStreamType //子码流为SD
		0, // dwLinkMode
		0, // hPlayWnd
		0
	}; // bBlocked //不需要回调，可以非阻塞
	NET_DVR_Init();
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	//参见 QByteArray::data()
	auto ba_ip = camProfile->camIP.toLocal8Bit();
	auto ba_user = camProfile->camUserName.toLocal8Bit();
	auto ba_pwd = camProfile->camPassword.toLocal8Bit();
	char* ip = ba_ip.data();
	char* user = ba_user.data();
	char* pwd = ba_pwd.data();
	lUserID = NET_DVR_Login_V30(ip, camProfile->camPort, user, pwd, &struDeviceInfo);
	if (lUserID < 0) {
		qDebug() << "IPC(" << ip << ") Login failed, error code " << NET_DVR_GetLastError();
		NET_DVR_Cleanup();
		return;
	}
	NET_DVR_SetExceptionCallBack_V30(0, nullptr, ExceptionCallBack, nullptr);
	syncCameraTime(); // 同步相机时间
}

HikVideoCapture::~HikVideoCapture()
{
	// 停止录制
	stop();
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
	if (!NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_TIMECFG, 0xFFFFFFFF, &(struDVRTime), sizeof(NET_DVR_TIME))) {
		qDebug("NET_DVR_SET_TIME error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
	}
}

bool HikVideoCapture::start()
{
	if ((lRealPlayHandle_HD != -1) && (lRealPlayHandle_SD != -1)) {
		qDebug() << "HikVideoCapture: Duplicate start";
		return true; // already started
	}
	lRealPlayHandle_HD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_HD, fRealDataCallBack, this);
	lRealPlayHandle_SD = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo_SD, nullptr, nullptr);
	if ((lRealPlayHandle_HD == -1) || (lRealPlayHandle_SD == -1)) {
		qDebug("NET_DVR_RealPlay_V40 error, %d", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return false;
	}
	nHandling = nHandling_Start;
	return true;
}

bool HikVideoCapture::stop()
{
	QTimer::singleShot(0, this, [=] {stopRecord(); }); //从子线程操作
	// 等待，直到录制已经停止
	while (bIsRecording) {
		QCoreApplication::processEvents();
	}
	//stopRecord();
	bool ret = true;
	if (lRealPlayHandle_HD != -1) {
		ret &= NET_DVR_StopRealPlay(lRealPlayHandle_HD);
		lRealPlayHandle_HD = -1;
	}
	if (lRealPlayHandle_SD != -1) {
		ret &= NET_DVR_StopRealPlay(lRealPlayHandle_SD);
		lRealPlayHandle_SD = -1;
	}
	//if handle==-1, already stopped, return true
	return ret;
}

void HikVideoCapture::startRecord()
{
	QMutexLocker locker(&mutex);
	if (bIsRecording)
		return;
	auto date = QDate::currentDate().toString("yyyyMMdd");
	auto dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	auto mark = getDeviceMark(deviceIndex);
	videoRelativeFilePath = QStringLiteral("%1/%2_%3.mp4").arg(date).arg(mark).arg(dateTime);
	QString videoFilePath = QStringLiteral("%1/%2").arg(g_videoDirPath).arg(videoRelativeFilePath);
	QByteArray ba = videoFilePath.toLocal8Bit();
	if (!NET_DVR_SaveRealData(lRealPlayHandle_SD, ba.data())) {
		qDebug() << "HikVideoCapture: startRecord error";
		return;
	}
	// 子线程操作
	timeoutTimer = new QTimer;
	timeoutTimer->setSingleShot(true); // 不直接用QTimer::singleShot，因为一旦开始，无法stop
	timeoutTimer->setInterval(MAX_RECORD_MSEC); // 设置超时时间为100s
	connect(timeoutTimer, &QTimer::timeout, this, [=] {
		stopRecord(); //子线程
		emit recordTimeout();
	});
	timeoutTimer->start(); //If the timer is already running, it will be stopped and restarted.
	bIsRecording = true;
	emit recordON();
}

void HikVideoCapture::stopRecord()
{
	QMutexLocker locker(&mutex);
	if (!bIsRecording)
		return;
	if (!NET_DVR_StopSaveRealData(lRealPlayHandle_SD)) {
		qDebug() << "HikVideoCapture: stopRecord error";
		return;
	}
	timeoutTimer->stop(); // 必须用子线程操作
	//QTimer::singleShot(0, this, [&] {timeoutTimer->stop(); }); // 即使直接调用stoprecord，也由子线程操控timer，保证安全
	bIsRecording = false;
	emit recordOFF();
}

void HikVideoCapture::DecCBFun(char* pBuf, FRAME_INFO* pFrameInfo)
{
	// 每x帧捕获1帧
	if (--nHandling)
		return;
	nHandling = camProfile->frameInterv;

	static int waitcount = 0;
	//检查库存：还没处理完
	if (nPendingFrame > 0) {
		// 防止写入太密集
		if (++waitcount == 500) {
			qDebug("Image is waiting for processing! Please increase gbhanding");
			waitcount = 0;
		}
		return;
	}
	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_YV12) {
		cv::Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1); //pImg = Gray
		cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
		cvtColor(src, pImg, CV_YUV2GRAY_YV12);
		mutex.lock();
		rawImage = pImg;
		nPendingFrame++;
		mutex.unlock();
		emit frameCaptured(); // 正向通知,反向阻塞(bStop)
	}
}

void CALLBACK HikVideoCapture::DecCBFun0(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, long nReserved1, long nReserved2)
{
	pCap0->DecCBFun(pBuf, pFrameInfo);
}

void CALLBACK HikVideoCapture::DecCBFun1(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, long nReserved1, long nReserved2)
{
	pCap1->DecCBFun(pBuf, pFrameInfo);
}

/// 实时流回调
void CALLBACK HikVideoCapture::fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize, void* pUser) //void *pUser
{
	auto* pCap = static_cast<HikVideoCapture*>(pUser);
	LONG& nPort = pCap->nPort;
	auto DecCBFun = [pCap]() -> auto
	{
		int index = pCap->deviceIndex;
		switch (index) {
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

			if (!PlayM4_Play(nPort, nullptr)) { //解码，不播放
				break;
			}
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun())) {
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA: //码流数据
		if (dwBufSize > 0 && nPort != -1) {
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize)) {
				qDebug() << "HikVideoCapture: fRealDataCallBack error" << PlayM4_GetLastError(nPort);
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
}

void CALLBACK HikVideoCapture::ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void* pUser)
{
	switch (dwType) {
	case EXCEPTION_RECONNECT: //预览时重连
		qDebug("---------IPC: reconnect-------%d\n", time(nullptr));
		break;
	default:
		break;
	}
}