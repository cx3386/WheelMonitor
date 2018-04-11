#pragma once
#include <opencv.hpp>
#include <QMetaType>

struct CamProfile {
	QString camIP = "192.168.2.84";
	int camPort = 8000;
	QString camUserName = "admin";
	QString camPassword = "baosteel123";
	int frameInterv = 7;
};
Q_DECLARE_METATYPE(CamProfile)

QDataStream& operator << (QDataStream &out, const CamProfile &p);
QDataStream& operator >> (QDataStream &in, CamProfile &p);
