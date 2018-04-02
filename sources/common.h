/**
 * \file common.h
 * \date 2018/01/19 20:51
 *
 * \author cx3386
 * Contact: cx3386@163.com
 * \par Copyright(c):
 * cx3386.
 * All Rights Reserved
 * \brief include the common definition.
 *
 * - detailed information about this application, used in windows property dialog.
 * - global application, file or folder path, or name.
 * - global database connection name(threads should NOT share connections to database).
 * - global enums.
 * - other sundry.
 *
 * \note
 *
 * \version 1.0
*/
#pragma once

/// detailed information about this application.

const QString FileSpec = "WheelMonitor";
const QString FileVer = "1.1.1";
const QString ProductName = QStringLiteral("宝钢环冷机台车轮子速度检测软件");
const QString ProductVer = "1.1.1";
const QString Copyright = "Copyright(C) 2018 ZJU. All rights reserved";
const QString Author = "cx3386@163.com";

/// App Dir defined in main.cpp
extern QString appDirPath;     ///< the directory contains the app.exe, '/'e.g. C:/QQ
extern QString appFilePath;    ///< the file path of app.exe, '/'e.g. C:/QQ/qq.exe
extern QString captureDirPath; ///< capture dir
extern QString configDirPath;    ///< config dir
extern QString videoDirPath; ///< video dir
extern QString matchDirPath;	///< match dir
//extern QString imageDirPath; ///< imageDirPath
extern QString logDirPath;     ///> log dir
extern QString ocrPatternDirPath;     ///< ocr sample dir
extern QString ocrDirPath;     ///> ocr dir
extern QString databaseFilePath; ///> database file path

//QString dbDir = QString("%1").arg(appDirPath);	///> db dir	-same as appdir
//QString iniDir = QString("%1").arg(appDirPath);	///> ini dir	-same as appdir
extern QString appName;

const QString BackupZipName = "WheelMonitor Bak.zip";
//need identity of cpu and mac
//bool bNeedIdentity;
const QString MainConnectionName = "main";
const QString ThreadConnectionName = "thread";

///> Alarm color, generally used in client-side
/**
 * use | to group different color
 * e.g. AlarmColorRed | AlarmColorYellow = 6, represents showing red & yellow at same time
 * not implement yet
 */
enum AlarmColor {
	AlarmColorUnkown = 0,
	AlarmColorGreen = 1,
	AlarmColorRed = 2,
	AlarmColorYellow = 4,
	AlarmColorGray = 8,
	AlarmOFF = AlarmColorGray
};

enum DEVNAMES {
};