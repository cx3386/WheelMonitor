#pragma once
#include <opencv.hpp>
#include <QMetaType>

struct CamProfile {
	char* camIP = "192.168.2.84";
	WORD camPort = 8000;
	char* camUserName = "admin";
	char* camPassword = "www.cx3386.com";
	int frameInterv = 7;
};
Q_DECLARE_METATYPE(CamProfile)

QDataStream& operator << (QDataStream &out, const CamProfile &p);
QDataStream& operator >> (QDataStream &in, CamProfile &p);
