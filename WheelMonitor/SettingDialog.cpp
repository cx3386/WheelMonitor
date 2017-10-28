#include "stdafx.h"
#include "SettingDialog.h"
#include "ImageProcess.h"
#include "HikVideoCapture.h"

SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.sensorCheckBox->setChecked(ImageProcess::sensorTriggered);
	ui.angleHighSpinBox->setValue(ImageProcess::angleBigRatio);
	ui.angleLowSpinBox->setValue(ImageProcess::angleSmallRatio);
	ui.radiusMaxSpinBox->setValue(ImageProcess::radius_max);
	ui.radiusMinSpinBox->setValue(ImageProcess::radius_min);
	ui.capIntervalSpinBox->setValue(HikVideoCapture::capInterval);
	ui.roiSpinBox_x->setValue(ImageProcess::roiRect.x);
	ui.roiSpinBox_y->setValue(ImageProcess::roiRect.y);
	ui.roiSpinBox_w->setValue(ImageProcess::roiRect.width);
	ui.roiSpinBox_h->setValue(ImageProcess::roiRect.height);
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::setOption()
{
	static QMutex mutex;
	mutex.lock();
	ImageProcess::sensorTriggered = ui.sensorCheckBox->isChecked();
	ImageProcess::angleBigRatio = ui.angleHighSpinBox->value();
	ImageProcess::angleSmallRatio = ui.angleLowSpinBox->value();
	ImageProcess::radius_max = ui.radiusMaxSpinBox->value();
	ImageProcess::radius_min = ui.radiusMinSpinBox->value();
	ImageProcess::roiRect = cv::Rect(ui.roiSpinBox_x->value(), ui.roiSpinBox_y->value(), ui.roiSpinBox_w->value(), ui.roiSpinBox_h->value());
	HikVideoCapture::capInterval = ui.capIntervalSpinBox->value();
	ImageProcess::angle2Speed = 60 * (M_PI * 0.650 / 360) / ((HikVideoCapture::capInterval + 1) / 25.0);
	mutex.unlock();
	qWarning("Option changed");
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