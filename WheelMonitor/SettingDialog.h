#pragma once

#include "ui_SettingDialog.h"
#include <QDialog>
#include <opencv2/opencv.hpp>
class SettingDialog : public QDialog {
	Q_OBJECT

public:
	SettingDialog(QWidget* parent = Q_NULLPTR);
	~SettingDialog();

private:
	Ui::SettingDialog ui;
	static cv::Rect roiRectCache;

protected:
	//	void closeEvent(QCloseEvent *event);

	signals :
			void roiChanged();

			private slots:
			void roiSlot(int);
			void on_okBtn_clicked();
			void on_cancelBtn_clicked();
};
