#include "stdafx.h"
#include "Plc.h"
#include "TrunckRef.h"

const double TrunckRef::std_L = 2.45;
const double TrunckRef::dist_min = std_L - 0.1;
const double TrunckRef::dist_max = std_L + 0.1;
TrunckRef::TrunckRef(Plc* plcSerial, QObject* parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, plc(plcSerial)
{
}

TrunckRef::~TrunckRef()
= default;

void TrunckRef::start()
{
	//这里必须用[=]而不是[&]
	QTimer::singleShot(0, this, [=]() {
		for (auto& ckp : ckps) {
			ckp = Ckpt{}; //重置
		}
		bUsrCtrl = true;
		speedIntegrator();
	});
}

void TrunckRef::stop()
{
	QTimer::singleShot(0, this, [=]() { bUsrCtrl = false; }); //用timeevent，避免跨线程操作带来的锁的管理。但是需要直接get的value，必须通过锁管理
}

void TrunckRef::speedIntegrator()
{
	if (!bUsrCtrl)
		return;
	QTimer::singleShot(readSpeedInterval, this, [=]() { speedIntegrator(); });
	auto spd = plc->getTruckSpeed();
	auto s_tmp = spd * readSpeedInterval / 60000;
	for (auto& ckp : ckps) {
		//从第一次触发后开始
		if (!ckp.bFirstStart)
			continue;
		ckp.distance += s_tmp;
		int ckpindex = &ckp - ckps;
		if (!ckp.triBeginReady && ckp.distance > dist_min) {
			emit expectTriBegin(ckpindex);
			//qDebug() << "ckp expect begin" << ckpindex;
			ckp.triBeginReady = true;
		}
		if (!ckp.triEndReady && ckp.distance > dist_max) {
			emit expectTriEnd(ckpindex);
			//qDebug() << "ckp expect end" << ckpindex;
			ckp.triEndReady = true;
			ckp.newWheel();
		}
	}
}

void TrunckRef::onCkpTri(int ckpId)
{
	auto& ckp = ckps[ckpId];
	if (!ckp.bFirstStart)
	{
		ckp.bFirstStart = true; //积分器在第一次tri信号开始运算，此后不间断（不会被置零）
		//qDebug() << "ckp integrator start:" << ckpId;
	}
	else
	{
		//qDebug() << "ckp tri:" << ckpId << " dis:" << ckp.distance;
	}
	ckp.anchor = ckp.distance;
	ckp.hasTri = true;
}