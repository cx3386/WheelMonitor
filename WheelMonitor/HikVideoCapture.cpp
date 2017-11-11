#include "stdafx.h"
#include "HikVideoCapture.h"

using namespace std;

//static member init; just like the global variables
//<��������><����>::<��̬���ݳ�Ա�� >= <ֵ>
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
	// ��ʼ��
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
	
	//�����쳣��Ϣ�ص�����
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
	struPlayInfo.hPlayWnd = h; //��ҪSDK����ʱ�����Ϊ��Чֵ����ȡ��������ʱ����Ϊ��
	struPlayInfo.lChannel = 1; //Ԥ��ͨ����
	struPlayInfo.dwStreamType = 1; //0-��������1-��������2-����3��3-����4���Դ�����
	struPlayInfo.dwLinkMode = 0; //0- TCP��ʽ��1- UDP��ʽ��2- �ಥ��ʽ��3- RTP��ʽ��4-RTP/RTSP��5-RSTP/HTTP
	struPlayInfo.bBlocked = 1; //0-������ȡ��, 1-����ȡ��, �������SDK�ڲ�connectʧ�ܽ�����5s�ĳ�ʱ���ܹ�����,���ʺ�����ѯȡ������.

	lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);

	NET_DVR_PREVIEWINFO struPlayInfo_HS = { 0 };
	struPlayInfo_HS.hPlayWnd = nullptr; //��ҪSDK����ʱ�����Ϊ��Чֵ����ȡ��������ʱ����Ϊ��
	struPlayInfo_HS.lChannel = 1; //Ԥ��ͨ����
	struPlayInfo_HS.dwStreamType = 0; //0-��������1-��������2-����3��3-����4���Դ�����
	struPlayInfo_HS.dwLinkMode = 0; //0- TCP��ʽ��1- UDP��ʽ��2- �ಥ��ʽ��3- RTP��ʽ��4-RTP/RTSP��5-RSTP/HTTP
	struPlayInfo_HS.bBlocked = 1; //0-������ȡ��, 1-����ȡ��, �������SDK�ڲ�connectʧ�ܽ�����5s�ĳ�ʱ���ܹ�����,���ʺ�����ѯȡ������.

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
	//�ر�Ԥ��
	if ((!NET_DVR_StopRealPlay(lRealPlayHandle)) || (!NET_DVR_StopRealPlay(lRealPlayHandle_HS))) {
		emit isStopCap(false);
		return false;
	}
	//ע���û�
	if (!NET_DVR_Logout(lUserID)) {
		emit isStopCap(false);
		return false;
	}
	//�ͷ�SDK��Դ
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
	//���ͼ������ɣ�������¼����һ֡����������
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
		bIsProcessing = true; //����������Ϊ��automic��ԭ�ӣ�����
		//mutex.unlock();
		emit pVideoCapture->imageNeedProcess(); //����֪ͨ,��������
	}
	gbHandling = capInterval; // every 8 frame
}

//ʵʱ���ص�
void CALLBACK HikVideoCapture::fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize, DWORD dwUser) //void *pUser
{
	switch (dwDataType) {
	case NET_DVR_SYSHEAD: //ϵͳͷ

		if (!PlayM4_GetPort(&nPort)) { //��ȡ���ſ�δʹ�õ�ͨ����
			break;
		}
		//m_iPort = lPort; //��һ�λص�����ϵͳͷ������ȡ�Ĳ��ſ�port�Ÿ�ֵ��ȫ��port���´λص�����ʱ��ʹ�ô�port�Ų���
		if (dwBufSize > 0) {
			if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME)) { //����ʵʱ������ģʽ
				break;
			}

			if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 10 * 1024 * 1024)) { //�����ӿ�
				break;
			}

			if (!PlayM4_Play(nPort, NULL)) { //���ſ�ʼ
				break;
			}
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun)) {
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA: //��������
		if (dwBufSize > 0 && nPort != -1) {
			if (!PlayM4_InputData(nPort, pBuffer, dwBufSize)) {
				qDebug() << "error" << PlayM4_GetLastError(nPort);
				break;
			}
		}
		break;
	default: //��������
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
	case EXCEPTION_RECONNECT: //Ԥ��ʱ����
		qDebug("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}