#pragma once

#include <QObject>

class Plc;
class TrunckRef : public QObject {
    Q_OBJECT

public:
    TrunckRef(Plc* plcSerial, QObject* parent = Q_NULLPTR);
    ~TrunckRef();
    /*���̵߳���*/
    void start();
    void stop();

private:
    Plc* plc;
    bool bUsrCtrl = false;
    //QTimer* readSpeedTimer; //!< ��ʱ��ȡ̨���ٶ�
    static const int readSpeedInterval = 250; // ms
    const double dist_min = 2.3; // unit:m
    const double dist_max = 2.4; // m
    struct Ckpt {
        bool bStart = false; //!< �ɵ�һ�����������tri�źſ�ʼ������һ��usrstop,���µȴ���һ��tri
        double s = 0; //!< �������Ѿ��߹����г�
        double anchor = 0; //!< tri�ĵص����s����λ��
        void newWheel()
        {
            s = s - anchor;
            anchor = 0;
        } //!< ��ʼһ���µ�����
        //void init() {} //!< ϵͳ������״̬�ѳ������½���ʱ����ʼ���ṹ
    } ckps[4]; //col,cor,cil,cir

    void speedIntegrator();

public slots:
    void onCkpTri(int ckpId);

signals:
    void expectTriBegin(int ckpId);
    void expectTriEnd(int ckpId);
};
