#pragma once

#include <QtSql>
#include <QtWidgets>

static bool creatDB()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

	db.setHostName("127.0.0.1");
	db.setDatabaseName("usersDB.db");
	db.setUserName("BaoSteel");
	db.setPassword("123456");
	if (!db.open())
	{
		qWarning() << "Database Error" << db.lastError().text();
		return false;
	}

	QSqlQuery query;
	query.exec("create table user(id varchar primary key, pwd varchar)");
	query.exec(QStringLiteral("insert into user values('BaoSteel', '123456')"));
	return true;
}