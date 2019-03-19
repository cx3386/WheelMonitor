#include "stdafx.h"
#include "PlcWatcher.h"
#include "PLC.h"

PlcWatcher::PlcWatcher(QWidget *parent)
	: QMainWindow(parent)
	, plc(new PLC)
	, plcThread(new QThread(this))

{
	ui.setupUi(this);
	plc->moveToThread(plcThread);
	connect(plc, &PLC::connectErr, this, [=](int code) {
		QMessageBox::warning(this, "FUCK!", QString("Can't open COM3, error code %1").arg(code), QMessageBox::Ok, QMessageBox::Ok);
	}); //��ʾcio0
	connect(plc, &PLC::cio0Update, this, &PlcWatcher::uiShowCio0); //��ʾcio0
	plcThread->start();
	plc->connect();
}

PlcWatcher::~PlcWatcher()
{
	plcThread->quit();
	plcThread->wait();
	plc->deleteLater();//���̵߳����ˣ�����Ŷ
}

void PlcWatcher::uiShowCio0(int cio0)
{
	cio0 = cio0 >> 1;
	ui.btn1->setChecked(cio0 & 1);
	cio0 = cio0 >> 1;
	ui.btn2->setChecked(cio0 & 1);
	cio0 = cio0 >> 1;
	ui.btn3->setChecked(cio0 & 1);
	cio0 = cio0 >> 1;
	ui.btn4->setChecked(cio0 & 1);
	cio0 = cio0 >> 1;
	ui.btn5->setChecked(cio0 & 1);
	cio0 = cio0 >> 1;
	ui.btn6->setChecked(cio0 & 1);
	cio0 = cio0 >> 1;
	ui.btn7->setChecked(cio0 & 1);
	cio0 = cio0 >> 1;
	ui.btn8->setChecked(cio0 & 1);
}

void PlcWatcher::startPlc()
{
	plc->start();
	ui.statusBar->showMessage(QStringLiteral("��ʼ���"));
}

void PlcWatcher::stopPlc()
{
	plc->stop();
	ui.statusBar->showMessage(QStringLiteral("ֹͣ���"));
}