#pragma once

#include <QWidget>

class QSqlTableModel;
class DataTableWidget : public QWidget
{
	Q_OBJECT

public:
	DataTableWidget(QWidget *parent = Q_NULLPTR);
	~DataTableWidget();
	bool insertRecord(QString num, QString time, int fragment, int totalmatch, int validmatch, double error, double cartspeed, QString speeds, bool alarm, bool checked, QString videopath);

private:
	QSqlTableModel *model;

	public slots:
	//void selectTableView();

	private slots :
		void readVideoPath(QModelIndex);

signals:
	void showVideo(const QUrl &);
};
