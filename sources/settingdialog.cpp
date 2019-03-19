#include "stdafx.h"
#include "settingdialog.h"
#include "confighelper.h"
#include "common.h"

SettingDialog::SettingDialog(ConfigHelper* ch, QWidget* parent /*= Q_NULLPTR*/)
	: helper(ch)
	, QDialog(parent)
{
	ui.setupUi(this);
	ini2ui();

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [this]() {ui2ini(); qWarning() << "Option changed"; this->accept(); });
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, [this]() {helper->read(); this->reject(); });
	connect(ui.buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &SettingDialog::ini2ui);

	connect(ui.roiSpinBox_x, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_y, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_w, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_h, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_x_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_y_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_w_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));
	connect(ui.roiSpinBox_h_3, SIGNAL(valueChanged(int)), this, SLOT(roiSlot(int)));

	// 根据内容的变化设置保存按钮
	// 更改：在ui中设定
	//trackChanged();

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
	//save
	ui.spb_saveDays->setValue(g_videoKeepDays);
	ui.lnEd_saveDir->setText(g_videoDirPath);
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

void SettingDialog::onClickSelSaveDir()
{
	//videoDirPath
	QFileDialog saveDirDlg(this);
	saveDirDlg.setFileMode(QFileDialog::Directory);
	g_mutex.lock();
	saveDirDlg.setDirectory(g_videoDirPath);
	g_mutex.unlock();
	QString dirPath;
	if (saveDirDlg.exec())
		dirPath = saveDirDlg.selectedFiles().at(0);
	if (dirPath.isEmpty())
		return;
	ui.lnEd_saveDir->setText(dirPath);
}

void SettingDialog::onClickClearCacheNow()
{
	emit clearVideo(1);
}

void SettingDialog::onClickClearCacheAll()
{
	emit clearVideo(0);
}

void SettingDialog::onUiChanged()
{
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void SettingDialog::ui2ini()
{
	//common
	helper->launchAtLogin = ui.LaunchAtLoginCheckBox->isChecked();
	helper->startAtLaunch = ui.startAtLaunchCheckBox->isChecked();
	helper->verboseLog = ui.verboseLogCheckBox->isChecked();

	//save
	g_mutex.lock();
	g_videoKeepDays = ui.spb_saveDays->value();
	g_videoDirPath = ui.lnEd_saveDir->text();
	g_mutex.unlock();
	QDate date = QDate::currentDate();
	QString today = date.toString("yyyyMMdd");
	QString tomorrow = date.addDays(1).toString("yyyyMMdd");
	QDir dir(g_videoDirPath);
	dir.mkpath(today);
	dir.mkpath(tomorrow);

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