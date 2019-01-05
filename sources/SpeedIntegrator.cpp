#include "stdafx.h"
#include "Plc.h"
#include "SpeedIntegrator.h"

const double SpeedIntegrator::std_L = 2.5;
const double SpeedIntegrator::dist_min = std_L - 0.2;
const double SpeedIntegrator::dist_max = std_L + 0.2;
SpeedIntegrator::SpeedIntegrator(Plc* plcSerial, QObject* parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, plc(plcSerial)
{
}

SpeedIntegrator::~SpeedIntegrator()
= default;

void SpeedIntegrator::start()
{
	//���������[=]������[&]
	QTimer::singleShot(0, this, [=]() {
		for (auto& ckp : ckps) {
			ckp = Ckpt{}; //����
		}
		bUsrCtrl = true;
		speedIntegrator();
	});
}

void SpeedIntegrator::stop()
{
	QTimer::singleShot(0, this, [=]() { bUsrCtrl = false; }); //��timeevent��������̲߳������������Ĺ���������Ҫֱ��get��value������ͨ��������
}

void SpeedIntegrator::speedIntegrator()
{
	if (!bUsrCtrl)
		return;
	QTimer::singleShot(readSpeedInterval, this, [=]() { speedIntegrator(); });
	auto spd = plc->getTruckSpeed();
	auto s_tmp = spd * readSpeedInterval / 60000;
	for (auto& ckp : ckps) {
		//�ӵ�һ�δ�����ʼ
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

void SpeedIntegrator::onCkpTri(int ckpId)
{
	auto& ckp = ckps[ckpId];
	if (!ckp.bFirstStart)
	{
		ckp.bFirstStart = true; //�������ڵ�һ��tri�źſ�ʼ���㣬�˺󲻼�ϣ����ᱻ���㣩
		//qDebug() << "ckp integrator start:" << ckpId;
	}
	else
	{
		//qDebug() << "ckp tri:" << ckpId << " dis:" << ckp.distance;
	}
	ckp.anchor = ckp.distance;
	ckp.hasTri = true;
}