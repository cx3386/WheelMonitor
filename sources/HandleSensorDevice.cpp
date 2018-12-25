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
		bool ckpTri = false; //!< ��ǰckp�Ƿ��д���������
		for (auto ss : ckp->sensors) {
			if (ss->sample.state(0) == ckp->triTiming()) {
				ss->nTri++;
				ckpTri = true;
			}
		}
		if (ckpTri && !ckp->hasTri)
		{
			emitDzSignal(ckp->id);//��ÿ��ckp�����жϣ���ֹ�ظ�
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
	// ��������������
	if (broken0 && broken1) {
		//pass
		//�޸ģ�����
		if (ckp->sensors[0]->nTri == 1 || ckp->sensors[1]->nTri == 1) ckpTri = true;
	}
	//0�Ǻõ�
	else if (!broken0 && broken1) {
		if (ckp->sensors[0]->nTri == 1) ckpTri = true;
	}
	//1�Ǻõ�
	else if (broken0 && !broken1) {
		if (ckp->sensors[1]->nTri == 1) ckpTri = true;
	}
	// �������������Ǻõ�
	else if (!broken0 && !broken1) {
		if (ckp->sensors[0]->nTri == 1 && ckp->sensors[1]->nTri == 1) ckpTri = true;//ͬʱ�������
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
	// �������nTri
	ckpt->newWheel();
}

void HandleSensorDevice::checkoutCkp(int ckpId)
{
	int ckp_m_id = CkPt::gid2mid(ckpId);
	auto ckp = dev->ckpts[ckp_m_id];
	//���û�м�⵽������ͼ������ź�
	if (!ckp->hasTri)
	{
		if (ckp->isEnter())
			emit dzIn();
		else
			emit dzOut();
		ckp->hasTri = true;
	}

	for (auto ss : ckp->sensors) {
		int nTri = ss->nTri; // ��expect�����ڣ������Ĵ���
		switch (nTri)
		{
		case 0:
			ss->expected = false;//һ�ζ�δ��������δ�ﵽԤ��
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
	int goodones = getExpects(); //!< ����expect=true�ĸ���
	bool updateSensor; //!< ������ǵ��֣������ss->broken��
	//��⵽�ź�,�����ܵ���
	if (goodones > 0)
	{
		//qDebug() << "dev" << devId << ",goodones" << goodones;
		updateSensor = true;
		if (dev->needHandled)
		{
			emit wheelFallOff(-1);
		}
	}
	//û���κ�һ����������⵽�źţ�������ȫ����
	else
	{
		if (dev->needHandled) {
			//��ʱӦ����brokens=4
			updateSensor = true;
			//�ϸ�����û��fall off
			dev->needHandled = false;
		}
		int brokens = getBrokens(); //!< ��ǰ�Ѿ������Ĵ���������
		switch (brokens)
		{
		case 0:
		case 1:
		case 2://������һ�λ�2�����ǵ�����ɵĲ���
			updateSensor = false;
			emit wheelFallOff(0);
			break;
		case 3://����3���ˣ����һ���������������豸���ˣ�Ҳ�����ǵ���
			updateSensor = true;
			dev->needHandled = true;
			break;
		case 4://ȫ��
			//updateSensor = true;//�ϴκ���ζ�ȫ����û�б�Ҫ����
			break;
		default:
			break;
		}
	}

	if (updateSensor)
		updateBrokenSensor();//���������豸��״̬
	else
		updateGoodSensor();//ֻ����goodones

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
	int cnt; //!< �������ڻ�����ss
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
	//ͳ�����д�������״̬����������ui��ע�⣺ֻ���µ�ǰdev
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