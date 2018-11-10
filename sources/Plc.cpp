#include "Plc.h"
#include "TrunckRef.h"
#include "confighelper.h"
#include "stdafx.h"
#include <QSerialPort>

/* CPU: CP1E - N30S1DR - A*/
/* I/O */
// INPUT:  2WORD in total. CIO0.0~0.11     CIO1.0~1.05     m=1
// OUTPUT: 2WORD in total. CIO100.0~100.07 CIO101.0~101.03 n=101
/*Read sensor state command*/
// "@00RRADDRdataFCS*\r",
// ADDR is register address to read(10based), e.g. 0101, 0001.
// data=FFFF(2B.hex?10based?) is how many word to read, e.g. 0001 means read 1 WORD
// FCS see getFCSCode()
//const char READ_SENSOR_STATE[] = "@00RR0000000141*\r";  ///< read CIO0 for 1 WORD
/*Read sensor state response*/
// "@00RR00dataFCS*\r", data=FFFF(2B)

//const char SENSOR_L_OFF_R_OFF[] = "@00RR00000040*\r"; //00
//const char SENSOR_L_OFF_R_ON[] = "@00RR00000242*\r";  //01
//const char SENSOR_L_ON_R_OFF[] = "@00RR00000848*\r";  //10
//const char SENSOR_L_ON_R_ON[] = "@00RR00000A31*\r";   //11

/*(deprecated)Write alarm light command*/
/*the center control alarm is 0001, which is real useful*/
//const char ALARM_LIGHT_ON[] = "@00WR010000F032*\r";		///< all on
//const char ALARM_LIGHT_OFF[] = "@00WR0100000044*\r";	///< all off
//const char ALARM_LIGHT_RED[] = "@00WR0100001144*\r";	///< red and alarm
//const char ALARM_LIGHT_GREEN[] = "@00WR0100002046*\r";  ///< green
//const char ALARM_LIGHT_YELLOW[] = "@00WR0100004040*\r"; ///< yellow

/* AD041 module */
// INPUT:  4WORD in total. CIO2~CIO5 m+1~m+4,
// OUTPUT: 2WORD in total. CIO102~CIO103) n+1,n+2
// STEPS:
// 1. write setup data to output word
//		- use the port
//		- average calculate
//		- select analog input range(00:-10~10v;01:0~10v;10:1~5v/4~20ma;11:0~5v/0~20ma)
//		- recv WR_CORRECT response
// 2. read A/D result from input word
//		- sent "read CIO2~5" command to PLC
//		- recv data(1WORD) of port01~04 from PLC in "@00RR00dataFCS*\r" data=FFFF(2B)
// NOTE: must init before it can start AD transverse and can read data from it.
// once init complete, cannot change until restart CPU
// const char INIT_AD[] = "@00WR0102800A800037*\r"; ///< ADO41 setup data. 800A 8000. use ad analog input port01, no average, 4-20ma
// const char READ_AD[] = "@00RR0002000143*\r"; ///< AD041. read CIO2 for 1 word

//const char WR_CORRECT_RESPONSE[] = "@00WR0045*\r"; ///< write response: cio correct

Plc::Plc(const ConfigHelper* _configHelper, QObject* parent)
    : QObject(parent)
    , configHelper(_configHelper)
    , trnkRef(new TrunckRef(this))
{
    QObject::connect(this, &Plc::ckpTri, trnkRef, &TrunckRef::onCkpTri);
    QObject::connect(trnkRef, &TrunckRef::expectTriBegin, this, [=](int id) { CkPt.expectIn = true; });
    QObject::connect(trnkRef, &TrunckRef::expectTriEnd, this, [=](int id) {});
    TODO;
    refThread = new QThread(this);
    trnkRef->moveToThread(refThread);
    refThread->start();
}

Plc::~Plc()
{
    plcSerialPort->close();
    refThread->quit();
    refThread->wait();
}

void Plc::start()
{
    QTimer::singleShot(0, this, [=]() {
        if (!bConnected)
            return;
        bUsrCtrl = true;
        readSensorPeriodically();
        readSpeedPeriodically(); // sensor置为空,使能第一次.state
        dev0.init();
        dev1.init();
        // 开始truckref的计数
        trnkRef->start();
    });
}

