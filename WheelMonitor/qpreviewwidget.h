#pragma once

#include <QWidget>

class QPreviewWidget : public QWidget
{
	Q_OBJECT

public:
	QPreviewWidget(QWidget *parent = Q_NULLPTR);
	~QPreviewWidget();
	void paintEvent(QPaintEvent *event);
};
