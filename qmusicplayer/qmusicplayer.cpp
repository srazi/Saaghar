/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *  It's based on "qmusicplayer" example of Qt ToolKit                     *
 *                                                                         *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).       *
 * All rights reserved.                                                    *
 * Contact: Nokia Corporation (qt-info@nokia.com)                          *
 *                                                                         *
 * This file is part of the examples of the Qt Toolkit.                    *
 *                                                                         *
 *  Copyright (C) 2010-2012 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 * $QT_BEGIN_LICENSE:BSD$                                                  *
 * You may use this file under the terms of the BSD license as follows:    *
 *                                                                         *
 * "Redistribution and use in source and binary forms, with or without     *
 * modification, are permitted provided that the following conditions are  *
 * met:                                                                    *
 *   * Redistributions of source code must retain the above copyright      *
 *     notice, this list of conditions and the following disclaimer.       *
 *   * Redistributions in binary form must reproduce the above copyright   *
 *     notice, this list of conditions and the following disclaimer in     *
 *     the documentation and/or other materials provided with the          *
 *     distribution.                                                       *
 *   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor  *
 *     the names of its contributors may be used to endorse or promote     *
 *     products derived from this software without specific prior written  *
 *     permission.                                                         *
 *                                                                         *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   *
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."   *
 * $QT_END_LICENSE$                                                        *
 *                                                                         *
 ***************************************************************************/

#include "qmusicplayer.h"

#include <QDesktopServices>
#include <QLCDNumber>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QStyle>
#include <QLabel>
#include <QVBoxLayout>
#include <QTime>
#include <QSettings>

//![0]
QMusicPlayer::QMusicPlayer(QWidget *parent) : QToolBar(parent)
{
	setObjectName("QMusicPlayer");

	_newTime = -1;

	startDir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
	audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	mediaObject = new Phonon::MediaObject(this);
	metaInformationResolver = new Phonon::MediaObject(this);

	mediaObject->setTickInterval(1000);
//![0]
//![2]
	connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
	connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
			this, SLOT(stateChanged(Phonon::State,Phonon::State)));
	connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
			this, SLOT(metaStateChanged(Phonon::State,Phonon::State)));
	connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
			this, SLOT(sourceChanged(Phonon::MediaSource)));
	//connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
	connect(mediaObject, SIGNAL(finished()), this, SLOT(aboutToFinish()));

	qDebug()<<"connect="<< connect(mediaObject, SIGNAL(seekableChanged(bool)), this, SLOT(seekableChanged(bool)));
//![2]

//![1]
	Phonon::createPath(mediaObject, audioOutput);
//![1]

	setupActions();
//	setupMenus();
	setupUi();
	timeLcd->display("00:00"); 
}

//![6]
void QMusicPlayer::seekableChanged(bool seekable)
{
	qDebug() << "seekableChanged"<<seekable<<"_newTime="<<_newTime;
	if (_newTime > 0 && seekable)
	{
		mediaObject->seek(_newTime);
		_newTime = -1;
	}
}

qint64 QMusicPlayer::currentTime()
{
	return mediaObject->currentTime();
}

void QMusicPlayer::setCurrentTime(qint64 time)
{
	_newTime = time;
	/*
	Phonon::State currentState = mediaObject->state();
	qDebug() <<"isSeakable="<<mediaObject->isSeekable()<< "setCTime-state="<<currentState<<"time="<<mediaObject->currentTime()<<"newTime="<<time;
	//stateChanged(Phonon::PausedState, Phonon::PausedState);
	mediaObject->play();
	mediaObject->seek(time);
	qDebug() <<"isSeakable="<<mediaObject->isSeekable() << "setCTime-state="<<currentState<<"time="<<mediaObject->currentTime()<<"newTime="<<time;*/
}

QString QMusicPlayer::source()
{
	return metaInformationResolver->currentSource().fileName();
}

