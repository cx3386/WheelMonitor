#include "stdafx.h"
#include "QImageItemDele.h"
#include "database.h"

QImageItemDele::QImageItemDele(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

QImageItemDele::~QImageItemDele()
{
}

void QImageItemDele::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	//保存一下原本的painter便于操作完后恢复，减少影响
	painter->save();
	//取出数据
	QByteArray ba = index.model()->data(index, Qt::DisplayRole).toByteArray();
	QImage im;
	im.loadFromData(ba, "JPG");
	painter->drawImage(option.rect, im);
	painter->restore();
}