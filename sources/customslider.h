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
	void mousePressEvent(QMouseEvent *ev); //��дQSlider��mousePressEvent�¼�
signals:
	void costomSliderClicked(int); //�Զ������굥���źţ����ڲ��񲢴���
};

#endif // CUSTOMSLIDER_H