void Plc::stop()
{
    QTimer::singleShot(0, this, [=]() {
        bUsrCtrl = false;
        trnkRef->stop();
        emit cio0Changed(0);
    }); //用timeevent，避免跨线程操作带来的锁的管理。但是需要get的value，必须通过锁管理
}

//! 连接到PLC，初始化AD041。在plc创建并移到子线程后执行，应在退出程序时断开
void Plc::connect()
{
    QTimer::singleShot(0, this, [=]() {
        /*setup serialport parameters*/
        plcSerialPort = new QSerialPort(this);
        plcSerialPort->setPortName(configHelper->pc2plc_portName); // default "COM3"
        plcSerialPort->setBaudRate(QSerialPort::Baud115200); // 2018.9.17
        plcSerialPort->setParity(QSerialPort::EvenParity);
        plcSerialPort->setDataBits(QSerialPort::Data7);
        plcSerialPort->setStopBits(QSerialPort::TwoStop);
        plcSerialPort->close();
        if (!plcSerialPort->open(QIODevice::ReadWrite)) {
            qWarning() << QStringLiteral("PLC: Can't open serial, error code %1").arg(plcSerialPort->error());
            emit connectError(1);
        } else {
            bConnected = true;
            emit connectError(0);
        }

        /*init the AD input module*/
        bool ok;
        int w1 = 1 << 15 | QString("1010").toInt(&ok, 2); //note: int(32) max ~= 10^10
        int w2 = 1 << 15;
        auto word102 = QByteArray::number(w1, 16).rightJustified(4, '0').toUpper();
        auto word103 = QByteArray::number(w2, 16).rightJustified(4, '0').toUpper();
        writePLC(genWRCode(102, word102 + word103));
    });
}

void Plc::readSensorPeriodically()
{
    if (!bUsrCtrl)
        return;
    QTimer::singleShot(cio0Interval, this, [=]() { readSensorPeriodically(); });
    auto ansCode = readPLC(genRRCode(0, 1));
    auto dataList = getRRData(ansCode);
    if (dataList.size() != 1) // the data should be 1 word
    {
        qWarning() << "PLCSerial: readSensor error";
        return;
    }
    bool ok;
    WORD cio0 = QString(dataList.at(1)).toInt(&ok, 16); // convert 'FFFF' to int(10) //0x000~0x1ff, in fact cio0~11(16) ranges to 0xffff.
        // 输入位CIO发生变化

    recordCio0(cio0);
    checkTri();
    checkExpect();
}

void Plc::readSpeedPeriodically()
{
    if (!bUsrCtrl)
        return;
    QTimer::singleShot(adInterval, this, [=]() { readSpeedPeriodically(); });
    /*	AD分辨率为 1/6000，精度为0.8%满量程（0~55°C），即48单位。
	4~20mA电流范围对应 0000~1770hex(0~6000)。对应的台车（中轴）速度为0~3.59m/min
	可转换的数据范围为FED4~189Chex(-300~6300)。3.2~4mA范围内的电流用二进制补码表示。
	当输入低于指定范围（如低于3.2mA）时，将启动断线检测功能，数据将变为8000hex。
	注意：这个返回的数据是2byte的有符号数，在**64位编译器**中的short（16bit）符合条件 */
    const short AD_min = 0xFED4, AD_max = 0x189c, AD_4ma = 0, AD_20ma = 0x1770, AD_err = 0x8000, AD_res = 48;
    const double scale = 3.59 / 6000; // 1个刻度对应的速度

    auto ansCode = readPLC(genRRCode(2, 1)); //read cio2 (m+1, AD01) for 1 word
    auto dataList = getRRData(ansCode);
    if (dataList.size() != 1) // the data should be 1 word
    {
        qWarning() << "PLCSerial: readTruck error";
        return;
    }
    bool ok;
    const short dec = QString(dataList.at(1)).toInt(&ok, 16); // convert 'FFFF' to int(10)
    QMutexLocker locker(&mutex); // protect truckSpeed
    truckSpeed = 0;
    //断线检测
    if (AD_err == dec) {
        emit truckSpeedError(1);
        return;
    }
    // out of range(4~20ma), consider the accuracy/resolution
    if (dec < AD_4ma - AD_res || dec > AD_20ma + AD_res) {
        emit truckSpeedError(2);
        return;
    }
    //正常数据
    else {
        truckSpeed = dec * scale;
        emit truckSpeedReady();
    }
}

