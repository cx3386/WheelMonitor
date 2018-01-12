#pragma once

#include <QWidget>
#include "plcserial.h"

class MySqlTableModel;
class Player;
class PlayBackWidget : public QWidget
{
	Q_OBJECT

public:
	PlayBackWidget(QWidget *parent = Q_NULLPTR);

	~PlayBackWidget();

	enum WheelsHeader
	{
		Wheels_ID = 0,
		Wheels_Num,
		Wheels_Time,
		Wheels_CalcSpeed,
		Wheels_RefSpeed,
		Wheels_Error,
		Wheels_AlarmLevel,
		Wheels_CheckState,
		Wheels_Fragment,
		Wheels_TotalMatch,
		Wheels_ValidMatch,
		Wheels_Speeds,
		Wheels_VideoPath,
	};
	enum CheckState
	{
		NoNeedCheck = 0,
		NeedCheck,
		Checked,
	};
	bool hasAlarm() const;

private:
	MySqlTableModel * model;
	QTableView* view;
	Player *player;
	void showAlarm();
	void showAll();

signals:
	void setAlarmLight(PLCSerial::AlarmColor alarmcolor);

	public slots:
	bool insertRecord(const QString &num, int alarmLevel, double absError, double refSpeed, int fragment, int totalMatch, int validMatch, const QString &savedSpeeds, const QString &videoPath);

	private slots :
	void readVideoPath(QModelIndex);
	void setSelectedChecked();
	void setAllChecked();
};
