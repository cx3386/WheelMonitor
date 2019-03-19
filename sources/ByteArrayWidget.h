#pragma once

#include <QWidget>
/*!
 * \class ByteArrayWidget
 *
 * \brief ����̳���QWidget���Զ�����һ����Ϊdata��property�����ڴ洢QByteArray��ר�����ڽ���QDataWidgetMapper::addMapping(?,data,property)������
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
