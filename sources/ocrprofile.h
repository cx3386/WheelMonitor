#pragma once
#include <opencv.hpp>
#include <QMetaType>
struct OcrProfile {
	int plate_x_min = 100;
	int plate_x_max = 250;
	int plate_y_min = 90;
	int plate_y_max = 200;
	int plate_width_min = 160;
	int plate_width_max = 190;
	int plate_height_min = 110;
	int plate_height_max = 130;
	int num_width_min = 20;
	int num_width_max = 50;
	int num_height_min = 50;
	int num_height_max = 75;
};
Q_DECLARE_METATYPE(OcrProfile)

QDataStream& operator << (QDataStream &out, const OcrProfile &p);
QDataStream& operator >> (QDataStream &in, OcrProfile &p);
