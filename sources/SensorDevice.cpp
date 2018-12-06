#include "SensorDevice.h"
#include "stdafx.h"

const QStringList SensorDevice::names{ "outer", "inner" };
const QStringList Sensor::names{ "left", "right" };
const QStringList CkPt::names{ "left", "right" };

SensorDevice::SensorDevice(int _id)
    : m_id(_id)
{
    name = names[m_id];
    id = m_id;
    ckpts = CkPt::creatAll(this);
}

//! 创建所有的（内/外圈）设备，返回设备的列表
QList<SensorDevice> SensorDevice::createAll()
{
    QList<SensorDevice> devs;
    //for (int i = 0; i < names.size(); ++i) {
    //    devs << SensorDevice{ i };
    //}
    return devs;
}

void SensorDevice::newWheel()
{
    alarm = 0;
    for (auto& ckpt : ckpts) {
        ckpt.newWheel();
    }
}

void SensorDevice::init()
{
    newWheel();
    for (auto& ckpt : ckpts) {
        ckpt.init();
    }
    lastalarm = 0;
}

SensorDevice::~SensorDevice()
{
}

Sensor::Sensor(CkPt* parent, int _id)
    : parentCkPt(parent)
    , m_id(_id)
{
    name = names[m_id];
    id = parentCkPt->id << 1 | m_id;
}

QList<Sensor> Sensor::createAll(CkPt* parent)
{
    QList<Sensor> sensors;
    for (int i = 0; i < names.size(); ++i) {
        sensors << Sensor{ parent, i };
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

CkPt::CkPt(SensorDevice* parent, int _id)
    : parentDev(parent)
    , m_id(_id)
{
    name = names[m_id];
    id = parentDev->id << 1 | m_id;
    sensors = Sensor::createAll(this);
}

QList<CkPt> CkPt::creatAll(SensorDevice* parent)
{
    QList<CkPt> ckps;
    for (int i = 0; i < names.size(); ++i) {
        ckps << CkPt{ parent, i };
    }
    return ckps;
}

void CkPt::newWheel()
{
    for (auto& sensor : sensors) {
        sensor.newWheel();
    }
}

void CkPt::init()
{
    for (auto& sensor : sensors) {
        sensor.newWheel();
        sensor.init();
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