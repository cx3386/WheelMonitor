#include "stdafx.h"
#include "SettingDialog.h"
#include "ImageProcess.h"
#include "HikVideoCapture.h"

SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.sensorCheckBox->setChecked(ImageProcess::sensorTriggered);
	ui.angleHighSpinBox->setValue(ImageProcess::angleHighThreshold);
	ui.angleLowSpinBox->setValue(ImageProcess::angleLowThreshold);
	ui.radiusMaxSpinBox->setValue(ImageProcess::radius_max);
	ui.radiusMinSpinBox->setValue(ImageProcess::radius_min);
	ui.capIntervalSpinBox->setValue(HikVideoCapture::capInterval);
}

SettingDialog::~SettingDialog()
{

}

void SettingDialog::setOption()
{
	static QMutex mutex;
	mutex.lock();
	ImageProcess::sensorTriggered = ui.sensorCheckBox->isChecked();
	ImageProcess::angleHighThreshold = ui.angleHighSpinBox->value();
	ImageProcess::angleLowThreshold = ui.angleLowSpinBox->value();
	ImageProcess::radius_max = ui.radiusMaxSpinBox->value();
	ImageProcess::radius_min = ui.radiusMinSpinBox->value();
	HikVideoCapture::capInterval = ui.capIntervalSpinBox->value();
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
