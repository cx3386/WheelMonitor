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

	/**
	 * \brief Returns true if the alarm list is not null;otherwise returns false.
	 *
	 * \sa alarmList()
	 *
	 */
	bool hasAlarm() const;

	/**
	 * \brief Returns the alarmList querying database
	 *
	 * \sa hasAlarm()
	 */
private:
	MySqlTableModel * allModel;  ///< tableModel for all record
	QTableView* allView;  ///< tableview for all record
	MySqlTableModel* alarmModel;  ///< talbleModel for unchecked record
	QTableView* alarmView;  ///< tableview for unchecked record
	Player *player;

	/**
	 * \brief initialize the alram(unchecked) table view
	 *
	 */
	void initAlarmTable();
	/**
	 * \brief initialize the all-record table view
	 *
	 */
	void initAllTable();
	QPushButton* checkSelBtn;
	QPushButton* checkAllBtn;
signals:
	void setAlarmLight(AlarmColor alarmcolor);

	public slots:
	void clearMedia(int index);
	void dbChanged();

	private slots :
	void readVideoPath(QModelIndex) const;
	void setSelectedChecked();
	void setAllChecked();
};
