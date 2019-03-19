#include "stdafx.h"
#include "ByteArrayWidget.h"

ByteArrayWidget::ByteArrayWidget(QWidget *parent)
	: QWidget(parent)
{
}

ByteArrayWidget::~ByteArrayWidget()
{
}

void ByteArrayWidget::setData(const QByteArray& data)
{
	m_data = data;
	emit dataChanged(m_data);
}

QByteArray ByteArrayWidget::data() const
{
	return m_data;
}