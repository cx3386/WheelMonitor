#pragma once

#include "Plc.h"
#include <QWidget>

class MySqlTableModel;
class Player;
class QImageItemDele;

class PlayBackWidget : public QWidget {
	Q_OBJECT
public:
	PlayBackWidget(QWidget* parent = Q_NULLPTR);
	~PlayBackWidget();
	MySqlTableModel* allModel;    //!<  tableModel for all record
	MySqlTableModel* alarmModel;  //!<  talbleModel for unchecked record

	/**
	 * \brief Returns true if the alarm list is not null;otherwise returns false.
	 */
	bool hasAlarm() const;
	QTableView* alarmView; //!< tablseview for unchecked record
	QComboBox* selectTableCb;

private:
	QTableView* allView; ///< tableview for all record
	Player* player;

	/**
	 * \brief initialize the alarm(unchecked) table view
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
	//最好不要共享同一实例
	QImageItemDele* delegate_alarm;
	QImageItemDele* delegate_all;

signals:
	void clearAlarm();

public slots:
	void clearMedia(); //!< 清除媒体信息；应当在失焦时调用
	void dbChanged();

private slots:
	void readVideoPath(QAbstractItemModel *model, QModelIndex index) const;
	void setSelectedChecked();
	void setAllChecked();
};
