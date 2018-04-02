#include "stdafx.h"
#include "ocrprofile.h"

QDataStream& operator<<(QDataStream &out, const OcrProfile &p)
{
	out << p.plate_x_min << p.plate_width_max << p.plate_y_min << p.plate_y_max << p.plate_width_min << p.plate_width_max << p.plate_height_min << p.plate_height_max << p.num_width_min << p.num_width_max << p.num_height_min << p.num_height_max;
	return out;
}

QDataStream& operator>>(QDataStream &in, OcrProfile &p)
{
	in >> p.plate_x_min >> p.plate_width_max >> p.plate_y_min >> p.plate_y_max >> p.plate_width_min >> p.plate_width_max >> p.plate_height_min >> p.plate_height_max >> p.num_width_min >> p.num_width_max >> p.num_height_min >> p.num_height_max;
	return in;
}