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

#ifndef PLAYER_H
#define PLAYER_H

#include "videowidget.h"

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QMediaPlayer;
class QPushButton;
class CustomSlider;

class QVideoWidget;
QT_END_NAMESPACE

class Player : public QWidget {
	Q_OBJECT

public:
	Player(QWidget* parent = 0);
	~Player();

	public slots:
	void setUrl(const QUrl &url);	//sql table set the playerurl

signals:
	void fullScreenChanged(bool fullScreen);
	void play();
	void pause();

	private slots:
	//void open();
	void durationChanged(qint64 duration);
	void positionChanged(qint64 progress);

	void seek(int seconds);

	void statusChanged(QMediaPlayer::MediaStatus status);
	//void stateChanged(QMediaPlayer::State state);
	void videoAvailableChanged(bool available);
	void setState(QMediaPlayer::State state);
	void playClicked();
	void openFilePath();

private:
	void handleCursor(QMediaPlayer::MediaStatus status);
	void updateDurationInfo(qint64 currentInfo);
	QMediaPlayer::State playerState;
	QMediaPlayer::State state() const;
	QMediaPlayer* player;
	VideoWidget* videoWidget;
	CustomSlider* slider;
	QPushButton* playButton;
	QPushButton* fullScreenButton;
	QPushButton* openFilePathBtn;
	QLabel* labelDuration;
	QString statusInfo;
	qint64 duration;
	QString currentFileName;
};

#endif // PLAYER_H
