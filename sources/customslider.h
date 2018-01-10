#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QCoreApplication>
#include <QMouseEvent>
#include <QSlider>

class CustomSlider : public QSlider
{
	Q_OBJECT
public:
	CustomSlider(QWidget *parent = 0)
		: QSlider(parent)
	{
	}
	CustomSlider(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR)
		: QSlider(orientation, parent)
	{
	}

protected:
	void mousePressEvent(QMouseEvent *ev); //重写QSlider的mousePressEvent事件
signals:
	void costomSliderClicked(int); //自定义的鼠标单击信号，用于捕获并处理
};

#endif // CUSTOMSLIDER_H
