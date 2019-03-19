/*!
 * \file QImageItemDele.h
 * \date 2019/03/18 11:08
 *
 * \author cx3386
 * Contact: cx3386@163.com
 * \par Copyright(c):
 * cx3386.
 * All Rights Reserved
 * \brief 将sql中存储的图片显示在表格中
 *
 * TODO: long description
 *
 * \note
 *
 * \version 1.0
 */
#pragma once

#include <QStyledItemDelegate>

class QImageItemDele : public QStyledItemDelegate
{
	Q_OBJECT

public:
	QImageItemDele(QObject *parent);
	~QImageItemDele();

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
