#pragma once

#include "LevelRecorder.h"
#include <QObject>

/*一个周期内的计数结构*/
class SenSor;
class CkPt;
class SensorDevice;
class Sensor : public QObject {
	Q_OBJECT
public:
	Sensor(int _id, CkPt* parent);
	int id; //最低位: 0-0;1-1
	int m_id; //id的最低位
	static int indexOf(QString _name) { return names.indexOf(_name); }
	static const QStringList names; //!< 每个检测点的传感器
	static QList<Sensor*> createAll(CkPt* parent);
	void newWheel();
	void init();
	CkPt* parentCkPt;
	QString name;
	int nTri; // 传感器触发的次数,一个周期内，只应出现一次
	bool expected = true; //!< 记录传感器是否坏掉
	LevelRecorder sample; //!< 记录传感器的采样信息
	int lastbroken = 2; //!< 上个周期内，是否坏掉0-no;1-may;2-yes,初始化为坏掉，即开机自检后显示为正常
	int broken; //!< 记录传感器是否坏掉（周期结束时更新）
private:
};

class CkPt : public QObject {
	Q_OBJECT
public:
	CkPt(int _id, SensorDevice* parent);
	int id; //最低位:0-l;1-r //即00外左进 01外右出 10内左出 11内右进
	int m_id;
	SensorDevice* parentDev;
	QString name;
	//LevelRecorder expectIn; //!< 根据台车速度积分，预测是否到达检测点
	static int indexOf(QString _name) { return names.indexOf(_name); }
	static const QStringList names; //!< 每个检测点的传感器
	static QList<CkPt*> creatAll(SensorDevice* parent); //!< 一次性创建所有的检测点
	QList<Sensor*> sensors;
	void newWheel();
	void init();
	//! 返回触发的时机，如果是进入，则下降沿触发
	int triTiming() { return isEnter() ? LevelRecorder::NegativeEdge : LevelRecorder::PositiveEdge; }
	//! 返回是否由此检测点进入
	bool isEnter();
	//! 返回是否由此检测点离开
	bool isLeave() { return !isEnter(); }

private:
};

class SensorDevice : public QObject {
	Q_OBJECT
public:
	SensorDevice(int _id, QObject* parent);
	int id;
	int m_id; //0-o;1-i
	QString name;
	int alarm; //0-正常,1-由下次循环判断,2-掉轮
	int lastalarm; //!< 只有一个传感器是好的，这个好的传感器又没亮，则无法判断是设备故障还是掉轮故障。交由下一个循环判断
	static int indexOf(QString _name) { return names.indexOf(_name); }
	static const QStringList names;
	static QList<SensorDevice*> createAll(QObject* parent);
	QList<CkPt*> ckpts;

	void newWheel();
	void init();
	~SensorDevice();
};
