#include "stdafx.h"

#include "SensorDevice.h"

const QStringList SensorDevice::names{ "outer", "inner" };
const QStringList Sensor::names{ "left", "right" };
const QStringList CkPt::names{ "left", "right" };

SensorDevice::SensorDevice(int _id, QObject* parent)
    : QObject(parent)
    , m_id(_id)
{
    name = names[m_id];
    id = m_id;
    ckpts = CkPt::creatAll(this);
}

//! 创建所有的（内/外圈）设备，返回设备的列表
QList<SensorDevice*> SensorDevice::createAll(QObject* parent)
{
    QList<SensorDevice*> devs;
    for (int i = 0; i < names.size(); ++i) {
        devs << new SensorDevice{ i, parent };
    }
    return devs;
}

void SensorDevice::newWheel()
{
    alarm = 0;
    for (auto ckpt : ckpts) {
        ckpt->newWheel();
    }
}

void SensorDevice::init()
{
    newWheel();
    for (auto ckpt : ckpts) {
        ckpt->init();
    }
    lastalarm = 0;
}

SensorDevice::~SensorDevice()
{
}

Sensor::Sensor(int _id, CkPt* parent)
    : QObject(parent)
    , parentCkPt(parent)
    , m_id(_id)
{
    name = names[m_id];
    id = parentCkPt->id << 1 | m_id;
}

QList<Sensor*> Sensor::createAll(CkPt* parent)
{
    QList<Sensor*> sensors;
    for (int i = 0; i < names.size(); ++i) {
        sensors << new Sensor{ i, parent };
    }
    return sensors;
}

void Sensor::newWheel()
{
    nTri = 0;
    expected = true;
    broken = 0;
}

void Sensor::init()
{
    newWheel();
    sample.init(false);
    lastbroken = 0;
}

CkPt::CkPt(int _id, SensorDevice* parent)
    : QObject(parent)
    , m_id(_id)
    , parentDev(parent)
{
    name = names[m_id];
    id = parentDev->id << 1 | m_id;
    sensors = Sensor::createAll(this);
}

QList<CkPt*> CkPt::creatAll(SensorDevice* parent)
{
    QList<CkPt*> ckps;
    for (int i = 0; i < names.size(); ++i) {
        ckps << new CkPt{ i, parent };
    }
    return ckps;
}

void CkPt::newWheel()
{
    for (auto sensor : sensors) {
        sensor->newWheel();
    }
}

void CkPt::init()
{
    for (auto sensor : sensors) {
        sensor->newWheel();
        sensor->init();
    }
    //expectIn.init(false);
}

bool CkPt::isEnter()
{
    if (parentDev->name == "outer")
        if (name == "left")
            return true;
    if (parentDev->name == "inner")
        if (name == "right")
            return true;
    return false;
}