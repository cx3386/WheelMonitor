#pragma once

#include <QWidget>

class QSqlTableModel;
class Player;
class FaultPlayBackWidget : public QWidget
{
	Q_OBJECT

public:
	FaultPlayBackWidget(QWidget *parent = Q_NULLPTR);

	~FaultPlayBackWidget();
	bool insertRecord(QString num, int fragment, int totalmatch, int validmatch, int alarmLevel, double error, double refspeed, QString speeds, QString videopath);
	bool isLastAlarm(const QString & num) const;

private:
	QSqlTableModel * model;
	QTableView* view;
	Player *player;
	void showAlarm();
	void showAll();
	enum WheelsHeader
	{
		Wheels_ID = 0,
		Wheels_Num,
		Wheels_Time,
		Wheels_CalcSpeed,
		Wheels_RefSpeed,
		Wheels_Error,
		Wheels_AlarmLevel,
		Wheels_Checked,
		Wheels_Fragment,
		Wheels_TotalMatch,
		Wheels_ValidMatch,
		Wheels_Speeds,
		Wheels_VideoPath
	};

	public slots:
	//void selectTableView();

	private slots :
		void readVideoPath(QModelIndex);
	void setChecked();
};
