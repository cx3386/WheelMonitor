#pragma once

#include <QMediaPlayer>
#include <QtWidgets>

class VideoPlayer : public QWidget
{
	Q_OBJECT
public:
	VideoPlayer(QWidget *parent = Q_NULLPTR);
	~VideoPlayer();

	HWND playHandle;

	public slots:
	void setUrl(const QUrl &url);	//sql table set the playerurl

	private slots :
	void play();
	//void fullScreen();
	void openFilePath();
	void mediaStatusChanged(QMediaPlayer::MediaStatus status);
	void mediaStateChanged(QMediaPlayer::State state);
	void positionChanged(qint64 position);
	void durationChanged(qint64 duration);
	void setPosition(int pos);	//update slider

private:
	bool bPlaying;
	QString currentFileName;
	//void initUI();
	//void setState();	//if video state changed, set the playBtn icon
	QMediaPlayer mediaPlayer;
	QSlider *positionSlider;
	QPushButton *playBtn;
	QPushButton *fullScreenBtn;
	QPushButton *openFilePathBtn;
	//QPushButton *fullScreenBtn;
	//QPushButton *filePathBtn;
	QLabel *durationLabel;
};
