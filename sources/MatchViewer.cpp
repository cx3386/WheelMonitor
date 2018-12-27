#include "stdafx.h"
#include "MatchViewer.h"
#include "opencv.hpp"
#include "imageprocess.h"

MatchViewer::MatchViewer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

MatchViewer::~MatchViewer()
{
}

void MatchViewer::bindDev(ImageProcess *imageProcess)
{
	m_imageProcess = imageProcess;
	connect(m_imageProcess, &ImageProcess::showMatch, this, [=]() {showMatch(); });
}

void MatchViewer::showMatch()
{
	cv::Mat src = m_imageProcess->getMatchResult();
	QImage im(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
	im = im.rgbSwapped();
	im = im.scaled(ui.label->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
	ui.label->setPixmap(QPixmap::fromImage(im));
}