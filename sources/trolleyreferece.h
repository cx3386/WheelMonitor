#pragma once

#include <QObject>

class PLCSerial;
class TrolleyReferece : public QObject
{
	Q_OBJECT

public:
	TrolleyReferece(PLCSerial *plcSerial, QObject *parent = Q_NULLPTR);
	~TrolleyReferece();

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