void QMusicPlayer::setSource(const QString &fileName)
{
	_newTime = -1;
	Phonon::MediaSource source(fileName);
	sources.clear();
	infoLabel->setText("");

	//if (!fileName.isEmpty())
	{
	sources.append(source);
	}

	metaInformationResolver->setCurrentSource(source);
	load(0);

////	if (!sources.isEmpty())
////	{

////	}
////	else
//	if (fileName.isEmpty())
//	{
//		stopAction->setEnabled(false);
//		togglePlayPauseAction->setEnabled(false);
//		//pauseAction->setEnabled(false);
//		togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
//		togglePlayPauseAction->setText(tr("Play"));
//		disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
//		disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
//		timeLcd->display("00:00");
//	}

	emit mediaChanged(fileName);

}

void QMusicPlayer::setSource()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Select Music Files"), startDir);

	QFileInfo selectedFile(file);
	startDir = selectedFile.path();

	if (file.isEmpty())
		return;

//	int index = sources.size();
//	foreach (QString string, files) {
//			Phonon::MediaSource source(string);

//		sources.append(source);
//	}

	setSource(file);

//	Phonon::MediaSource source(file);
//	sources.clear();
//	sources.append(source);

//	if (!sources.isEmpty())
//	{
//		setSource(file);
//		//metaInformationResolver->setCurrentSource(sources.at(0/*index*/));
//		//load(0);
//	}

}
//![6]

//void QMusicPlayer::about()
//{
//	QMessageBox::information(this, tr("About Music Player"),
//		tr("The Music Player example shows how to use Phonon - the multimedia"
//		   " framework that comes with Qt - to create a simple music player."));
//}

//![9]
void QMusicPlayer::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
	switch (newState) {
		case Phonon::ErrorState:
			emit mediaChanged("");
		//errors are handled in metaStateChanged()
//			if (mediaObject->errorType() == Phonon::FatalError) {
//				QMessageBox::warning(this, tr("Fatal Error"),
//				mediaObject->errorString());
//			} else {
//				QMessageBox::warning(this, tr("Error"),
//				mediaObject->errorString());
//			}
			break;
//![9]
//![10]
		case Phonon::PlayingState:
				//togglePlayPauseAction->setEnabled(false);
				//pauseAction->setEnabled(true);
				stopAction->setEnabled(true);
				togglePlayPauseAction->setEnabled(true);
				togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
				togglePlayPauseAction->setText(tr("Pause"));
				disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
				connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
				break;
		case Phonon::StoppedState:
				stopAction->setEnabled(false);
				togglePlayPauseAction->setEnabled(true);
				//pauseAction->setEnabled(false);
				togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
				togglePlayPauseAction->setText(tr("Play"));
				connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
				disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
				timeLcd->display("00:00");
				break;
		case Phonon::PausedState:
				//pauseAction->setEnabled(false);
				stopAction->setEnabled(true);
				togglePlayPauseAction->setEnabled(true);
				togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
				togglePlayPauseAction->setText(tr("Play"));
				connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
				disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
				break;
//![10]
		case Phonon::BufferingState:
				break;
		default:
			;
	}

	if (mediaObject->currentSource().type() == Phonon::MediaSource::Invalid)
	{
		stopAction->setEnabled(false);
		togglePlayPauseAction->setEnabled(false);
		togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
		togglePlayPauseAction->setText(tr("Play"));
	}
}

//![11]
void QMusicPlayer::tick(qint64 time)
{
	QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

	timeLcd->display(displayTime.toString("mm:ss"));
}
//![11]

//![12]
void QMusicPlayer::load(int index)
{
	bool wasPlaying = mediaObject->state() == Phonon::PlayingState;

	mediaObject->stop();
	mediaObject->clearQueue();

	if (index >= sources.size())
		return;

	mediaObject->setCurrentSource(sources.at(index));

	if (wasPlaying)
		mediaObject->play();
	else
		mediaObject->stop();
}
//![12]

//![13]
void QMusicPlayer::sourceChanged(const Phonon::MediaSource &source)
{
	//musicTable->selectRow(sources.indexOf(source));
	timeLcd->display("00:00");
}
//![13]

