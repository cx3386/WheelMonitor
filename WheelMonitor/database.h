#pragma once

#include <QtSql>

static bool initDataBase()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

	db.setHostName("127.0.0.1");
	db.setDatabaseName("WheelMonitor.db3");
	db.setUserName("BaoSteel"); //sqlite no support for encryption
	db.setPassword("123456");
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
	query.exec("create table user(id integer primary key autoincrement, username varchar(20) unique, pwd varchar(20))");
	query.exec("insert into user (username, pwd) values('BaoSteel', '123456')");
	//query.exec("create table matches(id integer primary key autoincrement, wheelid int, match 1, speed double, cart speed)");
	query.exec("create table wheels(id integer primary key autoincrement,"	//0
		"num varchar(3),"	//1
		"time datetime,"	//2
		"fragment int,"	//3
		"totalmatch int,"	//4
		"validmatch int,"	//5
		"error double,"	//6
		"cartspeed double,"	//7
		"speeds varchar(255)),"	//save rtspeeds and refspeeds vector
		"alarm bool,"	//is this wheel alarmed
		"checked bool,"	//is the alarm is checked, defaut is null, only alarm wheel need checked
		"videopath varchar(255)");
	//query.exec("insert into wheels (num,videopath) values('007','E:/1.mp4')");
	return true;
}