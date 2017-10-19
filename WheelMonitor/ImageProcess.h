#pragma once
#include <QMutex>
#include <QObject>
#include <opencv2/opencv.hpp>
#include "RobustMatcher.h"
#include "PLCSerial.h"

class ImageProcess : public QObject {
	Q_OBJECT
public:
	explicit ImageProcess(QObject* parent = nullptr);
	~ImageProcess();

	static double angle2Speed; //�ܳ�,ֻ��ʼ��һ�Σ�����ı���interval��ʧЧ
	static double angleLowThreshold;
	static double angleHighThreshold; //�Ƕ���ֵ�������򱨾�
	static int radius_min; //���ӵİ뾶������ֵ�����ڻ���Բʶ��
	static int radius_max; //���ӵİ뾶������ֵ�����ڻ���Բʶ��
	static int gs1; //Խ��Խ�죬׼ȷ��Խ��
	//����Բ���ʱ��˹�˲��ں˴�С��ֻ��Ϊ������Խ��Խģ����Բ�뱳�����ֲ�����ʱ����С��˹ֵ
	static double dp; //Բ���ۼ���ͼ��ֱ���������ͼ��֮�ȵĵ�����ֵԽС�������ۼ����ֱ���Խ�ߣ�ʶ��Բ�������
	static double minDist; //����任��⵽��Բ��Բ��֮�����С���룬ֵԽ��ĳЩԲ���ⲻ������ֵԽС��������ڵ�Բ�ᱻ����ļ���һ���غϵ�Բ
	static double param1; //���ݸ�canny��Ե������ӵĸ���ֵ���ҵ���ֵΪ����ֵ��һ��
	static double param2; //50~80����ʶ�𲻵���С���ҵ��ܶ�Բ��Խ��Խ��//���׶�Բ���ۼ�������ֵ��ֵԽ�󣬼�⵽��ԲԽ������ֵԽС�����Լ�⵽������������ڵ�Բ
	//double s = 1.5;//������ƥ���������С����ı�,���һֱ����ʾ�����ʵ������ֵ����Сgs2
	//int gs2;//����ƥ��û�и�˹����//����ƥ��ʱ��˹�˲��ں˴�С��ֻ��Ϊ������Խ��Խģ���������˹ֵ����ʹ����ƥ��ʱ��������Ҫ������

	static bool sensorTriggered;
	cv::Mat imageMatches;

private:
	//int iImgNoCycle;
	int coreImageProcess(); //0-no cycle, 1-matches success
	//bool isSameWheel;
	double angleSum;
	int angleCount;
	int iImgCount;
	int nDetectCount;

	void getAvgAngle();
	bool bIsInArea;
	bool bLastOUT;
	bool bWheelStopped;
	bool bStopProcess;
	QMutex mutex;
	RobustMatcher rMatcher;

signals:
	//void resultReady(const QString &result);
	void imageProcessReady();
	void showImageMatches();
	void setAlarmLight(PLCSerial::AlarmColor alarmcolor);
	void speedClcReady(double speed);
	void realSpeedReady(double speed);
	public slots:
	void doImageProcess();
	void startImageProcess();
	void stopImageProcess();
	void sensorIN();
	void sensorOUT();
	void wheelTimeout();
	//{
	//QString result;
	///* ... here is the expensive or blocking operation ... */
	//emit resultReady(result);
};
