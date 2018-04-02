#include "stdafx.h"
#include "improfile.h"

QDataStream& operator << (QDataStream &out, const ImProfile &p)
{
	out << p.warningRatio << p.alarmRatio << p.radius_min << p.radius_max << p.gs1 << p.gs2 << p.dp << p.minDist << p.param1 << p.param2 << p.sensorTriggered << p.roiRect.x << p.roiRect.y << p.roiRect.width << p.roiRect.height;
	return out;
}

QDataStream& operator >> (QDataStream &in, ImProfile &p)
{
	in >> p.warningRatio >> p.alarmRatio >> p.radius_min >> p.radius_max >> p.gs1 >> p.gs2 >> p.dp >> p.minDist >> p.param1 >> p.param2 >> p.sensorTriggered >> p.roiRect.x >> p.roiRect.y >> p.roiRect.width >> p.roiRect.height;
	return in;
}