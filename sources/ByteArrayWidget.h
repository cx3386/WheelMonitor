#pragma once

#include <QWidget>
/*!
 * \class ByteArrayWidget
 *
 * \brief 此类继承自QWidget，自定义了一个名为data的property，用于存储QByteArray。专门用于接收QDataWidgetMapper::addMapping(?,data,property)的数据
 *
 * \author cx3386
 * \date 2019/03/18
 */
class ByteArrayWidget : public QWidget
{
	Q_OBJECT
		Q_PROPERTY(QByteArray data READ data WRITE setData NOTIFY dataChanged)

public:
	ByteArrayWidget(QWidget *parent);
	~ByteArrayWidget();
	void setData(const QByteArray& data);
	QByteArray data() const;

signals:
	void dataChanged(const QByteArray&);

private:
	QByteArray m_data;
};
