#pragma once

#include "common.h"
#include "ocr.h"
#include <QtSql>

/// field index of table "wheels"
enum WheelHeader {
	Wheel_ID = 0,
	Wheel_I_O,
	Wheel_Num,
	Wheel_Plate,
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

//! 记录该车轮结算后的相关参数，以写入数据库
struct WheelDbInfo {
	int id;
	int i_o;
	QString num;
	QImage plate;
	double calcspeed;
	double refspeed;
	double error;
	QString time;
	int alarmlevel; //!< -2：连续两次结果不可信；-1：本次结果不可信；0：正常；1：预警；2：报警
	int checkstate;
	int ocrsize;
	int fragment;
	//int interrupts; //!< 记录车轮回合内，检测中断了几次。0-正常 >1-不连续
	int totalmatch;
	int validmatch;
	QString speeds;
	QString videopath;
	//临时增加
	double warnRatio;
	double alarmRatio;
};

/// the value of field "wheels_checkstate", represent whether need check. if need check, it will show in the alarm table
enum CheckState {
	NoNeedCheck = 0,
	NeedCheck,
	Checked,
};

//! 由主线程调用，连接名为默认
static bool initMainDb()
{
	if (!QSqlDatabase::drivers().contains("QSQLITE")) {
		qWarning() << "database: no SQLITE driver.";
		QMessageBox::critical(Q_NULLPTR, "Unable to load database", "This app needs the SQLITE driver");
	}
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(databaseFilePath);
	if (!db.open()) {
		qWarning() << "database: initMainDb failed. " << db.lastError().text();
		QMessageBox::critical(Q_NULLPTR, "Unable to initialize Database",
			"Eror initializing database: " + db.lastError().text());
		return false;
	}
	QSqlQuery query;
	QSqlTableModel model;

	// 检查user表
	model.setTable("user");
	model.select();
	if (!model.rowCount()) {
		query.exec("CREATE TABLE IF NOT EXISTS user (id integer primary key autoincrement, username varchar(20) unique, pwd varchar(20))");
		query.exec("insert into user (username, pwd) values('BaoSteel', '123456')");
	}

	//检查devs表
	model.setTable("devs");
	model.select();
	if (!model.rowCount()) {
		query.exec("CREATE TABLE IF NOT EXISTS devs ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"devIndex INTEGER UNIQUE,"
			"name TEXT"
			");");
		query.prepare("INSERT INTO devs (devIndex,name) VALUES(:devId,:devname);");
		query.bindValue(":devId", 0);
		query.bindValue(":devname", getDeviceMark(0));
		query.exec();
		query.prepare("INSERT INTO devs (devIndex,name) VALUES(:devId,:devname);");
		query.bindValue(":devId", 1);
		query.bindValue(":devname", getDeviceMark(1));
		query.exec();
	}

	//检查wheels表
	model.setTable("wheels");
	model.select();
	auto record = model.record();
	bool createNewTable = false;

	//检查数据表的列名是否正确
	QStringList refCols, dbCols;
	refCols << "id"
		<< "i_o"
		<< "num"
		<< "plate"
		<< "calcspeed"
		<< "refspeed"
		<< "error"
		<< "time"
		<< "alarmlevel"
		<< "checkstate"
		<< "ocrsize"
		<< "fragment"
		<< "totalmatch"
		<< "validmatch"
		<< "speeds"
		<< "videopath";
	for (int i = 0; i < record.count(); ++i) {
		dbCols << record.fieldName(i);
	}
	if (refCols != dbCols) {
		//create a new table, this will drop the history records
		QSqlQuery query(db);
		//以下语句存在无法成功删除的bug。debug版本可以删除，release无法删除。
		query.exec("PRAGMA foreign_keys = OFF;");
		query.exec("DROP TABLE wheels;");
		query.exec("PRAGMA foreign_keys = ON;");
		//更改为sqlite3专有语句-2018年11月12日陈翔
		query.exec("CREATE TABLE wheels ( "
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"i_o INTEGER REFERENCES devs (devIndex),"
			"num TEXT,"
			"plate BLOB,"
			"calcspeed REAL,"
			"refspeed REAL,"
			"error REAL,"
			"time NUMERIC,"
			"alarmlevel INTEGER,"
			"checkstate INTEGER,"
			"ocrsize INTEGER,"
			"fragment INTEGER,"
			"totalmatch INTEGER,"
			"validmatch INTEGER,"
			"speeds TEXT,"
			"videopath TEXT"
			");");
		//添加索引
		query.exec("CREATE INDEX idx_num_id ON wheels ( "
			"num ASC,"
			"id DESC"
			");");
		//query.exec("CREATE INDEX idx_io ON wheels ( "
		//	"i_o"
		//	");");
		query.exec("CREATE INDEX idx_checkstate ON wheels ( "
			"checkstate"
			");");
	}
	return true;
}

//! 由两个图像处理线程分别初始化
//! 取消：全部由主线程控制
//static bool initThreadDb(int deviceIndex)
//{
//	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", deviceIndex ? THREAD1_CONNECTION_NAME : THREAD0_CONNECTION_NAME);
//	db.setDatabaseName(databaseFilePath);
//	if (!db.open()) {
//		qWarning() << "database: initThreadDb failed. " << db.lastError().text();
//		QMessageBox::critical(Q_NULLPTR, "Unable to initialize Database",
//			"Error initializing database: " + db.lastError().text());
//		return false;
//	}
//	QSqlTableModel model(nullptr, db);
//
//	return true;
//}
