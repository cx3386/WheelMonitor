#pragma once

#include "LevelRecorder.h"

/*一个周期内的计数结构*/
class SenSor;
class CkPt;
class SensorDevice;
class Sensor {
public:
    Sensor(CkPt* parent, int _id);
    int id; //最低位: 0-0;1-1
    int m_id;
    static int indexOf(QString _name) { return names.indexOf(_name); }
    static const QStringList names; //!< 每个检测点的传感器
    static QList<Sensor> createAll(CkPt* parent);
    void newWheel();
    void init();
    CkPt* parentCkPt;
    QString name;
    int nTri; // 传感器触发的次数,一个周期内，只应出现一次
    bool expected = true; //!< 记录传感器是否坏掉
    LevelRecorder sample; //!< 记录传感器的采样信息
    int lastbroken; //!< 上个周期内，是否坏掉0-no;1-may;2-yes
    int broken; //!< 记录传感器是否坏掉（周期结束时更新）
private:
};

class CkPt {
public:
    CkPt(SensorDevice* parent, int _id);
    int id; //最低位:0-l;1-r
    int m_id;
    SensorDevice* parentDev;
    QString name;
    //LevelRecorder expectIn; //!< 根据台车速度积分，预测是否到达检测点
    static int indexOf(QString _name) { return names.indexOf(_name); }
    static const QStringList names; //!< 每个检测点的传感器
    static QList<CkPt> creatAll(SensorDevice* parent); //!< 一次性创建所有的检测点
    QList<Sensor> sensors;
    void newWheel();
    void init();
    //col左侧进入 0, cor右侧离开 1, cir右侧进入 2, cil左侧离开 3
    //! 返回触发的时机，如果是进入，则下降沿触发
    int triTiming() { return isEnter() ? LevelRecorder::NegativeEdge : LevelRecorder::PositiveEdge; }
    //! 返回是否由此检测点进入
    bool isEnter();
    //! 返回是否由此检测点离开
    bool isLeave() { return !isEnter(); }

private:
};

class SensorDevice {
public:
    SensorDevice(int _id);
    int id;
    int m_id; //0-o;1-i
    QString name;
    int alarm; //0-正常,1-由下次循环判断,2-掉轮
    int lastalarm;
    static int indexOf(QString _name) { return names.indexOf(_name); }
    static const QStringList names;
    static QList<SensorDevice> createAll();
    QList<CkPt> ckpts;

    void newWheel();
    void init();
    ~SensorDevice();
};
