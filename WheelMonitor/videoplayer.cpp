#include "stdafx.h"
#include "videoplayer.h"
#include <QVideoWidget>

VideoPlayer::VideoPlayer(QWidget *parent) : QWidget(parent)
{
	QVideoWidget *videoWidget = new QVideoWidget;

	fullScreenBtn = new QPushButton;
	fullScreenBtn->setToolTip(QStringLiteral("全屏"));
	//fullScreenBtn->setEnabled(false);
	fullScreenBtn->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
	connect(fullScreenBtn, &QPushButton::clicked, this, &VideoPlayer::showFullScreen);

	openFilePathBtn = new QPushButton;
	openFilePathBtn->setToolTip(QStringLiteral("打开路径"));
	//openFilePathBtn->setEnabled(false);
	openFilePathBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	connect(openFilePathBtn, &QPushButton::clicked, this, &VideoPlayer::openFilePath);

	playBtn = new QPushButton;
	//playBtn->setEnabled(false);
	playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	connect(playBtn, &QPushButton::clicked, this, &VideoPlayer::play);

	positionSlider = new QSlider(Qt::Horizontal);
	positionSlider->setRange(0, 0);
	connect(positionSlider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::setPosition);

	durationLabel = new QLabel;
	durationLabel->setText("00:00:00/00:00:00");
	mediaPlayer.setVideoOutput(videoWidget);

	QBoxLayout *controlLayout = new QHBoxLayout;
	controlLayout->setMargin(0);
	controlLayout->addWidget(playBtn);
	controlLayout->addWidget(fullScreenBtn);
	controlLayout->addWidget(openFilePathBtn);
	controlLayout->addStretch();
	controlLayout->addWidget(durationLabel);

	QBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(videoWidget);
	layout->addWidget(positionSlider);
	layout->addLayout(controlLayout);
	setLayout(layout);

	this->setEnabled(false);

	connect(&mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &VideoPlayer::mediaStatusChanged);
	connect(&mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::mediaStateChanged);
	connect(&mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
	connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
}

VideoPlayer::~VideoPlayer() {}

void VideoPlayer::setUrl(const QUrl &url)
{
	mediaPlayer.setMedia(url);
	currentFileName = url.toLocalFile();
	play();
}

void VideoPlayer::play()
{
	switch (mediaPlayer.state())
	{
	case QMediaPlayer::PlayingState:
		mediaPlayer.pause();
		break;
	default:
		mediaPlayer.play();
		break;
	}
}

void VideoPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
	switch (status)
	{
	case QMediaPlayer::NoMedia:
		//openFilePathBtn->setEnabled(false);
		//fullScreenBtn->setEnabled(false);
		this->setEnabled(false);
		break;
	default:
		//openFilePathBtn->setEnabled(true);
		//fullScreenBtn->setEnabled(true);
		this->setEnabled(true);
		break;
	}
}

void VideoPlayer::mediaStateChanged(QMediaPlayer::State state)
{
	switch (state)
	{
	case QMediaPlayer::PlayingState:
		playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
		break;
	default:
		playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
		break;
	}
}

void VideoPlayer::positionChanged(qint64 position)
{
	positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
	positionSlider->setRange(0, duration);
}

void VideoPlayer::setPosition(int position)
{
	mediaPlayer.setPosition(position);
}

void VideoPlayer::openFilePath()
{
	QProcess process;
	process.startDetached(QStringLiteral("explorer /select, \"%1\"").arg(currentFileName.replace("/", "\\")));
}