//! 用读取的cio数据检测是否满足触发条件（上升沿/下降沿）
void Plc::checkTri()
{
    auto _fckp = [&](CkPt& ckp) {
        auto _fss = [&](Sensor& sensor, Sensor& anotherss, bool in) {
            int triTiming;
            if (in)
                triTiming = LevelRecorder::NegativeEdge;
            else
                triTiming = LevelRecorder::PositiveEdge;
            if (sensor.sample.state(0) == triTiming)
                sensor.nTri++;
            if (sensor.nTri == 1) {
                // 如果该检测点另一个传感器没有坏，则检测该传感器是否亮了
                if (anotherss.lastbroken == 0) {
                    if (anotherss.nTri == 1)
                        handleCkpTri(ckp.id); //ol(0) or ir(1)
                }
                // 如果另一个传感器已经坏了，则独自工作
                else {
                    handleCkpTri(ckp.id);
                }
            }
        };
        bool in = false;
        if (ckp.id == 0 || ckp.id == 3)
            in = true; //ol ir 完全进入
        _fss(ckp.sensor0, ckp.sensor0, in);
        _fss(ckp.sensor1, ckp.sensor1, in);
    };
    _fckp(dev0.ckpL);
    _fckp(dev0.ckpR);
    _fckp(dev1.ckpL);
    _fckp(dev1.ckpR);
}

//< 在预测触发的时段内，检测光电开关和掉轮报警
void Plc::checkExpect()
{
    auto _fckp = [&](CkPt& ckp) {
        int colState = ckp.expectIn.state(0);
        // 在预期到达的时间开始时，开始检测点新的周期
        if (colState == LevelRecorder::PositiveEdge) {
            if (ckp.sensor0.nTri > 1 || ckp.sensor1.nTri > 1) {
                qDebug() << QStringLiteral("Plc: 传感器计算异常") << "sor0: " << ckp.sensor0.nTri << "sor1: " << ckp.sensor0.nTri;
            }
            ckp.newWheel();
        }
        // 在预期到达的最迟时间时，清算
        else if (colState == LevelRecorder::NegativeEdge) {
            auto _fss = [&](Sensor& sensor) {
                if (sensor.nTri == 1) {
                    sensor.expected = true;
                } else if (sensor.nTri == 0) {
                    sensor.expected = false;
                } else { /*默认为正确*/
                }
            };
            _fss(ckp.sensor0);
            _fss(ckp.sensor1);
            // 针对车轮离开检测区域的检测点，进行车轮传感器好坏的结算
            if (ckp.id == 1 || ckp.id == 2)
                checkBrokenAlarm(ckp.id >> 1);
        }
    };

    _fckp(dev0.ckpL);
    _fckp(dev0.ckpR);
    _fckp(dev1.ckpL);
    _fckp(dev1.ckpR);
}

