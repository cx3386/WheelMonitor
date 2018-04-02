#include "stdafx.h"
#include "camprofile.h"

QDataStream& operator<<(QDataStream &out, const CamProfile &p)
{
	out << p.camIP << p.camPort << p.camUserName << p.camPassword << p.frameInterv;
	return out;
}

QDataStream& operator>>(QDataStream &in, CamProfile &p)
{
	in >> p.camIP >> p.camPort >> p.camUserName >> p.camPassword >> p.frameInterv;
	return in;
}