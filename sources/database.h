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
	query.exec("CREATE TABLE wheels ( "
		"id         INTEGER         PRIMARY KEY AUTOINCREMENT,"
		"num        VARCHAR( 3 ),"
		"time       DATETIME,"
		"calcspeed  DOUBLE,"
		"refspeed   DOUBLE,"
		"error      DOUBLE,"
		"alarmlevel INT,"
		"checkstate INT,"
		"fragment   INT,"
		"totalmatch INT,"
		"validmatch INT,"
		"speeds     VARCHAR( 255 ),"
		"videopath  VARCHAR( 255 ) "
		");");
	//query.exec("insert into wheels (num,videopath) values('007','E:/1.mp4')");
	return true;
}