//![14]
void QMusicPlayer::metaStateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
	qDebug() << "metaStateChanged";
	if (newState == Phonon::ErrorState) {
		QMessageBox::warning(this, tr("Error opening files"),
			metaInformationResolver->errorString());
		while (!sources.isEmpty() &&
			   !(sources.takeLast() == metaInformationResolver->currentSource())) {}  /* loop */;
		return;
	}

	if (newState != Phonon::StoppedState && newState != Phonon::PausedState)
		return;

	if (metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid)
			return;

	QMap<QString, QString> metaData = metaInformationResolver->metaData();

	QString title = metaData.value("TITLE");
	if (title == "")
		title = metaInformationResolver->currentSource().fileName();
	//qDebug() << "metaData-keys="<<QStringList(metaData.keys()).join("!#!");
	//qDebug() <<"---"<<metaInformationResolver->metaData(Phonon::DateMetaData).join("{}");
	//qDebug() << "metaData-values="<<QStringList(metaData.values()).join("!#!");
	QStringList metaInfos;
	metaInfos << title << metaData.value("ARTIST") << metaData.value("ALBUM") << metaData.value("DATE");
	metaInfos.removeDuplicates();
	QString tmp = metaInfos.join("-");
	metaInfos = tmp.split("-", QString::SkipEmptyParts);
	infoLabel->setText(metaInfos.join(" - "));

//	QTableWidgetItem *titleItem = new QTableWidgetItem(title);
//	titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);
//	QTableWidgetItem *artistItem = new QTableWidgetItem(metaData.value("ARTIST"));
//	artistItem->setFlags(artistItem->flags() ^ Qt::ItemIsEditable);
//	QTableWidgetItem *albumItem = new QTableWidgetItem(metaData.value("ALBUM"));
//	albumItem->setFlags(albumItem->flags() ^ Qt::ItemIsEditable);
//	QTableWidgetItem *yearItem = new QTableWidgetItem(metaData.value("DATE"));
//	yearItem->setFlags(yearItem->flags() ^ Qt::ItemIsEditable);
//![14]

//	int currentRow = musicTable->rowCount();
//	musicTable->insertRow(currentRow);
//	musicTable->setItem(currentRow, 0, titleItem);
//	musicTable->setItem(currentRow, 1, artistItem);
//	musicTable->setItem(currentRow, 2, albumItem);
//	musicTable->setItem(currentRow, 3, yearItem);

////![15]
//	if (musicTable->selectedItems().isEmpty()) {
//		musicTable->selectRow(0);
//		mediaObject->setCurrentSource(metaInformationResolver->currentSource());
//	}

	Phonon::MediaSource source = metaInformationResolver->currentSource();
	int index = sources.indexOf(metaInformationResolver->currentSource()) + 1;
	if (sources.size() > index) {
		metaInformationResolver->setCurrentSource(sources.at(index));
	}
//	else {
//		musicTable->resizeColumnsToContents();
//		if (musicTable->columnWidth(0) > 300)
//			musicTable->setColumnWidth(0, 300);
//	}
}
//![15]

//![16]
void QMusicPlayer::aboutToFinish()
{
	mediaObject->stop();
//	int index = sources.indexOf(mediaObject->currentSource()) + 1;
//	if (sources.size() > index) {
//		mediaObject->enqueue(sources.at(index));
//	}
}
//![16]

void QMusicPlayer::setupActions()
{
	togglePlayPauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
	togglePlayPauseAction->setShortcut(tr("Ctrl+P"));
	togglePlayPauseAction->setDisabled(true);
//	pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
//	pauseAction->setShortcut(tr("Ctrl+A"));
//	pauseAction->setDisabled(true);
	stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
	stopAction->setShortcut(tr("Ctrl+S"));
	stopAction->setDisabled(true);
	nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
	nextAction->setShortcut(tr("Ctrl+N"));
	previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
	previousAction->setShortcut(tr("Ctrl+R"));
	setSourceAction = new QAction(tr("Set &Source"), this);
	//setSourceAction->setShortcut(tr("Ctrl+F"));
//	exitAction = new QAction(tr("E&xit"), this);
//	exitAction->setShortcuts(QKeySequence::Quit);
//	aboutAction = new QAction(tr("A&bout"), this);
//	aboutAction->setShortcut(tr("Ctrl+B"));
//	aboutQtAction = new QAction(tr("About &Qt"), this);
//	aboutQtAction->setShortcut(tr("Ctrl+Q"));

//![5]
	connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
	//connect(pauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
	connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));
