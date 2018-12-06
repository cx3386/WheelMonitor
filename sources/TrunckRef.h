#pragma once

#include <QObject>

class Plc;
class TrunckRef : public QObject {
    Q_OBJECT

public:
    TrunckRef(Plc* plcSerial, QObject* parent = Q_NULLPTR);
    ~TrunckRef();
    /*跨线程调用*/
    void start();
    void stop();

private:
    Plc* plc;
    bool bUsrCtrl = false;
    //QTimer* readSpeedTimer; //!< 定时读取台车速度
    static const int readSpeedInterval = 250; // ms
    const double dist_min = 2.3; // unit:m
    const double dist_max = 2.4; // m
    struct Ckpt {
        bool bStart = false; //!< 由第一个经过检测点的tri信号开始计数，一旦usrstop,重新等待第一个tri
        double s = 0; //!< 本周期已经走过的行程
        double anchor = 0; //!< tri的地点距离s起点的位置
        void newWheel()
        {
            s = s - anchor;
            anchor = 0;
        } //!< 开始一个新的周期
        //void init() {} //!< 系统从连续状态脱出并重新进入时，初始化结构
    } ckps[4]; //col,cor,cil,cir

    void speedIntegrator();

public slots:
    void onCkpTri(int ckpId);

signals:
    void expectTriBegin(int ckpId);
    void expectTriEnd(int ckpId);
};
