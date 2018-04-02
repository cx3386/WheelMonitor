#pragma once

#include "ui_SettingDialog.h"
#include <QDialog>
#include <opencv2/opencv.hpp>

class ConfigHelper;
class SettingDialog : public QDialog {
	Q_OBJECT

public:
	explicit SettingDialog(ConfigHelper& ch, QWidget* parent = Q_NULLPTR);
	~SettingDialog();

private:
	Ui::SettingDialog ui;
	ConfigHelper *helper;

	void helperToUi();
	void uiToHelper();

	private slots:
	void onAccepted();
	void onRejected();
	void onChanged();
	void onRestore();
	void roiSlot(int);
	void on_okBtn_clicked();
	void on_cancelBtn_clicked();
};
