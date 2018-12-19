#include "stdafx.h"

#include "AlarmEvent.h"
#include "Plc.h"
#include "TrunckRef.h"
#include "confighelper.h"
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
    QObject::connect(trnkRef, &TrunckRef::expectTriBegin, this, [=](int id) {
        int devId = id >> 1;
        int ckpId = id & 1;
        auto ckpt = devs[devId]->ckpts[ckpId];
        // ��Ԥ�ڵ����ʱ�俪ʼʱ����ʼ�����µ�����
        bool errorCntTri = false;
        for (auto ss : ckpt->sensors) {
            if (ss->nTri != 1) {
                errorCntTri = true;
                qDebug() << QStringLiteral("�����������쳣 ID:") << ss->id << " nTri:" << ss->nTri;
            }
        }
        ckpt->newWheel();
    });
    QObject::connect(trnkRef, &TrunckRef::expectTriEnd, this, [=](int id) {
        int devId = id >> 1;
        int ckpId = id & 1;
        auto ckpt = devs[devId]->ckpts[ckpId];
        // ����CKPT
        for (auto ss : ckpt->sensors) {
            if (ss->nTri == 1) {
                ss->expected = true;
            } else if (ss->nTri == 0) {
                ss->expected = false;
            } else {
            }
        }
        // ����Ǽ����ǳ����뿪��������㳵�ִ������û�
        if (ckpt->isLeave())
            checkBrokenAlarm(devId);
    });
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
        cio0_pre = 0;
        readSensorPeriodically();
        readSpeedPeriodically(); // sensor��Ϊ��,ʹ�ܵ�һ��.state
        for (auto dev : devs) {
            dev->init();
        }
        // ��ʼtruckref�ļ���
        trnkRef->start();
    });
}

void Plc::stop()
{
    QTimer::singleShot(0, this, [=]() {
        bUsrCtrl = false;
        trnkRef->stop();
        emit cio0Update(0);
    }); //��timeevent��������̲߳������������Ĺ���������Ҫget��value������ͨ��������
}

//! ���ӵ�PLC����ʼ��AD041����plc�������Ƶ����̺߳�ִ�У�Ӧ���˳�����ʱ�Ͽ�
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
        qWarning() << "read cio0 error. anscode " << ansCode;
        return;
    }
    bool ok;
    int cio0 = QString(dataList.at(0)).toInt(&ok, 16); // convert 'FFFF' to int(10) //0x000~0x1ff, in fact cio0~11(16) ranges to 0xffff.
    // PATCH: cioO.02����cio0.10 //  [12/5/2018 cx3386]
    // ��ȡ��iλ������
    auto getBit = [](int cio, int loc) -> bool { return cio >> loc & 1; };
    // ����iλ����Ϊb
    auto setBit = [](int& cio, int loc, bool bit) {
        if (bit)
            cio |= 1 << loc;
        else
            cio &= ~(1 << loc);
    };
    auto patch_swicth = [getBit, setBit](int& cio, int bit1, int bit2) {
        auto bit_tmp = getBit(cio, bit1);
        setBit(cio, bit1, getBit(cio, bit2));
        setBit(cio, bit2, bit_tmp);
    };
    patch_swicth(cio0, 2, 10);
    if (cio0 != cio0_pre) {
        emit cio0Update(cio0);
        cio0_pre = cio0;
    }
    recordCio0(cio0);
    checkTri();
}

