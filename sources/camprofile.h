#pragma once

#include <QMetaType>
#include <opencv.hpp>

struct CamProfile {
    QString camIP = "192.168.2.84";
    int camPort = 8000;
    QString camUserName = "admin";
    QString camPassword = "baosteel123";
    int frameInterv = 7;
};
Q_DECLARE_METATYPE(CamProfile)

// Ϊ�������ļ�.ini�Ŀɶ��ԣ���������Щ���أ�����ʹ��QSetting��stream���ܣ��ᵼ��qstring�Ա������Ʊ��棩
QDataStream& operator<<(QDataStream& out, const CamProfile& p);
QDataStream& operator>>(QDataStream& in, CamProfile& p);
