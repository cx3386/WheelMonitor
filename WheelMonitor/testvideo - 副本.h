#pragma once

#include <QObject>

class TestVideo : public QObject {
	Q_OBJECT
public:
	TestVideo(QObject* parant = 0);
	~TestVideo();

	public slots:
	int sendVideoFrame();
	void processReady();
private:
	bool isProcessing;
};
