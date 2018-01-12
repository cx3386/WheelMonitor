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
		switch (level) //alertlevel standard is red, orange, yellow, blue(decrease order)
		{
		case -2:return QVariant(QColor(Qt::yellow));
		case -1:return QVariant(QColor(0, 112, 192));  //blue
		case 0:return QVariant(QColor(0, 176, 80));  //green
		case 1:return QVariant(QColor(255, 97, 0));  //orange
		case 2:return QVariant(QColor(Qt::red));
		default:return QVariant();
		}
	}
	else if (role == Qt::TextAlignmentRole)
	{
		return QVariant(Qt::AlignCenter);
	}
	return QSqlTableModel::data(item, role);
}