void Plc::checkBrokenAlarm(int devId)
{
    auto&& selDev = [&](int id) -> auto& { return id == 0 ? dev0 : dev1; };
    auto&& dev = selDev(devId);
    int countExpected = dev.ckpL.sensor0.expected + dev.ckpL.sensor1.expected + dev.ckpR.sensor0.expected + dev.ckpR.sensor1.expected;

    /*如果没有任何一个传感器检测到车轮正常到达的信号，
	1. 掉轮了
	2. 传感器已经全部报废了（或只剩一个）*/
    if (countExpected == 0) {
        if (dev.lastalarm == 1) {
            /*上个轮子只有一个传感器是好的，并且没有检测到轮子*/
            //这次还是没检测到轮子。传感器全坏了
            dev.ckpL.sensor0.broken = 2;
            dev.ckpL.sensor1.broken = 2;
            dev.ckpR.sensor0.broken = 2;
            dev.ckpR.sensor1.broken = 2;
        }
        auto _cnt_sensor = [&](Sensor& sensor) {if (sensor.lastbroken == 2) return 1; else return 0; };
        auto _cnt_ckp = [&](CkPt& ckp) { return _cnt_sensor(ckp.sensor0) + _cnt_sensor(ckp.sensor1); };
        auto _count_broken = [&](Device& dev) { return _cnt_ckp(dev.ckpL) + _cnt_ckp(dev.ckpR); };
        int cntbrk = _count_broken(dev);
        /*历史记录中，传感器已经全部坏了*/
        if (cntbrk == 4) {
            dev.alarm = 0;
        }
        /*已经坏了3个传感器，无法确认本次的异常是唯一正常的传感器坏了，还是掉轮了*/
        else if (cntbrk == 3) {
            dev.alarm = 1;
        }
        /*上次至少还有2个传感器是好的，本次一个都没有检测到车轮到达信号，确认掉轮*/
        else {
            dev.alarm = 2;
            emit alarmwheel(0); //本应该刚刚离开的dz的轮子掉了
        }
    }
    /*任何一个传感器检测到车轮，掉轮不可能*/
    else {
        dev.alarm = 0;
        if (dev.lastalarm == 1)
            /*上个轮子只有一个传感器是好的，并且没有检测到轮子。这次却检测到了信号，则好的这个传感器没坏，是上个轮子确实掉了*/
            emit alarmwheel(-1);
    }
    //确认没有掉轮情况下，所有unexpected都是由于传感器坏了
    if (dev.alarm == 0) {
        auto f = [&](Sensor& sensor) {if (sensor.expected == false) { sensor.broken = 2; } };
        f(dev.ckpL.sensor0);
        f(dev.ckpL.sensor1);
        f(dev.ckpR.sensor0);
        f(dev.ckpR.sensor1);
    }
    // 无论有没有掉轮，所有正常输出的传感器都是好的all expected set to not broken
    auto f = [&](Sensor& sensor) {
        if (sensor.expected == true) {
            sensor.broken = 0;
        }
        if (sensor.lastbroken < 2 && sensor.broken == 2) {
            emit 传感器坏咯
        }
        sensor.lastbroken = sensor.broken;
    };
    f(dev.ckpL.sensor0);
    f(dev.ckpL.sensor1);
    f(dev.ckpR.sensor0);
    f(dev.ckpR.sensor1);
    dev.lastalarm = dev.alarm;
}

void Plc::recordCio0(WORD cio0)
{
    bool bChanged = false;
    Device* dev;
    CkPt* ckp;
    Sensor* sensor;
    //cio 0.01 sol0
    cio0 = cio0 >> 1;
    dev = &dev0;
    ckp = &dev->ckpL;
    sensor = &ckp->sensor0;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;
    //cio 0.02 sol1
    cio0 = cio0 >> 1;
    dev = &dev0;
    ckp = &dev->ckpL;
    sensor = &ckp->sensor1;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;
    //cio 0.03 sor0
    cio0 = cio0 >> 1;
    dev = &dev0;
    ckp = &dev->ckpR;
    sensor = &ckp->sensor0;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;
    //cio 0.04 sor1
    cio0 = cio0 >> 1;
    dev = &dev0;
    ckp = &dev->ckpR;
    sensor = &ckp->sensor1;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;
    //cio 0.05 sir0
    cio0 = cio0 >> 1;
    dev = &dev1;
    ckp = &dev->ckpR;
    sensor = &ckp->sensor0;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;
    //cio 0.06 sir1
    cio0 = cio0 >> 1;
    dev = &dev1;
    ckp = &dev->ckpR;
    sensor = &ckp->sensor1;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;
    //cio 0.07 sil0
    cio0 = cio0 >> 1;
    dev = &dev1;
    ckp = &dev->ckpL;
    sensor = &ckp->sensor0;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;
    //cio 0.08 sil1
    cio0 = cio0 >> 1;
    dev = &dev1;
    ckp = &dev->ckpL;
    sensor = &ckp->sensor1;
    sensor->sample.push(cio0 & 1); // 记录sensor的状态
    auto state = sensor->sample.state(0);
    if (state == LevelRecorder::NegativeEdge || state == LevelRecorder::PositiveEdge)
        bChanged = true;

    if (bChanged) {
        emit cio0Changed(cio0);
    }; // 更改ui显示
}

