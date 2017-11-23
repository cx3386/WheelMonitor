#include "stdafx.h"
#include "mywidget.h"

MyWidget::MyWidget(QWidget *parent)
	: QWidget(parent)
{
}

MyWidget::~MyWidget()
{
}
void MyWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	emit myResize();
}