#include "stdafx.h"
#include "settingdialog.h"
#include "confighelper.h"

SettingDialog::SettingDialog(ConfigHelper& ch, QWidget* parent /*= Q_NULLPTR*/)
	: helper(&ch)
	, QDialog(parent)
{
	ui.setupUi(this);
	helperToUi();

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&]() {uiToHelper(); helper->save(); qWarning() << "Option changed"; this->accept(); });
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, [&]() {helper->read(); this->reject(); });
	connect(ui.buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, [&]() {helper->read(); helperToUi(); });

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

void SettingDialog::helperToUi()
{
	//common
	ui.LaunchAtLoginCheckBox->setChecked(helper->launchAtLogin);
	ui.startAtLaunchCheckBox->setChecked(helper->startAtLaunch);
	ui.verboseLogCheckBox->setChecked(helper->verboseLog);

	/*dev*/
	//dev1
	ui.capIntervalSpinBox->setValue(helper->dev[0].camProf.frameInterv);
	ui.sensorCheckBox->setChecked(helper->dev[0].imProf.sensorTriggered);
	ui.alarmRatioSpinBox->setValue(helper->dev[0].imProf.alarmRatio);
	ui.warningRatioSpinBox->setValue(helper->dev[0].imProf.warningRatio);
	ui.radiusMaxSpinBox->setValue(helper->dev[0].imProf.radius_max);
	ui.radiusMinSpinBox->setValue(helper->dev[0].imProf.radius_min);
	ui.roiSpinBox_x->setValue(helper->dev[0].imProf.roiRect.x);
	ui.roiSpinBox_y->setValue(helper->dev[0].imProf.roiRect.y);
	ui.roiSpinBox_w->setValue(helper->dev[0].imProf.roiRect.width);
	ui.roiSpinBox_h->setValue(helper->dev[0].imProf.roiRect.height);
	//dev2
	ui.capIntervalSpinBox_3->setValue(helper->dev[1].camProf.frameInterv);
	ui.sensorCheckBox_3->setChecked(helper->dev[1].imProf.sensorTriggered);
	ui.alarmRatioSpinBox_3->setValue(helper->dev[1].imProf.alarmRatio);
	ui.warningRatioSpinBox_3->setValue(helper->dev[1].imProf.warningRatio);
	ui.radiusMaxSpinBox_3->setValue(helper->dev[1].imProf.radius_max);
	ui.radiusMinSpinBox_3->setValue(helper->dev[1].imProf.radius_min);
	ui.roiSpinBox_x_3->setValue(helper->dev[1].imProf.roiRect.x);
	ui.roiSpinBox_y_3->setValue(helper->dev[1].imProf.roiRect.y);
	ui.roiSpinBox_w_3->setValue(helper->dev[1].imProf.roiRect.width);
	ui.roiSpinBox_h_3->setValue(helper->dev[1].imProf.roiRect.height);
}

void SettingDialog::uiToHelper()
{
	//common
	helper->launchAtLogin = ui.LaunchAtLoginCheckBox->isChecked();
	helper->startAtLaunch = ui.startAtLaunchCheckBox->isChecked();
	helper->verboseLog = ui.verboseLogCheckBox->isChecked();

	/*dev*/
	//dev1
	helper->dev[0].camProf.frameInterv = ui.capIntervalSpinBox->value();
	helper->dev[0].imProf.sensorTriggered = ui.sensorCheckBox->isChecked();
	helper->dev[0].imProf.alarmRatio = ui.alarmRatioSpinBox->value();
	helper->dev[0].imProf.warningRatio = ui.warningRatioSpinBox->value();
	helper->dev[0].imProf.radius_max = ui.radiusMaxSpinBox->value();
	helper->dev[0].imProf.radius_min = ui.radiusMinSpinBox->value();
	helper->dev[0].imProf.roiRect.x = ui.roiSpinBox_x->value();
	helper->dev[0].imProf.roiRect.y = ui.roiSpinBox_y->value();
	helper->dev[0].imProf.roiRect.width = ui.roiSpinBox_w->value();
	helper->dev[0].imProf.roiRect.height = ui.roiSpinBox_h->value();
	//dev2
	helper->dev[1].camProf.frameInterv = ui.capIntervalSpinBox_3->value();
	helper->dev[1].imProf.sensorTriggered = ui.sensorCheckBox_3->isChecked();
	helper->dev[1].imProf.alarmRatio = ui.alarmRatioSpinBox_3->value();
	helper->dev[1].imProf.warningRatio = ui.warningRatioSpinBox_3->value();
	helper->dev[1].imProf.radius_max = ui.radiusMaxSpinBox_3->value();
	helper->dev[1].imProf.radius_min = ui.radiusMinSpinBox_3->value();
	helper->dev[1].imProf.roiRect.x = ui.roiSpinBox_x_3->value();
	helper->dev[1].imProf.roiRect.y = ui.roiSpinBox_y_3->value();
	helper->dev[1].imProf.roiRect.width = ui.roiSpinBox_w_3->value();
	helper->dev[1].imProf.roiRect.height = ui.roiSpinBox_h_3->value();
}

void SettingDialog::roiSlot(int)
{
	helper->dev[0].imProf.roiRect = cv::Rect(ui.roiSpinBox_x->value(), ui.roiSpinBox_y->value(), ui.roiSpinBox_w->value(), ui.roiSpinBox_h->value());
	helper->dev[1].imProf.roiRect = cv::Rect(ui.roiSpinBox_x_3->value(), ui.roiSpinBox_y_3->value(), ui.roiSpinBox_w_3->value(), ui.roiSpinBox_h_3->value());
}