#pragma once
#include "LevelRecorder.h"
#include "common.h"
#include <QObject>

class ConfigHelper;
class QSerialPort;
class TrunckRef;

//! plc其他硬件设备相关类，包括传感器、plc io、plc ad
class Plc : public QObject {
    Q_OBJECT
public:
    //! 必须传入configure, const成员变量在初始化列表中初始化
    explicit Plc(const ConfigHelper* _configHelper, QObject* parent = nullptr);
    ~Plc();
    // 用于mainwindow跨线程控制。通过信号槽机制，onXxx将在plcThread中执行
    void start(); //!< usrctrl
    void stop(); //!< usrctrl
    void connect(); //!< 通过串口连接到plc
    //! 获取台车速度(m/min)，线程安全的
    double getTruckSpeed(int devId = 0) const
    {
        QMutexLocker locker(&mutex);
        return truckSpeed * speedCompensationCoeff[devId];
    }

private:
    const ConfigHelper* configHelper;
    TrunckRef* trnkRef;
    mutable QMutex mutex;
    QSerialPort* plcSerialPort;
    QThread* refThread;
    double truckSpeed; //linear Velocity read from AD, unit:m/min

    /// the rate between inner calc speed and measured speed, of which value is optimized by LS method (minimize the error)
    const double speedCompensationCoeff[2] = { 0.954, 0.950 }; // [out, in]

    /*一个周期内的计数结构*/
    struct Sensor {
        Sensor(int _id)
            : id()
        {
        }
        int id; //最低位: 0-0;1-1
        int nTri; // 传感器触发的次数,一个周期内，只应出现一次
        bool expected = true; //!< 记录传感器是否坏掉
        LevelRecorder sample; //!< 记录传感器的采样信息
        int lastbroken; //!< 上个周期内，是否坏掉0-no;1-may;2-yes
        int broken; //!< 记录传感器是否坏掉（周期结束时更新）
        void newWheel()
        {
            nTri = 0;
            expected = true;
            broken = 0;
        }
        void init()
        {
            newWheel();
            sample.init(false);
            lastbroken = 0;
        }
    };
    struct CkPt {
        CkPt(int _id)
            : id(id)
        {
        }
        int id; //最低位:0-l;1-r
        LevelRecorder expectIn; //!< 根据台车速度积分，预测是否到达检测点
        Sensor sensor0 { id << 1 | 0 }, sensor1 { id << 1 | 1 };
        void newWheel()
        {
            sensor1.newWheel();
            sensor0.newWheel();
        }
        void init()
        {
            newWheel();
            sensor0.init();
            sensor1.init();
            expectIn.init(false);
        }
    };
    //col左侧进入 0, cor右侧离开 1, cir右侧进入 2, cil左侧离开 3
    struct Device {
        Device(int _id)
            : id(id)
        {
        }
        int id; // 最低位:0-o;1-i
        int alarm; //0-正常,1-由下次循环判断,2-掉轮
        int lastalarm;
        CkPt ckpL { id << 1 | 0 }, ckpR { id << 1 | 1 };
        void newWheel()
        {
            alarm = 0;
            ckpL.newWheel();
            ckpR.newWheel();
        }
        void init()
        {
            newWheel();
            ckpL.init();
            ckpR.init();
            lastalarm = 0;
        }
    } dev0 { 0 }, dev1 { 1 };

    bool bUsrCtrl = false;
    bool bConnected = false;
    /* one read write period >300ms */
    int cio0Interval = 1000; // msec
    int adInterval = 250; //msec; should > 7/25 s

    void writePLC(QByteArray plcData);
    QByteArray readPLC(QByteArray plcData);
    /**
	 * \brief return a FCS code generated by cmd code
	 * e.g., cmd = '@00RR00020001', fcs code = '0x43', the full code = "@00RR0002000143*\r"
	 *
	 * \param QByteArray cmd
	 */
    static QByteArray getFCSCode(QByteArray cmd);
    static inline QByteArray getFullCode(QByteArray cmd)
    {
        return cmd + getFCSCode(cmd) + "*\r";
    }
    /**
	 * \brief generate a RR command code
	 * PC to PLC ask(command) code
	 * "@00RR0001(ADDR)0001(No. of words)FCS*\r"
	 * except data and end code is hex, the others is BCD.
	 *
	 * \param int addr (cio) register address of beginning word to read
	 * \param int num how many words to read
	 */
    static QByteArray genRRCode(int addr, int num);
    /**
	* \brief generate a WR command code
	* PC to PLC ask(command) code
	* "@00WR0100(ADDR)data(words,hex)FCS*\r"
	* except data and end code is hex, the others is BCD.
	*
	* \param int addr (cio) register address of beginning word to write
	* \param QByteArray data how many words to read or the data to write
	*/
    static QByteArray genWRCode(int addr, QByteArray data);

    /**
	 * \brief check answer code if it's right.
	 * PLC to PC answer(response) code:
	 * "@00RR00(endcode,hex)data(words,hex)FCS*\r"
	 * "@00WR00(endcode,hex)FCS*\r"
	 * except data and end code is hex, the others is BCD.
	 *
	 * \param QByteArray
	 */
    static bool checkAnsCode(QByteArray);
    /**
	 * \brief get RR data from response code
	 *
	 * \param QByteArray
	 */
    static QByteArrayList getRRData(QByteArray);

    void recordCio0(WORD cio0); //!< cio0发送到界面进行处理

    void checkTri();

    void checkExpect();

    void checkBrokenAlarm(int devId);

    void handleCkpTri(int ckpId)
    {
        if (ckpId == 0 || ckpId == 3) {
            emit _DZIn(ckpId >> 1);
        } else if (ckpId == 1 || ckpId == 4) {
            emit _DZOut(ckpId >> 1);
        }
        emit ckpTri(ckpId);
    }

private slots:
    void readSensorPeriodically();
    //! read truck reference speed from AD module. a cycle includes sending cmd and recv response, costs ~300ms
    void readSpeedPeriodically();

signals:
    void cio0Changed(WORD);
    void _DZIn(int id); //!< 车轮进入DZ, id为deviceIndex
    void _DZOut(int id); //!< 车轮离开DZ, id为deviceIndex
    void ckpTri(int ckpId);
    void connectError(int errorId);
    void truckSpeedReady();
    /**
	 * \brief 台车(中轴)速度信号读取错误
	 * errcode 1: disconnect
	 * errcode 2: out of range(4~20ma)
	 * \param int errorCode
	 */
    void truckSpeedError(int);

public slots:
    void onAlarmEvent(int);
};