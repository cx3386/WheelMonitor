/**
 * \file common.h
 *
 * \author cx3386
 * \date 2018/01/24
 *
 * \brief include the common definition.
 * - detailed information about this application, used in windows property dialog.
 * - global application, file or folder path, or name.
 * - global database connection name(threads should NOT share connections to database).
 * - global enums.
 * - other sundry.
 */
#pragma once

 // detailed information about this application.

const QString FILE_SPEC = QStringLiteral("宝钢环冷机台车轮子速度检测软件");
const char FILE_VER[] = "1.1.1";
const QString PRODUCT_NAME = QStringLiteral("宝钢环冷机台车轮子速度检测软件");
const char PRODUCT_VER[] = "1.1.1";
const char COPYRIGHT[] = "Copyright(C) 2018 ZJU. All Rights Reserved";
const char AUTHOR[] = "cx3386@163.com";
const char LANGUAGE[] = "zh_cn";

// App Dir defined in main.cpp
extern QString appDirPath;     ///< the directory contains the app.exe, '/'e.g. C:/QQ
extern QString appFilePath;    ///< the file path of app.exe, '/'e.g. C:/QQ/qq.exe
extern QString captureDirPath; ///< capture dir
extern QString configDirPath;    ///< config dir
extern QString videoDirPath; ///< video dir
extern QString cacheDirPath; ///< cache dir
extern QString matchDirPath;	///< match dir
//extern QString imageDirPath; ///< imageDirPath
extern QString logDirPath;     ///< log dir
extern QString ocrPatternDirPath;     ///< ocr sample dir
extern QString ocrDirPath;     ///< ocr dir
extern QString databaseFilePath; ///< database file path

//char dbDir = char("%1").arg(appDirPath);	///< db dir	-same as appdir
//char iniDir = char("%1").arg(appDirPath);	///< ini dir	-same as appdir
extern QString appName;

const char BACKUP_ZIP_NAME[] = "WheelMonitor Bak.zip";
//need identity of cpu and mac
//bool bNeedIdentity;
const char MAIN_CONNECTION_NAME[] = "main";
const char THEAD0_CONNECTION_NAME[] = "thread0";
const char THEAD1_CONNECTION_NAME[] = "thread1";

/// Alarm color, generally used in client-side
enum AlarmEvent
{
	AlarmLevel
};

static char* str2charx(const std::string &str) {
	auto ch = (char *)malloc(std::strlen(str.c_str()) + 1);
	std::strcpy(ch, str.c_str());
	return ch;
}

/**
 * \brief get device mark of device index
 *
 * \param int id device index
 */
static QString getDeviceMark(int id) {
	if (0 == id) return QStringLiteral("外");
	if (1 == id) return QStringLiteral("内");
}

/**
 * \brief get device index of device mark
 *
 * \param QString qstr device mark
 */
static int getDeviceIndex(QString qstr) {
	if (qstr == QStringLiteral("外")) return 0;
	if (qstr == QStringLiteral("内")) return 1;
}