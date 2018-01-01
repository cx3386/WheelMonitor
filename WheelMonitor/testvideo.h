#pragma once

#include <QObject>

class TestVideo : public QObject
{
	Q_OBJECT

public:
	TestVideo(QObject *parent = Q_NULLPTR);
	~TestVideo();
	public slots:
	int sendVideoFrame();
	void processReady();
private:
	bool isProcessing;
};
