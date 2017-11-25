#pragma once

#include <QDialog>
#include "ui_SettingDialog.h"

class SettingDialog : public QDialog
{
	Q_OBJECT

public:
	SettingDialog(QWidget *parent = Q_NULLPTR);
	~SettingDialog();
	void setOption();

private:
	Ui::SettingDialog ui;

protected:
	//	void closeEvent(QCloseEvent *event);

	signals :
			void roiChanged();

			private slots:
			void roiSlot(int);
};
