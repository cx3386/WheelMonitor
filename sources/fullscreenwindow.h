#pragma once

#include <QWidget>
#include "ui_fullscreenwindow.h"

class FullScreenWindow : public QWidget
{
	Q_OBJECT

public:
	FullScreenWindow(QWidget *parent = Q_NULLPTR);
	~FullScreenWindow();

private:
	Ui::FullScreenWindow ui;
};
