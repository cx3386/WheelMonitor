#pragma once

#include "ui_AlarmLCDBoard.h"
#include <QFrame>

class AlarmLCDBoard : public QFrame {
	Q_OBJECT

public:
	AlarmLCDBoard(QWidget* parent = Q_NULLPTR);
	~AlarmLCDBoard();
	QLineEdit* devName; //!< 显示内/外圈字段
	QLineEdit* devIndex; //!< 用作代理用途
	QLineEdit* alarmNum; //!< 显示报警号码字段
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Ui::AlarmLCDBoard ui;
	QWidget* masking;
signals:
	void clicked();
};