//![5]
	connect(setSourceAction, SIGNAL(triggered()), this, SLOT(setSource()));
//	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
//	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
//	connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

//void QMusicPlayer::setupMenus()
//{
//	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
//	fileMenu->addAction(setSourceAction);
//	fileMenu->addSeparator();
//	fileMenu->addAction(exitAction);

//	QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
//	aboutMenu->addAction(aboutAction);
//	aboutMenu->addAction(aboutQtAction);
//}

//![3]
void QMusicPlayer::setupUi()
{//return;
//![3]
//	QToolBar *bar = new QToolBar(this);

//	bar->addAction(togglePlayPauseAction);
//	bar->addAction(pauseAction);
//	bar->addAction(stopAction);
	addAction(setSourceAction);
	addSeparator();
	addAction(togglePlayPauseAction);
	//addAction(pauseAction);
	addAction(stopAction);
	addSeparator();

//![4]
	seekSlider = new Phonon::SeekSlider(this);
	seekSlider->setMediaObject(mediaObject);

	infoLabel = new QLabel("");

	volumeSlider = new Phonon::VolumeSlider(this);
	volumeSlider->setAudioOutput(audioOutput);
//![4]
	volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	//QLabel *volumeLabel = new QLabel;
	//volumeLabel->setPixmap(QPixmap("images/volume.png"));

	QPalette palette;
	palette.setBrush(QPalette::Light, Qt::darkGray);

	timeLcd = new QLCDNumber;
	timeLcd->setPalette(palette);

	//QStringList headers;
	//headers << tr("Title") << tr("Artist") << tr("Album") << tr("Year");

//	musicTable = new QTableWidget(0, 4);
//	musicTable->setFixedSize(1,1);
//	musicTable->hide();
//	musicTable->setHorizontalHeaderLabels(headers);
//	musicTable->setSelectionMode(QAbstractItemView::SingleSelection);
//	musicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
//	connect(musicTable, SIGNAL(cellPressed(int,int)),
//			this, SLOT(tableClicked(int,int)));

//	QHBoxLayout *seekerLayout = new QHBoxLayout;
//	seekerLayout->addWidget(seekSlider);
//	seekerLayout->addWidget(timeLcd);

//	QHBoxLayout *playbackLayout = new QHBoxLayout;
//	playbackLayout->addWidget(bar);
//	playbackLayout->addStretch();
//	playbackLayout->addWidget(volumeLabel);
//	playbackLayout->addWidget(volumeSlider);

	QVBoxLayout *infoLayout = new QVBoxLayout;
	infoLayout->addWidget(seekSlider);
	infoLayout->addWidget(infoLabel,0, Qt::AlignCenter);
	infoLayout->setSpacing(0);
	infoLayout->setContentsMargins(0,0,0,0);

	QWidget *infoWidget = new QWidget;
	infoWidget->setLayout(infoLayout);

//	QHBoxLayout *mainLayout = new QHBoxLayout;
//	mainLayout->addWidget(seekSlider);
//	mainLayout->addWidget(timeLcd);
//	mainLayout->addWidget(volumeSlider);

	addWidget(infoWidget);
	addWidget(timeLcd);
	addWidget(volumeSlider);
	

	setLayoutDirection(Qt::LeftToRight);

	//addWidget(widget);
	//setCentralWidget(widget);
	//setWindowTitle("Phonon Music Player");
}

void QMusicPlayer::readPlayerSettings(QSettings *settingsObject)
{
	settingsObject->beginGroup("QMusicPlayer");
	if (audioOutput)
	{
		audioOutput->setMuted(settingsObject->value("muted",false).toBool());
		audioOutput->setVolume(settingsObject->value("volume",0.4).toReal());
	}
	settingsObject->endGroup();
}

void QMusicPlayer::savePlayerSettings(QSettings *settingsObject)
{
	settingsObject->beginGroup("QMusicPlayer");
	if (audioOutput)
	{
		settingsObject->setValue("muted", audioOutput->isMuted());
		settingsObject->setValue("volume", audioOutput->volume());
	}
	settingsObject->endGroup();
}