void Plc::readSpeedPeriodically()
{
    if (!bUsrCtrl)
        return;
    QTimer::singleShot(adInterval, this, [=]() { readSpeedPeriodically(); });
    /*	AD�ֱ���Ϊ 1/6000������Ϊ0.8%�����̣�0~55��C������48��λ��
	4~20mA������Χ��Ӧ 0000~1770hex(0~6000)����Ӧ��̨�������ᣩ�ٶ�Ϊ0~3.59m/min
	��ת�������ݷ�ΧΪFED4~189Chex(-300~6300)��3.2~4mA��Χ�ڵĵ����ö����Ʋ����ʾ��
	���������ָ����Χ�������3.2mA��ʱ�����������߼�⹦�ܣ����ݽ���Ϊ8000hex��
	ע�⣺������ص�������2byte���з���������**64λ������**�е�short��16bit���������� */
    const short AD_min = 0xFED4, AD_max = 0x189c, AD_4ma = 0, AD_20ma = 0x1770, AD_err = 0x8000, AD_res = 48;
    const double scale = 3.59 / 6000; // 1���̶ȶ�Ӧ���ٶ�

    auto ansCode = readPLC(genRRCode(2, 1)); //read cio2 (m+1, AD01) for 1 word
    auto dataList = getRRData(ansCode);
    if (dataList.size() != 1) // the data should be 1 word
    {
        qWarning() << "read ad error. anscode " << ansCode;
        return;
    }
    bool ok;
    const short dec = QString(dataList.at(0)).toInt(&ok, 16); // convert 'FFFF' to int(10)
    QMutexLocker locker(&mutex); // protect truckSpeed
    truckSpeed = 0;
    //���߼��
    if (AD_err == dec) {
        emit truckSpeedError(1);
        return;
    }
    // out of range(4~20ma), consider the accuracy/resolution
    if (dec < AD_4ma - AD_res || dec > AD_20ma + AD_res) {
        emit truckSpeedError(2);
        return;
    }
    //��������
    else {
        truckSpeed = dec * scale;
        emit truckSpeedReady();
    }
}

void Plc::recordCio0(int cio0)
{
    //cio 0.01 sol0
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("outer")]->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("left")]->sample.push(cio0 & 1);
    //cio 0.02 sol1
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("outer")]->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("right")]->sample.push(cio0 & 1);
    //cio 0.03 sor0
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("outer")]->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("left")]->sample.push(cio0 & 1);
    //cio 0.04 sor1
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("outer")]->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("right")]->sample.push(cio0 & 1);
    //cio 0.05 sir0
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("inner")]->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("right")]->sample.push(cio0 & 1);
    //cio 0.06 sir1
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("inner")]->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("left")]->sample.push(cio0 & 1);
    //cio 0.07 sil0
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("inner")]->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("right")]->sample.push(cio0 & 1);
    //cio 0.08 sil1
    cio0 = cio0 >> 1;
    devs[SensorDevice::indexOf("inner")]->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("left")]->sample.push(cio0 & 1);
}

//! �ö�ȡ��cio���ݼ���Ƿ����㴥��������������/�½��أ�
void Plc::checkTri()
{
    for (auto dev : devs) {
        for (auto ckp : dev->ckpts) {
            QList<int> brokenId;
            bool isTrigger = false; //����
            for (auto ss : ckp->sensors) {
                if (ss->sample.state(0) == ckp->triTiming()) {
                    isTrigger = true;
                    ss->nTri++;
                    if (ss->lastbroken == 2)
                        brokenId << ss->m_id;
                }
            }
            bool ckpSig = false;
            if (isTrigger) {
                // ����������
                if (brokenId.empty()) {
                    if (ckp->sensors[0]->nTri == 1 && ckp->sensors[1]->nTri == 1)
                        ckpSig = true;
                }
                // ��������
                else if (brokenId.size() == 2) {
                }
                // ֻ��һ����,���õ��Ǹ�
                else if (brokenId.size() == 1) {
                    if (ckp->sensors[1 - brokenId[0]]->nTri == 1)
                        ckpSig = true;
                }
                if (ckpSig) {
                    if (ckp->isEnter())
                        emit _DZIn(dev->id);
                    else {
                        emit _DZOut(dev->id);
                    }
                    emit ckpTri(ckp->id);
                }
            }
        }
    }
}

