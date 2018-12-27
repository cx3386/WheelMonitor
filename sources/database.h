#pragma once

#include "common.h"
#include <QtSql>

/// field index of table "wheels"
enum WheelHeader {
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

//! ��¼�ó��ֽ�������ز�������д�����ݿ�
struct WheelDbInfo {
	int id;
	int i_o;
	QString num;
	double calcspeed;
	double refspeed;
	double error;
	QString time;
	int alarmlevel;
	int checkstate;
	int ocrsize;
	int fragment;
	//int interrupts; //!< ��¼���ֻغ��ڣ�����ж��˼��Ρ�0-���� >1-������
	int totalmatch;
	int validmatch;
	QString speeds;
	QString videopath;
};

/// the value of field "wheels_checkstate", represent whether need check. if need check, it will show in the alarm table
enum CheckState {
	NoNeedCheck = 0,
	NeedCheck,
	Checked,
};

//! �����̵߳���
static bool initMainDb()
{
	if (!QSqlDatabase::drivers().contains("QSQLITE")) {
		qWarning() << "database: no SQLITE driver.";
		QMessageBox::critical(Q_NULLPTR, "Unable to load database", "This app needs the SQLITE driver");
	}
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", MAIN_CONNECTION_NAME);
	db.setDatabaseName(databaseFilePath);
	if (!db.open()) {
		qWarning() << "database: initMainDb failed. " << db.lastError().text();
		QMessageBox::critical(Q_NULLPTR, "Unable to initialize Database",
			"Eror initializing database: " + db.lastError().text());
		return false;
	}
	QSqlQuery query(db);
	QSqlTableModel model(nullptr, db);
	// ���user��
	model.setTable("user");
	model.select();
	// if no user table, insert the default user;
	if (!model.rowCount()) {
		query.exec("create table user (id integer primary key autoincrement, username varchar(20) unique, pwd varchar(20))");
		query.exec("insert into user (username, pwd) values('BaoSteel', '123456')");
	}
	//���iner
	model.setTable("devs");
	model.select();
	if (!model.rowCount()) {
		query.exec("CREATE TABLE devs ("
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
	return true;
}

//! ������ͼ�����̷ֱ߳��ʼ��
static bool initThreadDb(int deviceIndex)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", deviceIndex ? THREAD1_CONNECTION_NAME : THREAD0_CONNECTION_NAME);
	db.setDatabaseName(databaseFilePath);
	if (!db.open()) {
		qWarning() << "database: initThreadDb failed. " << db.lastError().text();
		QMessageBox::critical(Q_NULLPTR, "Unable to initialize Database",
			"Error initializing database: " + db.lastError().text());
		return false;
	}
	QSqlTableModel model(nullptr, db);
	model.setTable("wheels");
	model.select();
	auto record = model.record();
	bool createNewTable = false;
	if (record.isEmpty())
		createNewTable = true;
	else {
		//������ݱ�ṹ�Ƿ���ȷ
		QStringList refCols, dbCols;
		refCols << "id"
			<< "i_o"
			<< "num"
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
		createNewTable = refCols != dbCols;
	}
	if (createNewTable) { //create a new table, this will drop the history records
		QSqlQuery query(db);
		query.exec("DROP TABLE wheels;");
		query.exec("PRAGMA foreign_keys = ON;");
		query.exec("CREATE TABLE wheels ( "
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"i_o INTEGER REFERENCES devs (devIndex),"
			"num TEXT,"
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
			");"); //����Ϊsqlite3ר�����-2018��11��12�ճ���
 //Ϊ��������������
		query.exec("CREATE INDEX idx_wheels ON wheels ( "
			"i_o ASC,"
			"num ASC"
			");");
	}
	return true;
}