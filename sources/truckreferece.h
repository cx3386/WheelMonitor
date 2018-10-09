#pragma once

#include <QObject>

class PLCSerial;
class TruckReferece : public QObject
{
	Q_OBJECT

public:
	TruckReferece(PLCSerial *plcSerial, QObject *parent = Q_NULLPTR);
	~TruckReferece();

private:
	PLCSerial * plc;
	double wheelDistance_min = 2.3; // unit:m
	double wheelDistance_max = 2.4; // m
	double globalTotalDistance;
	double traveledDistance[8]; ///< this wheel traveled distance. reset when a new wheel comes in
	int timeInterval = 250;//ms
	void speedIntegrator();
	public slots:
	inline void onWheelArrive(int sensorIndex) { traveledDistance[sensorIndex] = 0; }
};