void Plc::checkBrokenAlarm(int devId)
{
    auto dev = devs[devId];
    int countExpected, cntLastBrk;
    for (auto ckp : dev->ckpts) {
        for (auto ss : ckp->sensors) {
            countExpected += ss->expected;
            if (ss->lastbroken == 2)
                cntLastBrk++;
        }
    }

    /*���û���κ�һ����������⵽��������������źţ�
	1. ������
	2. �������Ѿ�ȫ�������ˣ���ֻʣһ����*/
    if (countExpected == 0) {
        if (dev->lastalarm == 1) {
            /*�ϸ�����ֻ��һ���������Ǻõģ�����û�м�⵽����*/
            //��λ���û��⵽���ӡ�������ȫ����
            for (auto ckp : dev->ckpts) {
                for (auto ss : ckp->sensors) {
                    ss->broken = 2;
                }
            }
        }
        /*��ʷ��¼�У��������Ѿ�ȫ������*/
        if (cntLastBrk == 4) {
            dev->alarm = 0;
        }
        /*�Ѿ�����3�����������޷�ȷ�ϱ��ε��쳣��Ψһ�����Ĵ��������ˣ����ǵ�����*/
        else if (cntLastBrk == 3) {
            dev->alarm = 1;
        }
        /*�ϴ����ٻ���2���������Ǻõģ�����һ����û�м�⵽���ֵ����źţ�ȷ�ϵ���*/
        else {
            dev->alarm = 2;
            emit wheelFallOff(0); //��Ӧ�øո��뿪��dz�����ӵ���
        }
    }
    /*�κ�һ����������⵽���֣������ܵ���*/
    else {
        dev->alarm = 0;
        if (dev->lastalarm == 1)
            /*�ϸ�����ֻ��һ���������Ǻõģ�����û�м�⵽���ӡ����ȴ��⵽���źţ���õ����������û�������ϸ�����ȷʵ����*/
            emit wheelFallOff(-1);
    }
    //ȷ��û�е�������£�����unexpected�������ڴ���������
    if (dev->alarm == 0) {
        for (auto ckp : dev->ckpts) {
            for (auto ss : ckp->sensors) {
                if (!ss->expected) {
                    ss->broken = 2;
                }
            }
        }
    }
    // ������û�е��֣�������������Ĵ��������Ǻõ�all expected set to not broken
    bool ok;
    int sensorState = QString("11111111").toInt(&ok, 2);
    for (auto ckp : dev->ckpts) {
        for (auto ss : ckp->sensors) {
            if (ss->expected) {
                ss->broken = 0;
            }
            if (ss->lastbroken < 2 && ss->broken == 2) {
                sensorState ^= (1 << ss->id);
            }
            ss->lastbroken = ss->broken;
        }
    }
    emit sensorUpdate(sensorState);
    dev->lastalarm = dev->alarm;
}

QByteArray Plc::readPLC(QByteArray plcData)
{
    QByteArray responseData;
    plcSerialPort->write(plcData);
    if (plcSerialPort->waitForBytesWritten()) {
        if (plcSerialPort->waitForReadyRead()) {
            responseData = plcSerialPort->readAll();
            while (plcSerialPort->waitForReadyRead(100))
                responseData += plcSerialPort->readAll();
        }
    }
    if (responseData.isNull())
        qWarning() << "response data is null";
    return responseData;
}

void Plc::writePLC(QByteArray plcData)
{
    plcSerialPort->write(plcData);
    if (plcSerialPort->waitForBytesWritten()) {
        if (plcSerialPort->waitForReadyRead()) {
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
    // ��ȡ���������֣�FFFF��Ϊ��λ
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
/**
 * \brief ���пط�������
 *
 * ע�⣺���Ḳ��ԭ��״̬
 */
void Plc::onHardAlarm(int cio)
{
    QByteArray plcData;
    plcData = genWRCode(100, QByteArray::number(cio, 16).rightJustified(4, '0').toUpper());
    writePLC(plcData);
}