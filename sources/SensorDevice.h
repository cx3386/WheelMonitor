#pragma once

#include "LevelRecorder.h"

/*һ�������ڵļ����ṹ*/
class SenSor;
class CkPt;
class SensorDevice;
class Sensor {
public:
    Sensor(CkPt* parent, int _id);
    int id; //���λ: 0-0;1-1
    int m_id;
    static int indexOf(QString _name) { return names.indexOf(_name); }
    static const QStringList names; //!< ÿ������Ĵ�����
    static QList<Sensor> createAll(CkPt* parent);
    void newWheel();
    void init();
    CkPt* parentCkPt;
    QString name;
    int nTri; // �����������Ĵ���,һ�������ڣ�ֻӦ����һ��
    bool expected = true; //!< ��¼�������Ƿ񻵵�
    LevelRecorder sample; //!< ��¼�������Ĳ�����Ϣ
    int lastbroken; //!< �ϸ������ڣ��Ƿ񻵵�0-no;1-may;2-yes
    int broken; //!< ��¼�������Ƿ񻵵������ڽ���ʱ���£�
private:
};

class CkPt {
public:
    CkPt(SensorDevice* parent, int _id);
    int id; //���λ:0-l;1-r
    int m_id;
    SensorDevice* parentDev;
    QString name;
    //LevelRecorder expectIn; //!< ����̨���ٶȻ��֣�Ԥ���Ƿ񵽴����
    static int indexOf(QString _name) { return names.indexOf(_name); }
    static const QStringList names; //!< ÿ������Ĵ�����
    static QList<CkPt> creatAll(SensorDevice* parent); //!< һ���Դ������еļ���
    QList<Sensor> sensors;
    void newWheel();
    void init();
    //col������ 0, cor�Ҳ��뿪 1, cir�Ҳ���� 2, cil����뿪 3
    //! ���ش�����ʱ��������ǽ��룬���½��ش���
    int triTiming() { return isEnter() ? LevelRecorder::NegativeEdge : LevelRecorder::PositiveEdge; }
    //! �����Ƿ��ɴ˼������
    bool isEnter();
    //! �����Ƿ��ɴ˼����뿪
    bool isLeave() { return !isEnter(); }

private:
};

class SensorDevice {
public:
    SensorDevice(int _id);
    int id;
    int m_id; //0-o;1-i
    QString name;
    int alarm; //0-����,1-���´�ѭ���ж�,2-����
    int lastalarm;
    static int indexOf(QString _name) { return names.indexOf(_name); }
    static const QStringList names;
    static QList<SensorDevice> createAll();
    QList<CkPt> ckpts;

    void newWheel();
    void init();
    ~SensorDevice();
};
