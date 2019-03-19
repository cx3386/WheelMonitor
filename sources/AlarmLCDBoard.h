#pragma once

#include "ui_AlarmLCDBoard.h"
#include <QFrame>

class ByteArrayWidget;
class AlarmLCDBoard : public QFrame {
	Q_OBJECT

public:
	AlarmLCDBoard(QWidget* parent = Q_NULLPTR);
	~AlarmLCDBoard();
	QLineEdit* devIndex; //!< devId代理
	ByteArrayWidget* plate_ba;	//!< plate代理
	QLineEdit* alarmNum; //!< 显示报警号码字段

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Ui::AlarmLCDBoard ui;
	QWidget* masking;
	QLineEdit* devName; //!< 显示内/外圈字段
	QLabel* plate;  //!< 显示车牌

signals:
	void clicked();
};
