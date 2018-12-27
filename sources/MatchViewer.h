#pragma once

#include <QWidget>
#include "ui_MatchViewer.h"

class ImageProcess;
class MatchViewer : public QWidget
{
	Q_OBJECT

public:
	MatchViewer(QWidget *parent = Q_NULLPTR);
	~MatchViewer();
	void bindDev(ImageProcess *imageProcess);

private:
	Ui::MatchViewer ui;
	ImageProcess* m_imageProcess;
private slots:
	void showMatch();
};
