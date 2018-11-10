#pragma once

#include <QDialog>
#include "ui_ViewSensorImage.h"

class ViewSensorImage : public QDialog
{
	Q_OBJECT

public:
	ViewSensorImage(QWidget *parent = Q_NULLPTR);
	~ViewSensorImage();

private:
	Ui::ViewSensorImage ui;
};
