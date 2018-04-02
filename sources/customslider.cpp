#include "stdafx.h"
#include "customslider.h"

void CustomSlider::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) //�ж����
	{
		int dur = maximum() - minimum();
		int pos = minimum() + dur * ((double)ev->x() / width());
		if (pos != sliderPosition())
		{
			setValue(pos);
		} //�����Զ������굥���ź�
		emit costomSliderClicked(pos);
	}
	QSlider::mousePressEvent(ev); //ע��Ӧ�����ø��������������¼����������Բ�Ӱ���϶������
}