#pragma once

#include <QtSql>
#include "common.h"

/// field index of table "wheels"
enum WheelHeader
{
	Wheel_ID = 0,
	Wheel_I_O,
	Wheel_Num,
	Wheel_CalcSpeed,
	Wheel_RefSpeed,
	Wheel_Error,
	Wheel_Time,
	Wheel_AlarmLevel,
	Wheel_CheckState,
	Wheel_OcrSize,
	Wheel_Fragment,
	Wheel_TotalMatch,
	Wheel_ValidMatch,
	Wheel_Speeds,
	Wheel_VideoPath,
};

/// parameters of wheels to be recorded in the database
struct WheelDbInfo
{
	int id;
	QString i_o;
	QString num;
	double calcspeed;
	double refspeed;
	double error;
	QString time;
	int alarmlevel;
	int checkstate;
	int ocrsize;
	int fragment;
	int totalmatch;
	int validmatch;
	QString speeds;
	QString videopath;
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
	if (!QSqlDatabase::drivers().contains("QSQLITE"))
	{
		qWarning() << "database: no SQLITE driver.";
		QMessageBox::critical(Q_NULLPTR, "Unable to load database", "This app needs the SQLITE driver");
	}
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", MAIN_CONNECTION_NAME);
	db.setDatabaseName(databaseFilePath);
	if (!db.open())
	{
		qWarning() << "database: initMainDb failed. " << db.lastError().text();
		QMessageBox::critical(Q_NULLPTR, "Unable to initialize Database",
			"Eror initializing database: " + db.lastError().text());
		return false;
	}
	QSqlQuery query(db);
	QSqlTableModel model(nullptr, db);
	model.setTable("user");
	model.select();
	// if no user table, insert the default user;
	if (!model.rowCount())
	{
		query.exec("create table user (id integer primary key autoincrement, username varchar(20) unique, pwd varchar(20))");
		query.exec("insert into user (username, pwd) values('BaoSteel', '123456')");
	}
	return true;
}

static bool initThreadDb(int deviceIndex)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", deviceIndex ? THEAD1_CONNECTION_NAME : THEAD0_CONNECTION_NAME);
	db.setDatabaseName(databaseFilePath);
	if (!db.open())
	{
		qWarning() << "database: initThreadDb failed. " << db.lastError().text();
		QMessageBox::critical(Q_NULLPTR, "Unable to initialize Database",
			"Eror initializing database: " + db.lastError().text());
		return false;
	}
	QSqlTableModel model(nullptr, db);
	model.setTable("wheels");
	model.select();
	auto record = model.record();
	QStringList refCols, dbCols;
	refCols << "id" << "i_o" << "num" << "calcspeed" << "refspeed" << "error" << "time" << "alarmlevel" << "checkstate" << "ocrsize" << "fragment" << "totalmatch" << "validmatch" << "speeds" << "videopath";
	for (int i = 0; i < record.count(); ++i)
	{
		dbCols << record.fieldName(i);
	}
	if (refCols != dbCols)
	{//create a new table, this will drop the history records
		QSqlQuery query(db);
		query.exec("DROP TABLE wheels;");
		query.exec("CREATE TABLE wheels ( "
			"id         INTEGER         PRIMARY KEY AUTOINCREMENT,"
			"i_o        VARCHAR( 1 ),  "
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
	return true;
}