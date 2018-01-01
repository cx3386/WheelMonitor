#include "SettingDialog.h"
#include "HikVideoCapture.h"
#include "ImageProcess.h"
#include "MainWindow.h"
#include "stdafx.h"

cv::Rect SettingDialog::roiRectCache;

SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	roiRectCache = ImageProcess::g_imgParam.roiRect;
	/*settingsTab*/
	ui.appAutoRunCheckBox->setChecked(MainWindow::bAppAutoRun);
	ui.verboseLogCheckBox->setChecked(MainWindow::bVerboseLog);

	/*advancedTab*/
	ui.sensorCheckBox->setChecked(ImageProcess::g_imgParam.sensorTriggered);
	ui.alarmRatioSpinBox->setValue(ImageProcess::g_imgParam.alarmRatio);
	ui.warningRatioSpinBox->setValue(ImageProcess::g_imgParam.warningRatio);
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

void SettingDialog::roiSlot(int)
{
	ImageProcess::g_imgParam.roiRect = cv::Rect(ui.roiSpinBox_x->value(), ui.roiSpinBox_y->value(), ui.roiSpinBox_w->value(), ui.roiSpinBox_h->value());
	emit roiChanged();
}

void SettingDialog::on_okBtn_clicked()
{
	bool isChanged = false;
	static QMutex mutex;
	mutex.lock();
	/*settingsTab*/
	if (MainWindow::bAppAutoRun != ui.appAutoRunCheckBox->isChecked())
	{
		MainWindow::bAppAutoRun = ui.appAutoRunCheckBox->isChecked();
		isChanged = true;
	}

	if (MainWindow::bVerboseLog != ui.verboseLogCheckBox->isChecked())
	{
		MainWindow::bVerboseLog = ui.verboseLogCheckBox->isChecked();
		isChanged = true;
	}

	/*advancedTab*/
	if (ImageProcess::g_imgParam.sensorTriggered != ui.sensorCheckBox->isChecked())
	{
		ImageProcess::g_imgParam.sensorTriggered = ui.sensorCheckBox->isChecked();
		isChanged = true;
	}
	if (ImageProcess::g_imgParam.alarmRatio != ui.alarmRatioSpinBox->value())
	{
		ImageProcess::g_imgParam.alarmRatio = ui.alarmRatioSpinBox->value();
		isChanged = true;
	}

	if (ImageProcess::g_imgParam.warningRatio != ui.warningRatioSpinBox->value())
	{
		ImageProcess::g_imgParam.warningRatio = ui.warningRatioSpinBox->value();
		isChanged = true;
	}
	if (ImageProcess::g_imgParam.radius_max != ui.radiusMaxSpinBox->value())
	{
		ImageProcess::g_imgParam.radius_max = ui.radiusMaxSpinBox->value();
		isChanged = true;
	}
	if (ImageProcess::g_imgParam.radius_min != ui.radiusMinSpinBox->value())
	{
		ImageProcess::g_imgParam.radius_min = ui.radiusMinSpinBox->value();
		isChanged = true;
	}
	if (HikVideoCapture::capInterval != ui.capIntervalSpinBox->value())
	{
		HikVideoCapture::capInterval = ui.capIntervalSpinBox->value();
		ImageProcess::g_imgParam.angle2Speed = 60 * (M_PI * 0.650 / 360) / ((HikVideoCapture::capInterval + 1) / 25.0);
		isChanged = true;
	}
	if (ImageProcess::g_imgParam.roiRect != roiRectCache)
	{
		isChanged = true;
	}
	mutex.unlock();
	if (isChanged)
	{
		qWarning("SettingDialog: Option changed");
	}
}

void SettingDialog::on_cancelBtn_clicked()
{
	ImageProcess::g_imgParam.roiRect = roiRectCache;
	ui.roiSpinBox_x->setValue(ImageProcess::g_imgParam.roiRect.x);
	ui.roiSpinBox_y->setValue(ImageProcess::g_imgParam.roiRect.y);
	ui.roiSpinBox_w->setValue(ImageProcess::g_imgParam.roiRect.width);
	ui.roiSpinBox_h->setValue(ImageProcess::g_imgParam.roiRect.height);
}