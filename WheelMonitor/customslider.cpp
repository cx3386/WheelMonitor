#include "customslider.h"

void CustomSlider::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) //判断左键
    {
        int dur = maximum() - minimum();
        int pos = minimum() + dur * ((double)ev->x() / width());
        if (pos != sliderPosition()) {
            setValue(pos);
        } //发送自定义的鼠标单击信号
        emit costomSliderClicked(pos);
    }
    QSlider::mousePressEvent(ev); //注意应最后调用父类的鼠标点击处理事件，这样可以不影响拖动的情况
}
