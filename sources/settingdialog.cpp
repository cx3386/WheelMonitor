#include "stdafx.h"
#include "settingdialog.h"
#include "confighelper.h"

SettingDialog::SettingDialog(ConfigHelper* ch, QWidget* parent /*= Q_NULLPTR*/)
	: helper(ch)
	, QDialog(parent)
{
	ui.setupUi(this);
	ini2ui();

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&]() {ui2ini(); qWarning() << "Option changed"; this->accept(); });
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, [&]() {helper->read(); this->reject(); });
	connect(ui.buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &SettingDialog::ini2ui);

	connect(ui.roiSpinBox_x, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_y, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_w, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_h, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_x_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_y_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_w_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_h_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));

	auto onChanged = [&]() {ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true); };
	auto onChanged1 = [&](const QString &) {ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true); };
	connect(ui.LaunchAtLoginCheckBox, &QCheckBox::stateChanged, this, onChanged);
	connect(ui.startAtLaunchCheckBox, &QCheckBox::stateChanged, this, onChanged);
	connect(ui.verboseLogCheckBox, &QCheckBox::stateChanged, this, onChanged);
	//dev1
	connect(ui.capIntervalSpinBox, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.sensorCheckBox, &QCheckBox::stateChanged, this, onChanged);
	connect(ui.alarmRatioSpinBox, static_cast<void (QDoubleSpinBox::*)(const QString &)>(&QDoubleSpinBox::valueChanged), this, onChanged1);
	connect(ui.warningRatioSpinBox, static_cast<void (QDoubleSpinBox::*)(const QString &)>(&QDoubleSpinBox::valueChanged), this, onChanged1);
	connect(ui.radiusMaxSpinBox, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.radiusMinSpinBox, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_x, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_y, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_w, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_h, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	//dev2
	connect(ui.capIntervalSpinBox_3, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.sensorCheckBox_3, &QCheckBox::stateChanged, this, onChanged);
	connect(ui.alarmRatioSpinBox_3, static_cast<void (QDoubleSpinBox::*)(const QString &)>(&QDoubleSpinBox::valueChanged), this, onChanged1);
	connect(ui.warningRatioSpinBox_3, static_cast<void (QDoubleSpinBox::*)(const QString &)>(&QDoubleSpinBox::valueChanged), this, onChanged1);
	connect(ui.radiusMaxSpinBox_3, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.radiusMinSpinBox_3, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_x_3, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_y_3, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_w_3, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);
	connect(ui.roiSpinBox_h_3, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this, onChanged1);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::ini2ui()
{
	helper->read();
	//common
	ui.LaunchAtLoginCheckBox->setChecked(helper->launchAtLogin);
	ui.startAtLaunchCheckBox->setChecked(helper->startAtLaunch);
	ui.verboseLogCheckBox->setChecked(helper->verboseLog);

	/*dev*/
	//dev1
	ui.capIntervalSpinBox->setValue(helper->device[0].camProfile.frameInterv);
	ui.sensorCheckBox->setChecked(helper->device[0].imProfile.sensorTriggered);
	ui.alarmRatioSpinBox->setValue(helper->device[0].imProfile.alarmRatio);
	ui.warningRatioSpinBox->setValue(helper->device[0].imProfile.warningRatio);
	ui.radiusMaxSpinBox->setValue(helper->device[0].imProfile.radius_max);
	ui.radiusMinSpinBox->setValue(helper->device[0].imProfile.radius_min);
	ui.roiSpinBox_x->setValue(helper->device[0].imProfile.roiRect.x);
	ui.roiSpinBox_y->setValue(helper->device[0].imProfile.roiRect.y);
	ui.roiSpinBox_w->setValue(helper->device[0].imProfile.roiRect.width);
	ui.roiSpinBox_h->setValue(helper->device[0].imProfile.roiRect.height);
	//dev2
	ui.capIntervalSpinBox_3->setValue(helper->device[1].camProfile.frameInterv);
	ui.sensorCheckBox_3->setChecked(helper->device[1].imProfile.sensorTriggered);
	ui.alarmRatioSpinBox_3->setValue(helper->device[1].imProfile.alarmRatio);
	ui.warningRatioSpinBox_3->setValue(helper->device[1].imProfile.warningRatio);
	ui.radiusMaxSpinBox_3->setValue(helper->device[1].imProfile.radius_max);
	ui.radiusMinSpinBox_3->setValue(helper->device[1].imProfile.radius_min);
	ui.roiSpinBox_x_3->setValue(helper->device[1].imProfile.roiRect.x);
	ui.roiSpinBox_y_3->setValue(helper->device[1].imProfile.roiRect.y);
	ui.roiSpinBox_w_3->setValue(helper->device[1].imProfile.roiRect.width);
	ui.roiSpinBox_h_3->setValue(helper->device[1].imProfile.roiRect.height);
}

void SettingDialog::ui2ini()
{
	//common
	helper->launchAtLogin = ui.LaunchAtLoginCheckBox->isChecked();
	helper->startAtLaunch = ui.startAtLaunchCheckBox->isChecked();
	helper->verboseLog = ui.verboseLogCheckBox->isChecked();

	/*dev*/
	//dev1
	helper->device[0].camProfile.frameInterv = ui.capIntervalSpinBox->value();
	helper->device[0].imProfile.sensorTriggered = ui.sensorCheckBox->isChecked();
	helper->device[0].imProfile.alarmRatio = ui.alarmRatioSpinBox->value();
	helper->device[0].imProfile.warningRatio = ui.warningRatioSpinBox->value();
	helper->device[0].imProfile.radius_max = ui.radiusMaxSpinBox->value();
	helper->device[0].imProfile.radius_min = ui.radiusMinSpinBox->value();
	helper->device[0].imProfile.roiRect.x = ui.roiSpinBox_x->value();
	helper->device[0].imProfile.roiRect.y = ui.roiSpinBox_y->value();
	helper->device[0].imProfile.roiRect.width = ui.roiSpinBox_w->value();
	helper->device[0].imProfile.roiRect.height = ui.roiSpinBox_h->value();
	//dev2
	helper->device[1].camProfile.frameInterv = ui.capIntervalSpinBox_3->value();
	helper->device[1].imProfile.sensorTriggered = ui.sensorCheckBox_3->isChecked();
	helper->device[1].imProfile.alarmRatio = ui.alarmRatioSpinBox_3->value();
	helper->device[1].imProfile.warningRatio = ui.warningRatioSpinBox_3->value();
	helper->device[1].imProfile.radius_max = ui.radiusMaxSpinBox_3->value();
	helper->device[1].imProfile.radius_min = ui.radiusMinSpinBox_3->value();
	helper->device[1].imProfile.roiRect.x = ui.roiSpinBox_x_3->value();
	helper->device[1].imProfile.roiRect.y = ui.roiSpinBox_y_3->value();
	helper->device[1].imProfile.roiRect.width = ui.roiSpinBox_w_3->value();
	helper->device[1].imProfile.roiRect.height = ui.roiSpinBox_h_3->value();

	helper->save();
}

void SettingDialog::roiSlot(int)
{
	helper->device[0].imProfile.roiRect = cv::Rect(ui.roiSpinBox_x->value(), ui.roiSpinBox_y->value(), ui.roiSpinBox_w->value(), ui.roiSpinBox_h->value());
	helper->device[1].imProfile.roiRect = cv::Rect(ui.roiSpinBox_x_3->value(), ui.roiSpinBox_y_3->value(), ui.roiSpinBox_w_3->value(), ui.roiSpinBox_h_3->value());
}