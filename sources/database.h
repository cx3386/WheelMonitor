#pragma once

#include <QtSql>
#include "common.h"

static bool initDataBase()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

	db.setDatabaseName(databaseFilePath);
	if (!db.open())
	{
		//qWarning() << "Database Error" << db.lastError().text(); //not log at this time
		QMessageBox::critical(0, qApp->tr("Cannot open database"),
			qApp->tr("Unable to establish a database connection.\n"
				"This application needs SQLite support. Please read "
				"the Qt SQL driver documentation for information how "
				"to build it.\n\n"
				"Click Cancel to exit."),
			QMessageBox::Cancel);
		return false;
	}

	QSqlQuery query;
	query.exec("create table user (id integer primary key autoincrement, username varchar(20) unique, pwd varchar(20))");
	query.exec("insert into user (username, pwd) values('BaoSteel', '123456')");
	//query.exec("create table matches(id integer primary key autoincrement, wheelid int, match 1, speed double, cart speed)");
	query.exec("create table wheels (id integer primary key autoincrement," //0
		"num varchar(3),"										   //1
		"time datetime,"										   //2
		"calcspeed double,"										   //3
		"refspeed double,"										   //4
		"error double,"											   //5
		"alarmlevel bool,"										   //6 0-right,1-warn,2-alrm
		"checked bool,"											   //7 is the alarm is checked, defaut is null, only alarm wheel need checked
		"fragment int,"											   //8
		"totalmatch int,"										   //9
		"validmatch int,"										   //10
		"speeds varchar(255),"									   //11 save rtspeeds and refspeeds vector
		"videopath varchar(255))");								   //12 videopath
	//query.exec("insert into wheels (num,videopath) values('007','E:/1.mp4')");
	return true;
}