#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QSlider>

class CustomSlider : public QSlider
{
	Q_OBJECT
public:
	CustomSlider(QWidget *parent = nullptr)
		: QSlider(parent)
	{
	}
	CustomSlider(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR)
		: QSlider(orientation, parent)
	{
	}

protected:
	/** \brief override QSlider::mousePressEvent
	 */
	void mousePressEvent(QMouseEvent *ev) override;
signals:
	/** \brief A custom mouse-click signal for capturing and processing
	 * \param
	 * \return
	 */
	void costomSliderClicked(int);
};

#endif // CUSTOMSLIDER_H
