#pragma once

#include <QtSql>
#include "common.h"

/// field index of table "wheels"
enum WheelsHeader
{
	Wheels_ID = 0,
	Wheels_Num,
	Wheels_CalcSpeed,
	Wheels_RefSpeed,
	Wheels_Error,
	Wheels_Time,
	Wheels_AlarmLevel,
	Wheels_CheckState,
	Wheels_OcrSize,
	Wheels_Fragment,
	Wheels_TotalMatch,
	Wheels_ValidMatch,
	Wheels_Speeds,
	Wheels_VideoPath,
};

/// the value of field "wheels_checkstate", represent whether need check. if need check, it will show in the alarm table
enum CheckState
{
	NoNeedCheck = 0,
	NeedCheck,
	Checked,
};

static bool initMainDb()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", MainConnectionName);

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
	QSqlQuery query(db);
	QSqlTableModel model(nullptr, db);
	model.setTable("user");
	model.select();
	/// if no user table, insert the default user;
	if (!model.rowCount())
	{
		query.exec("create table user (id integer primary key autoincrement, username varchar(20) unique, pwd varchar(20))");
		query.exec("insert into user (username, pwd) values('BaoSteel', '123456')");
	}
	return true;
}

static void initThreadDb()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", ThreadConnectionName);
	QSqlTableModel model(nullptr, db);
	model.setTable("wheels");
	model.select();
	auto record = model.record();
	QStringList refCols, dbCols;
	refCols << "id" << "num" << "calcspeed" << "refspeed" << "error" << "time" << "alarmlevel" << "checkstate" << "ocrsize" << "fragment" << "totalmatch" << "validmatch" << "speeds" << "videopath";
	for (int i = 0; i < record.count(); ++i)
	{
		dbCols << record.fieldName(i);
	}
	if (refCols != dbCols)
	{
		QSqlQuery query(db);
		query.exec("DROP TABLE wheels;");
		query.exec("CREATE TABLE wheels ( "
			"id         INTEGER         PRIMARY KEY AUTOINCREMENT,"
			"num        VARCHAR( 3 ),"
			"calcspeed  DOUBLE,"
			"refspeed   DOUBLE,"
			"error      DOUBLE,"
			"time       DATETIME,"
			"alarmlevel INT,"
			"checkstate INT,"
			"ocrsize    INT,"
			"fragment   INT,"
			"totalmatch INT,"
			"validmatch INT,"
			"speeds     VARCHAR( 255 ),"
			"videopath  VARCHAR( 255 ) "
			");");
	}
}