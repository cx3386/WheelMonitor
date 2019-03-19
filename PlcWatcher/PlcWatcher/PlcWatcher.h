#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PlcWatcher.h"

class PLC;
class PlcWatcher : public QMainWindow
{
	Q_OBJECT

public:
	PlcWatcher(QWidget *parent = Q_NULLPTR);
	~PlcWatcher();

private:
	Ui::PlcWatcherClass ui;
	PLC *plc;
	QThread* plcThread;

private slots:
	void uiShowCio0(int cio0);
	void startPlc();
	void stopPlc();
};
