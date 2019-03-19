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

//! ��¼�ó��ֽ�������ز�������д�����ݿ�
struct WheelDbInfo {
	int id;
	int i_o;
	QString num;
	QImage plate;
	double calcspeed;
	double refspeed;
	double error;
	QString time;
	int alarmlevel; //!< -2���������ν�������ţ�-1�����ν�������ţ�0��������1��Ԥ����2������
	int checkstate;
	int ocrsize;
	int fragment;
	//int interrupts; //!< ��¼���ֻغ��ڣ�����ж��˼��Ρ�0-���� >1-������
	int totalmatch;
	int validmatch;
	QString speeds;
	QString videopath;
	//��ʱ����
	double warnRatio;
	double alarmRatio;
};

/// the value of field "wheels_checkstate", represent whether need check. if need check, it will show in the alarm table
enum CheckState {
	NoNeedCheck = 0,
	NeedCheck,
	Checked,
};

//! �����̵߳��ã�������ΪĬ��
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

	// ���user��
	model.setTable("user");
	model.select();
	if (!model.rowCount()) {
		query.exec("CREATE TABLE IF NOT EXISTS user (id integer primary key autoincrement, username varchar(20) unique, pwd varchar(20))");
		query.exec("insert into user (username, pwd) values('BaoSteel', '123456')");
	}

	//���devs��
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

	//���wheels��
	model.setTable("wheels");
	model.select();
	auto record = model.record();
	bool createNewTable = false;

	//������ݱ�������Ƿ���ȷ
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
		//�����������޷��ɹ�ɾ����bug��debug�汾����ɾ����release�޷�ɾ����
		query.exec("PRAGMA foreign_keys = OFF;");
		query.exec("DROP TABLE wheels;");
		query.exec("PRAGMA foreign_keys = ON;");
		//����Ϊsqlite3ר�����-2018��11��12�ճ���
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
		//�������
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

//! ������ͼ�����̷ֱ߳��ʼ��
//! ȡ����ȫ�������߳̿���
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
