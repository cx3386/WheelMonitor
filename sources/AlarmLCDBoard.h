#pragma once

#include "ui_AlarmLCDBoard.h"
#include <QFrame>

class ByteArrayWidget;
class AlarmLCDBoard : public QFrame {
	Q_OBJECT

public:
	AlarmLCDBoard(QWidget* parent = Q_NULLPTR);
	~AlarmLCDBoard();
	QLineEdit* devIndex; //!< devId����
	ByteArrayWidget* plate_ba;	//!< plate����
	QLineEdit* alarmNum; //!< ��ʾ���������ֶ�

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Ui::AlarmLCDBoard ui;
	QWidget* masking;
	QLineEdit* devName; //!< ��ʾ��/��Ȧ�ֶ�
	QLabel* plate;  //!< ��ʾ����

signals:
	void clicked();
};
