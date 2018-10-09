#include "stdafx.h"
#include "truckreferece.h"
#include "plcserial.h"

TruckReferece::TruckReferece(PLCSerial *plcSerial, QObject *parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, plc(plcSerial)
{
}

TruckReferece::~TruckReferece()
= default;

void TruckReferece::speedIntegrator()
{
	for (double & i : traveledDistance)
	{
		i += timeInterval * (plc->getTruckSpeed());
	}
}