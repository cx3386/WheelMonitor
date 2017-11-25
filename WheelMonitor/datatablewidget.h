#pragma once

#include <QWidget>

class QSqlTableModel;
class DataTableWidget : public QWidget
{
	Q_OBJECT

public:
	DataTableWidget(QWidget *parent = Q_NULLPTR);
	~DataTableWidget();
	QSqlTableModel *model;

private:

	private slots :
		void readVideoPath(QModelIndex);

signals:
	void showVideo(const QUrl &);
};
