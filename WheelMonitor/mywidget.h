#pragma once

#include <QWidget>

class MyWidget : public QWidget
{
	Q_OBJECT

public:
	MyWidget(QWidget *parent = Q_NULLPTR);
	~MyWidget();

	protected slots:
	void resizeEvent(QResizeEvent *event);
signals:
	void myResize();
};
