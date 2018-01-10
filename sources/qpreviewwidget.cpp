#include "stdafx.h"
#include "qpreviewwidget.h"

QPreviewWidget::QPreviewWidget(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
}

QPreviewWidget::~QPreviewWidget()
{
}

void QPreviewWidget::paintEvent(QPaintEvent * event)
{
	Q_UNUSED(event);
	QPainter painter(this);

	painter.setRenderHint(QPainter::Antialiasing, true);
	// 设置画笔颜色、宽度
	painter.setPen(QPen(Qt::red, 2));
	// 设置画刷颜色
	//painter.setBrush(QColor(255, 160, 90));
	painter.drawRect(50, 50, 160, 100);
}