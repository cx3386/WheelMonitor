#pragma once
#include <QObject>
#include <QMutex>
#include <opencv2/opencv.hpp>

class ImageProcess : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcess(QObject *parent = nullptr);
	~ImageProcess();

	const double angle2Speed = 60 * (M_PI * 0.650 / 360) / (8.0 / 25.0); //周长,只初始化一次，如果改变了interval则失效
	static double angleLowThreshold;
	static double angleHighThreshold;		//角度阈值，超出则报警
	static int radius_min; //轮子的半径的下阈值，用于霍夫圆识别
	static int radius_max;	//轮子的半径的上阈值，用于霍夫圆识别
	static int gs1;//越大越快，准确率越低
					   //霍夫圆检测时高斯滤波内核大小，只能为奇数，越大越模糊，圆与背景区分不明显时，减小高斯值
	static double dp;//圆心累加器图像分辨率与输入图像之比的倒数，值越小，代表累加器分辨率越高，识别圆会更加慢
	static double minDist;//霍夫变换检测到的圆的圆心之间的最小距离，值越大，某些圆会检测不出来，值越小，多个相邻的圆会被错误的检测成一个重合的圆
	static double param1;//传递给canny边缘检测算子的高阈值，且低阈值为高阈值的一半
	static double param2;//50~80大了识别不到，小了找到很多圆。越大越快//检测阶段圆心累加器的阈值，值越大，检测到的圆越完美，值越小，可以检测到更多根本不存在的圆
	//double s = 1.5;//特征点匹配距离与最小距离的比,如果一直不显示，可适当增大该值，减小gs2
	//int gs2;//特征匹配没有高斯处理//特征匹配时高斯滤波内核大小，只能为奇数，越大越模糊，增大高斯值，可使特征匹配时集中在主要特征处
	
	static bool sensorTriggered;
	cv::Mat imageMatches;
	

private:
	//int iImgNoCycle;
	int coreImageProcess();	//0-no cycle, 1-matches success
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

signals:
	//void resultReady(const QString &result);
	void imageProcessReady();
	void showImageMatches();
	void setAlarmLight(const char* lightcolor);
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
