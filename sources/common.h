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
#include <QString>

 // ע�⣺C�к꣬C++��constexpr��Ӧ������ͷ�ļ���
extern QMutex g_mutex; //!< global mutex
extern int g_videoKeepDays; //!< keep the video for n days;
 // detailed information about this application.
 //const QString FILE_SPEC = QStringLiteral("���ֻ����̨�������ٶȼ�����");
 //const char FILE_VER[] = "1.1.1";
const QString PRODUCT_NAME = QStringLiteral("���ֻ����̨������״̬���ϵͳ"); //!< displayname
//const char PRODUCT_VER[] = "1.1.1";
const char COPYRIGHT[] = "Copyright(C) 2018 ZJU. All Rights Reserved";
const char AUTHOR[] = "cx3386@163.com";
//const char LANGUAGE[] = "zh_cn";

// App Dir defined in main.cpp
extern QString appDirPath; //!<  the directory contains the app.exe, '/'e.g. C:/QQ
extern QString appFilePath; //!< the file path of app.exe, '/'e.g. C:/QQ/qq.exe
extern QString captureDirPath; //!< capture dir
extern QString configDirPath; //!< config dir
extern QString g_videoDirPath; //!< video dir
extern QString cacheDirPath; //!< cache dir
extern QString matchDirPath; //!< match dir
//extern QString imageDirPath; //!< imageDirPath
extern QString logDirPath; //!< log dir
extern QString ocrPatternDirPath; //!< ocr sample dir
extern QString ocrDirPath; //!< ocr dir
extern QString databaseFilePath; //!< database file path

//char dbDir = char("%1").arg(appDirPath);	//!< db dir	-same as appdir
//char iniDir = char("%1").arg(appDirPath);	//!< ini dir	-same as appdir

const char BACKUP_ZIP_NAME[] = "WheelMonitor Bak.zip";
//need identity of cpu and mac
//bool bNeedIdentity;

/* database connection name*/
//const char MAIN_CONNECTION_NAME[] = "main";
//const char THREAD0_CONNECTION_NAME[] = "thread0";
//const char THREAD1_CONNECTION_NAME[] = "thread1";

//���������ռ䣬LOCAL����static��ͬ��C++11
namespace {
	/**
		 * \brief get device mark of device index
		 *
		 * \param int id device index
		 */
	inline QString getDeviceMark(int id)
	{
		QString mark;
		switch (id)
		{
		case 0:
			mark = QStringLiteral("��");
			break;
		case 1:
			mark = QStringLiteral("��");
			break;
		default:
			break;
		}
		return mark;
	}

	/**
		 * \brief get device index of device mark
		 *
		 * \param QString qstr device mark
		 */
	inline int getDeviceIndex(QString qstr)
	{
		if (qstr == QStringLiteral("��"))
			return 0;
		if (qstr == QStringLiteral("��"))
			return 1;
	}
}