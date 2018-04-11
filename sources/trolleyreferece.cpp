#include "stdafx.h"
#include "trolleyreferece.h"
#include "plcserial.h"

TrolleyReferece::TrolleyReferece(PLCSerial *plcSerial, QObject *parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, plc(plcSerial)
{
}

TrolleyReferece::~TrolleyReferece()
= default;

void TrolleyReferece::speedIntegrator()
{
	for (double & i : traveledDistance)
	{
		i += timeInterval * (plc->getTrolleySpeed());
	}
}