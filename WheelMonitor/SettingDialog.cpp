#include "stdafx.h"
#include "SettingDialog.h"
#include "ImageProcess.h"
#include "HikVideoCapture.h"
#include "MainWindow.h"

SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	/*settingsTab*/
	ui.appAutoRunCheckBox->setChecked(MainWindow::bAppAutoRun);
	ui.verboseLogCheckBox->setChecked(MainWindow::bVerboseLog);

	/*advancedTab*/
	ui.sensorCheckBox->setChecked(ImageProcess::g_imgParam.sensorTriggered);
	ui.angleHighSpinBox->setValue(ImageProcess::g_imgParam.angleBigRatio);
	ui.angleLowSpinBox->setValue(ImageProcess::g_imgParam.angleSmallRatio);
	ui.radiusMaxSpinBox->setValue(ImageProcess::g_imgParam.radius_max);
	ui.radiusMinSpinBox->setValue(ImageProcess::g_imgParam.radius_min);
	ui.capIntervalSpinBox->setValue(HikVideoCapture::capInterval);
	ui.roiSpinBox_x->setValue(ImageProcess::g_imgParam.roiRect.x);
	ui.roiSpinBox_y->setValue(ImageProcess::g_imgParam.roiRect.y);
	ui.roiSpinBox_w->setValue(ImageProcess::g_imgParam.roiRect.width);
	ui.roiSpinBox_h->setValue(ImageProcess::g_imgParam.roiRect.height);
	connect(ui.roiSpinBox_x, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_y, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_w, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_h, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::setOption()
{
	static QMutex mutex;
	mutex.lock();
	/*settingsTab*/
	MainWindow::bAppAutoRun = ui.appAutoRunCheckBox->isChecked();
	MainWindow::bVerboseLog = ui.verboseLogCheckBox->isChecked();

	/*advancedTab*/
	ImageProcess::g_imgParam.sensorTriggered = ui.sensorCheckBox->isChecked();
	ImageProcess::g_imgParam.angleBigRatio = ui.angleHighSpinBox->value();
	ImageProcess::g_imgParam.angleSmallRatio = ui.angleLowSpinBox->value();
	ImageProcess::g_imgParam.radius_max = ui.radiusMaxSpinBox->value();
	ImageProcess::g_imgParam.radius_min = ui.radiusMinSpinBox->value();
	HikVideoCapture::capInterval = ui.capIntervalSpinBox->value();
	ImageProcess::g_imgParam.angle2Speed = 60 * (M_PI * 0.650 / 360) / ((HikVideoCapture::capInterval + 1) / 25.0);
	mutex.unlock();
	qWarning("Option changed");
}

void SettingDialog::roiSlot(int)
{
	ImageProcess::g_imgParam.roiRect = cv::Rect(ui.roiSpinBox_x->value(), ui.roiSpinBox_y->value(), ui.roiSpinBox_w->value(), ui.roiSpinBox_h->value());
	emit roiChanged();
}

//void settingdialog::closeevent(qcloseevent * event)
//{
//	if (maybesave()
//	{
//		event->accept();
//	}
//	else
//	{
//		event->ignore();
//	}
//}