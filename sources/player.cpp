/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "stdafx.h"
#include "player.h"
#include "customslider.h"
#include <QMediaService>

Player::Player(QWidget* parent)
	: QWidget(parent)
	, videoWidget(nullptr)
	, slider(nullptr)
{
	//! [create-objs]
	player = new QMediaPlayer(this);

	//! [create-objs]

	connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
	connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));

	connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
	connect(player, SIGNAL(videoAvailableChanged(bool)), this, SLOT(videoAvailableChanged(bool)));

	videoWidget = new VideoWidget(this);
	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	player->setVideoOutput(videoWidget);

	slider = new CustomSlider(Qt::Horizontal, this);
	slider->setRange(0, player->duration());
	slider->setTracking(true);
	slider->setCursor(QCursor(Qt::PointingHandCursor));
	slider->setStyleSheet("QSlider::add-page:horizontal{   background-color: rgb(87, 97, 106);  height:4px; }"
		"QSlider::sub-page:horizontal {background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(231,80,229, 255), stop:1 rgba(7,208,255, 255));"
		"height:4px;}"

		"QSlider::add-page:hover:horizontal{   background-color: rgb(87, 97, 106);  height:6px; }"
		"QSlider::sub-page:hover:horizontal {background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(231,80,229, 255), stop:1 rgba(7,208,255, 255));"
		"height:6px;}"

		"QSlider::handle:horizontal {margin-top:-2px; margin-bottom:-2px;"
		"border-radius:4px;  background: rgb(222,222,222); width:14px; height:18px;}"

		"QSlider::groove:horizontal {background:transparent;height:4px}"
		"QSlider::groove:hover:horizontal {background:transparent;height:6px}");

	labelDuration = new QLabel(this);
	labelDuration->setStyleSheet("font-size: 12px;");
	connect(slider, &CustomSlider::sliderMoved, this, &Player::seek);
	connect(slider, &CustomSlider::costomSliderClicked, this, &Player::seek);

	playButton = new QPushButton(this);
	playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	connect(playButton, SIGNAL(clicked()), this, SLOT(playClicked()));
	connect(this, SIGNAL(play()), player, SLOT(play()));
	connect(this, SIGNAL(pause()), player, SLOT(pause()));

	setState(player->state());
	connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(setState(QMediaPlayer::State)));	//play/pause

	fullScreenButton = new QPushButton(this);
	fullScreenButton->setCheckable(true);
	fullScreenButton->setToolTip(QStringLiteral("全屏"));
	fullScreenButton->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));

	openFilePathBtn = new QPushButton(this);
	openFilePathBtn->setToolTip(QStringLiteral("打开路径"));
	openFilePathBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	connect(openFilePathBtn, &QPushButton::clicked, this, &Player::openFilePath);

	QBoxLayout* controlLayout = new QHBoxLayout;
	controlLayout->setMargin(0);
	controlLayout->addWidget(playButton);
	controlLayout->addWidget(fullScreenButton);
	controlLayout->addWidget(openFilePathBtn);
	controlLayout->addStretch();
	controlLayout->addWidget(labelDuration);

	QBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(videoWidget);
	layout->addWidget(slider);
	layout->addLayout(controlLayout);

	setLayout(layout);
	this->setEnabled(false);
}

Player::~Player()
= default;

void Player::setUrl(const QUrl & url)
{
	player->setMedia(url);
	currentFileName = url.toLocalFile();
	emit playButton->clicked();	//if detect media changed, start play
}

void Player::durationChanged(qint64 duration)
{
	this->duration = duration / 1000;
	slider->setMaximum(duration);
}

void Player::positionChanged(qint64 progress)
{
	if (!slider->isSliderDown()) {
		slider->setValue(progress);
	}
	updateDurationInfo(progress / 1000);
}

void Player::seek(int mseconds)
{
	player->setPosition(mseconds);
}

void Player::statusChanged(QMediaPlayer::MediaStatus status)
{
	handleCursor(status);

	// handle status message
	switch (status) {
	case QMediaPlayer::UnknownMediaStatus:
	case QMediaPlayer::NoMedia:
	case QMediaPlayer::BufferingMedia:
	case QMediaPlayer::LoadingMedia:
	case QMediaPlayer::StalledMedia:
	case QMediaPlayer::InvalidMedia:
		this->setEnabled(false);
		break;
	case QMediaPlayer::EndOfMedia:
		emit play();
		//emit playButton->clicked();
		//break;
	default:
		this->setEnabled(true);
		break;
	}
}

void Player::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
	if (status == QMediaPlayer::LoadingMedia || status == QMediaPlayer::BufferingMedia || status == QMediaPlayer::StalledMedia)
		setCursor(QCursor(Qt::BusyCursor));
	else
		unsetCursor();
#endif
}

void Player::videoAvailableChanged(bool available)
{
	if (!available) {
		disconnect(fullScreenButton, SIGNAL(clicked(bool)),
			videoWidget, SLOT(setFullScreen(bool)));
		disconnect(videoWidget, SIGNAL(fullScreenChanged(bool)),
			fullScreenButton, SLOT(setChecked(bool)));
		videoWidget->setFullScreen(false);
	}
	else {
		connect(fullScreenButton, SIGNAL(clicked(bool)),
			videoWidget, SLOT(setFullScreen(bool)));
		connect(videoWidget, SIGNAL(fullScreenChanged(bool)),
			fullScreenButton, SLOT(setChecked(bool)));

		if (fullScreenButton->isChecked())
			videoWidget->setFullScreen(true);
	}
}

void Player::updateDurationInfo(qint64 currentInfo)
{
	QString tStr;
	if (currentInfo || duration) {
		QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60, currentInfo % 60, (currentInfo * 1000) % 1000);
		QTime totalTime((duration / 3600) % 60, (duration / 60) % 60, duration % 60, (duration * 1000) % 1000);
		QString format = "mm:ss";
		if (duration > 3600)
			format = "hh:mm:ss";
		tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
	}
	labelDuration->setText(tStr);
}

QMediaPlayer::State Player::state() const
{
	return playerState;
}

void Player::setState(QMediaPlayer::State state)
{
	if (state != playerState) {
		playerState = state;

		switch (state) {
		case QMediaPlayer::StoppedState:
			playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
			break;
		case QMediaPlayer::PlayingState:
			playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
			break;
		case QMediaPlayer::PausedState:
			playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
			break;
		}
	}
}

void Player::playClicked()
{
	switch (playerState) {
	case QMediaPlayer::StoppedState:
	case QMediaPlayer::PausedState:
		emit play();
		break;
	case QMediaPlayer::PlayingState:
		emit pause();
		break;
	}
}

void Player::openFilePath()
{
	QProcess process;
	process.startDetached(QStringLiteral("explorer /select, \"%1\"").arg(currentFileName.replace("/", "\\")));
}