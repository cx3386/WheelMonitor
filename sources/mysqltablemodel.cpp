#include "stdafx.h"
#include "mysqltablemodel.h"
#include "playbackwidget.h"

MySqlTableModel::MySqlTableModel(QObject *parent /*= Q_NULLPTR*/, QSqlDatabase db /*= QSqlDatabase()*/) :QSqlTableModel(parent, db)
{
}

MySqlTableModel::~MySqlTableModel()
= default;

QVariant MySqlTableModel::data(const QModelIndex &item, int role /*= Qt::DisplayRole*/) const
{
	if (role == Qt::BackgroundColorRole)
	{
		int level = data(index(item.row(), PlayBackWidget::Wheels_AlarmLevel)).toInt();
		if (level == -1)
		{
			return QVariant(QColor(Qt::blue)); //miss test
		}
		if (level == 0)
		{
			return QVariant();
		}
		else if (level == 1)
		{
			return QVariant(QColor(Qt::yellow));
		}
		else if (level == 2)
		{
			return QVariant(QColor(Qt::red));
		}
		return QVariant();
	}
	else if (role == Qt::TextAlignmentRole)
	{
		return QVariant(Qt::AlignCenter);
	}
	return QSqlTableModel::data(item, role);
}