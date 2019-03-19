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
	//����һ��ԭ����painter���ڲ������ָ�������Ӱ��
	painter->save();
	//ȡ������
	QByteArray ba = index.model()->data(index, Qt::DisplayRole).toByteArray();
	QImage im;
	im.loadFromData(ba, "JPG");
	painter->drawImage(option.rect, im);
	painter->restore();
}