QByteArray Plc::readPLC(QByteArray plcData)
{
    QByteArray responseData;
    plcSerialPort->write(plcData);
    if (plcSerialPort->waitForBytesWritten(100)) {
        if (plcSerialPort->waitForReadyRead(100)) {
            QByteArray responseData = plcSerialPort->readAll();
            while (plcSerialPort->waitForReadyRead(100))
                responseData += plcSerialPort->readAll();
        }
    }
    if (responseData.isNull())
        qWarning() << "PLC: readPLC() error";
    return responseData;
}

void Plc::writePLC(QByteArray plcData)
{
    plcSerialPort->write(plcData);
    if (plcSerialPort->waitForBytesWritten(100)) {
        if (plcSerialPort->waitForReadyRead(100)) {
            QByteArray responseData = plcSerialPort->readAll();
            while (plcSerialPort->waitForReadyRead(100))
                responseData += plcSerialPort->readAll();
            if (!checkAnsCode(responseData))
                qWarning() << "PLC: Wrong response!";
        }
    }
}
QByteArray Plc::getFCSCode(QByteArray cmd)
{
    int res = 0;
    for (auto c : cmd) {
        res ^= c; // char is auto convert to ASCII
    }
    return QByteArray::number(res, 16).toUpper();
}

QByteArray Plc::genRRCode(int addr, int num)
{
    if (addr <= 6143 && addr >= 0 && num >= 1 && num <= 6144) {
        return getFullCode(QString("@00RR%1%2").arg(addr, 4, 10, QChar('0')).arg(num, 4, 10, QChar('0')).toUtf8());
    }
    qWarning() << "PLCSerial: addr or No. of words is error";
    return QByteArray();
}

QByteArray Plc::genWRCode(int addr, QByteArray data)
{
    if (addr <= 6143 && addr >= 0 && data.size() % 4 == 0) {
        return getFullCode(QString("@00WR%1%2").arg(addr, 4, 10, QChar('0')).arg(QString(data)).toUtf8());
    }
    qWarning() << "PLCSerial: addr or No. of words is error";
    return QByteArray();
}

bool Plc::checkAnsCode(QByteArray code)
{
    // null response
    if (code.isNull()) {
        return false;
    }
    //check the format
    if (code.startsWith("@00") && code.endsWith("*\r")) {
        code.chop(2); // chop the terminator
        auto fcs_recv = code.right(2);
        code.chop(2); // chop the fcs
        auto fcs_clc = getFCSCode(code);
        if (fcs_clc == fcs_recv) {
            auto headerCode = code.mid(3, 2);
            auto endCode = code.mid(5, 2);
            if (endCode == "00") {
                if (headerCode == "WR" || headerCode == "RR") {
                    return true;
                }
            }
        }
    }
    return false;
}

QByteArrayList Plc::getRRData(QByteArray ansCode)
{
    QByteArrayList words;
    if (!checkAnsCode(ansCode)) {
        qWarning() << "PLCSerial: plc response data err!";
        return words;
    }
    if (ansCode.at(3) != 'R') {
        qWarning() << "PLCSerial: try to read data from write cmd!";
        return words;
    }
    auto data = ansCode.mid(7, ansCode.size() - 7 - 4);
    // 读取的数据以字（FFFF）为单位
    if (data.size() % 4 != 0) {
        qWarning() << "PLCSerial: plc response data err!";
        return words;
    }
    QByteArray word;
    for (size_t i = 0; i < data.size(); i++) {
        word += data.at(i);
        if ((i + 1) % 4 == 0) {
            words << word;
            word.clear();
        }
    }
    return words;
}
void Plc::onAlarmEvent(int id)
{
    // 此id已经根据调用的deviceIndex进行过处理,外0内1
    //001 010 101 110
    //bug2: 如何关闭警报呢-直接清零
    QByteArray plcData;

    //static防止会覆盖原有状态
    static WORD a = 0;
    switch (id) {
    case 0:
        a = 0; // will send 0000, cut off all alarm
        break;
    case 1:
        a |= 1 << 4; //CIO 100.04 out warn
        break;
    case 2:
        a |= 1 << 5; //CIO 100.05 out alarm
        break;
    case 5:
        a |= 1 << 6; //CIO 100.06 in warn
        break;
    case 6:
        a |= 1 << 7; //CIO 100.07 in alarm
        break;

    default: // not do anything
        break;
    }
    plcData = genWRCode(100, QByteArray::number(a, 16).rightJustified(4, '0').toUpper());
    writePLC(plcData);
}