#pragma once

#include <QSqlTableModel>

class MySqlTableModel : public QSqlTableModel
{
	Q_OBJECT

public:
	MySqlTableModel(QObject *parent = Q_NULLPTR, QSqlDatabase db = QSqlDatabase());
	~MySqlTableModel();

	QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
};
