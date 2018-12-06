#include "TrunckRef.h"
#include "Plc.h"
#include "stdafx.h"

TrunckRef::TrunckRef(Plc* plcSerial, QObject* parent /*= Q_NULLPTR*/)
    : QObject(parent)
    , plc(plcSerial)
{
}

TrunckRef::~TrunckRef()
    = default;

void TrunckRef::start()
{
    QTimer::singleShot(0, this, [=]() {
        for (int i = 0; i < 4; i++) {
            auto& ckp = ckps[i];
            ckp = Ckpt{};
        }
        bUsrCtrl = true;
        speedIntegrator();
    });
}

void TrunckRef::stop()
{
    QTimer::singleShot(0, this, [=]() { bUsrCtrl = false; }); //��timeevent��������̲߳������������Ĺ���������Ҫֱ��get��value������ͨ��������
}

void TrunckRef::speedIntegrator()
{
    if (!bUsrCtrl)
        return;
    QTimer::singleShot(readSpeedInterval, this, [=]() { speedIntegrator(); });
    auto spd = plc->getTruckSpeed();
    auto s_tmp = spd * readSpeedInterval / 60000;
    for (int i = 0; i < 4; i++) {
        auto& ckp = ckps[i];
        if (!ckp.bStart)
            return;
        ckp.s += s_tmp;
        if (ckp.s == dist_min) {
            emit expectTriBegin(i);
        }
        if (ckp.s == dist_max) {
            emit expectTriEnd(i);
            ckp.newWheel();
        }
    }
}

void TrunckRef::onCkpTri(int ckpId)
{
    auto& ckp = ckps[ckpId];
    ckp.anchor = ckp.s;
    ckp.bStart = true;
    qDebug() << "dist between wheels: ckp(" << ckpId << ")=" << ckp.s;
}