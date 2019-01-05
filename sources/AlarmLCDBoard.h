#pragma once

#include "ui_AlarmLCDBoard.h"
#include <QFrame>

class AlarmLCDBoard : public QFrame {
	Q_OBJECT

public:
	AlarmLCDBoard(QWidget* parent = Q_NULLPTR);
	~AlarmLCDBoard();
	QLineEdit* devName; //!< ��ʾ��/��Ȧ�ֶ�
	QLineEdit* devIndex; //!< ����������;
	QLineEdit* alarmNum; //!< ��ʾ���������ֶ�
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Ui::AlarmLCDBoard ui;
	QWidget* masking;
signals:
	void clicked();
};
