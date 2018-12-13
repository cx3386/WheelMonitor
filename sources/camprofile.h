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

// 为了设置文件.ini的可读性，仅保留这些重载，而不使用QSetting的stream功能（会导致qstring以编码形势保存）
QDataStream& operator<<(QDataStream& out, const CamProfile& p);
QDataStream& operator>>(QDataStream& in, CamProfile& p);
