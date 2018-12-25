#include "stdafx.h"

#include "HandleSensorDevice.h"

HandleSensorDevice::HandleSensorDevice(int _devId, QObject *parent)
	: QObject(parent)
	, devId(_devId)
	, dev(new SensorDevice(_devId, this))
{
}

HandleSensorDevice::~HandleSensorDevice()
{
}

void HandleSensorDevice::input(int io)
{
	io2dev(io);
	checkTri();
}

void HandleSensorDevice::io2dev(int io)
{
	switch (devId)
	{
	case 0:
		//cio 0.01 sol0
		io = io >> 1;
		dev->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("left")]->sample.push(io & 1);
		//cio 0.02 sol1
		io = io >> 1;
		dev->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("right")]->sample.push(io & 1);
		//cio 0.03 sor0
		io = io >> 1;
		dev->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("left")]->sample.push(io & 1);
		//cio 0.04 sor1
		io = io >> 1;
		dev->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("right")]->sample.push(io & 1);
		break;
	case 1:
		//cio 0.05 sir0
		io = io >> 5;
		dev->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("right")]->sample.push(io & 1);
		//cio 0.06 sir1
		io = io >> 1;
		dev->ckpts[CkPt::indexOf("right")]->sensors[Sensor::indexOf("left")]->sample.push(io & 1);
		//cio 0.07 sil0
		io = io >> 1;
		dev->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("right")]->sample.push(io & 1);
		//cio 0.08 sil1
		io = io >> 1;
		dev->ckpts[CkPt::indexOf("left")]->sensors[Sensor::indexOf("left")]->sample.push(io & 1);
		break;
	default:
		break;
	}
}

void HandleSensorDevice::checkTri()
{
	for (auto ckp : dev->ckpts) {
		bool ckpTri = false; //!< 当前ckp是否有传感器触发
		for (auto ss : ckp->sensors) {
			if (ss->sample.state(0) == ckp->triTiming()) {
				ss->nTri++;
				ckpTri = true;
			}
		}
		if (ckpTri && !ckp->hasTri)
		{
			emitDzSignal(ckp->id);//对每个ckp进行判断，防止重复
		}
	}
}

void HandleSensorDevice::emitDzSignal(int ckpId)
{
	int ckp_m_id = CkPt::gid2mid(ckpId);
	auto ckp = dev->ckpts[ckp_m_id];
	bool broken0 = ckp->sensors[0]->broken;
	bool broken1 = ckp->sensors[1]->broken;
	bool ckpTri = false;
	// 两个传感器都坏
	if (broken0 && broken1) {
		//pass
		//修改：任意
		if (ckp->sensors[0]->nTri == 1 || ckp->sensors[1]->nTri == 1) ckpTri = true;
	}
	//0是好的
	else if (!broken0 && broken1) {
		if (ckp->sensors[0]->nTri == 1) ckpTri = true;
	}
	//1是好的
	else if (broken0 && !broken1) {
		if (ckp->sensors[1]->nTri == 1) ckpTri = true;
	}
	// 两个传感器都是好的
	else if (!broken0 && !broken1) {
		if (ckp->sensors[0]->nTri == 1 && ckp->sensors[1]->nTri == 1) ckpTri = true;//同时检测两个
	}

	if (ckpTri) {
		if (ckp->isEnter()) {
			emit dzIn();
		}
		else {
			emit dzOut();
		}
		emit triInt(ckp->id);
		ckp->hasTri = true;
	}
}

void HandleSensorDevice::newCkp(int ckpId)
{
	int ckp_m_id = CkPt::gid2mid(ckpId);
	auto ckpt = dev->ckpts[ckp_m_id];
	// ……检查nTri
	ckpt->newWheel();
}

void HandleSensorDevice::checkoutCkp(int ckpId)
{
	int ckp_m_id = CkPt::gid2mid(ckpId);
	auto ckp = dev->ckpts[ckp_m_id];
	//如果没有检测到，触发图像检测的信号
	if (!ckp->hasTri)
	{
		if (ckp->isEnter())
			emit dzIn();
		else
			emit dzOut();
		ckp->hasTri = true;
	}

	for (auto ss : ckp->sensors) {
		int nTri = ss->nTri; // 在expect区间内，触发的次数
		switch (nTri)
		{
		case 0:
			ss->expected = false;//一次都未触发，则未达到预期
			break;
		default:
			ss->expected = true;
			break;
		}
	}
	if (ckp->isLeave())
		checkoutDev();
}

void HandleSensorDevice::checkoutDev()
{
	int goodones = getExpects(); //!< 所有expect=true的个数
	bool updateSensor; //!< 如果不是掉轮，则更新ss->broken。
	//检测到信号,不可能掉轮
	if (goodones > 0)
	{
		//qDebug() << "dev" << devId << ",goodones" << goodones;
		updateSensor = true;
		if (dev->needHandled)
		{
			emit wheelFallOff(-1);
		}
	}
	//没有任何一个传感器检测到信号，即现在全坏了
	else
	{
		if (dev->needHandled) {
			//此时应该是brokens=4
			updateSensor = true;
			//上个车轮没有fall off
			dev->needHandled = false;
		}
		int brokens = getBrokens(); //!< 先前已经坏掉的传感器个数
		switch (brokens)
		{
		case 0:
		case 1:
		case 2://不可能一次坏2个，是掉轮造成的不亮
			updateSensor = false;
			emit wheelFallOff(0);
			break;
		case 3://坏了3个了，最后一个不亮，可能是设备坏了，也可能是掉轮
			updateSensor = true;
			dev->needHandled = true;
			break;
		case 4://全坏
			//updateSensor = true;//上次和这次都全坏，没有必要更新
			break;
		default:
			break;
		}
	}

	if (updateSensor)
		updateBrokenSensor();//更新所有设备的状态
	else
		updateGoodSensor();//只更新goodones

	updateSensorToUi();
}

void HandleSensorDevice::updateBrokenSensor()
{
	for (auto ckp : dev->ckpts) {
		for (auto ss : ckp->sensors) {
			if (!ss->expected)
				ss->broken = true;
			else
				ss->broken = false;
		}
	}
}
void HandleSensorDevice::updateGoodSensor()
{
	for (auto ckp : dev->ckpts) {
		for (auto ss : ckp->sensors) {
			if (ss->expected)
				ss->broken = false;
		}
	}
}

int HandleSensorDevice::getBreaks()
{
	int cnt; //!< 此周期内坏掉的ss
	for (auto ckp : dev->ckpts) {
		for (auto ss : ckp->sensors) {
			if (!ss->expected && !ss->broken) cnt++;
		}
	}
	return cnt;
}

int HandleSensorDevice::getBrokens()
{
	int cnt;
	for (auto ckp : dev->ckpts) {
		for (auto ss : ckp->sensors) {
			if (ss->broken) cnt++;
		}
	}
	return cnt;
}

int HandleSensorDevice::getExpects()
{
	int cnt = 0;
	for (auto ckp : dev->ckpts) {
		for (auto ss : ckp->sensors) {
			if (ss->expected) cnt++;
		}
	}
	return cnt;
}

void HandleSensorDevice::updateSensorToUi()
{
	//统计所有传感器的状态，并更新至ui。注意：只更新当前dev
	bool ok;
	int sensorState = QString("11111111").toInt(&ok, 2);
	for (auto ckp : dev->ckpts) {
		for (auto ss : ckp->sensors) {
			if (ss->broken) {
				sensorState ^= (1 << (ss->show_id - 1));
			}
		}
	}
	emit sensorUpdate(sensorState);
}