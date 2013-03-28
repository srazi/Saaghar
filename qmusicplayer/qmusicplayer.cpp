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
 *  Copyright (C) 2010-2013 by S. Razi Alavizadeh                          *
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
#include <QToolButton>
#include <QMenu>
#include <QDockWidget>

QHash<QString, QVariant> QMusicPlayer::listOfPlayList = QHash<QString, QVariant>();
QMusicPlayer::SaagharPlayList QMusicPlayer::hashDefaultPlayList = QMusicPlayer::SaagharPlayList();
QHash<QString, QMusicPlayer::SaagharPlayList*> QMusicPlayer::hashPlayLists = QHash<QString, QMusicPlayer::SaagharPlayList*>();


const bool itemsAsTopItem = true;

QMusicPlayer::QMusicPlayer(QWidget* parent) : QToolBar(parent)
{
    setObjectName("QMusicPlayer");

    setStyleSheet("background-image:url(\":/resources/images/transp.png\"); border:none;");

    notLoaded = true;
    dockList = 0;

    playListManager = new PlayListManager;
    connect(playListManager, SIGNAL(mediaPlayRequested(int)), this, SLOT(playMedia(int)));
    connect(this, SIGNAL(mediaChanged(QString,QString,int,bool)), playListManager, SLOT(currentMediaChanged(QString,QString,int,bool)));

    QMusicPlayer::hashDefaultPlayList.PATH = "";

    _newTime = -1;

    startDir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    QList<Phonon::AudioOutputDevice> audioOutputDevices = Phonon::BackendCapabilities::availableAudioOutputDevices();
    foreach (const Phonon::AudioOutputDevice newAudioOutput, audioOutputDevices) {
        if (newAudioOutput.name().contains("waveout", Qt::CaseInsensitive)) {
            audioOutput->setOutputDevice(newAudioOutput);
            break;
        }
    }
    qDebug() << "audioOutput==" << audioOutputDevices << "output=" << audioOutput->outputDevice().name();
    mediaObject = new Phonon::MediaObject(this);
    metaInformationResolver = new Phonon::MediaObject(this);

    mediaObject->setTickInterval(1000);

    playListManager->setMediaObject(mediaObject);

    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(metaStateChanged(Phonon::State,Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(finished()), this, SLOT(aboutToFinish()));

    connect(mediaObject, SIGNAL(seekableChanged(bool)), this, SLOT(seekableChanged(bool)));

    Phonon::createPath(mediaObject, audioOutput);

    setupActions();
    setupUi();
    timeLcd->display("00:00");
}

void QMusicPlayer::seekableChanged(bool seekable)
{
    qDebug() << "seekableChanged" << seekable << "_newTime=" << _newTime;
    if (_newTime > 0 && seekable) {
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
}

QString QMusicPlayer::source()
{
    return metaInformationResolver->currentSource().fileName();
}

void QMusicPlayer::setSource(const QString &fileName, const QString &title, int mediaID)
{
    _newTime = -1;
    Phonon::MediaSource source(fileName);
    sources.clear();
    infoLabel->setText("");

    if (!fileName.isEmpty()) {
        notLoaded = false;
        sources.append(source);
        metaInformationResolver->setCurrentSource(source);
        load(0);
        if (mediaID > 0) {
            QString playListName;
            playListContains(mediaID, &playListName);
            if (!playListName.isEmpty()) {
                playListManager->setCurrentPlayList(playListName);
            }
        }
    }
    else {
        notLoaded = true;
        mediaObject->clear();
        metaInformationResolver->clear();
        qDebug() << "DISABLE";
    }

    if (!title.isEmpty()) {
        currentTitle = title;
    }
    currentID = mediaID;
    emit mediaChanged(fileName, currentTitle, currentID, false);
}

void QMusicPlayer::setSource()
{
    qDebug() << "setSource=" << currentID
             << "\n=====================\nAvailable Mime Types=\n"
             << Phonon::BackendCapabilities::availableMimeTypes()
             << "\n=====================";

    QString file = QFileDialog::getOpenFileName(this, tr("Select Music Files"), startDir,
                   "Common Supported Files (" + QMusicPlayer::commonSupportedMedia().join(" ") +
                   ");;Audio Files (" + QMusicPlayer::commonSupportedMedia("audio").join(" ") +
                   ");;Video Files (Audio Stream) (" + QMusicPlayer::commonSupportedMedia("video").join(" ") +
                   ");;All Files (*.*)");

    if (file.isEmpty()) {
        return;
    }

    QFileInfo selectedFile(file);
    startDir = selectedFile.path();

    setSource(file, currentTitle, currentID);
}

void QMusicPlayer::loadPlayListFile()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Saaghar Play List"), startDir,
                   "Saaghar Play List (*.spl *.m3u8 *.m3u);;All Files (*.*)");

    if (file.isEmpty()) {
        return;
    }

    QFileInfo selectedFile(file);
    startDir = selectedFile.path();

    loadPlayList(file);
}

void QMusicPlayer::removeSource()
{
    _newTime = -1;
    sources.clear();
    infoLabel->setText("");
    notLoaded = true;
    mediaObject->clear();
    metaInformationResolver->clear();
    qDebug() << "removeSource->DISABLE";

    emit mediaChanged("", "", currentID, true);

    removeFromPlayList(currentID);
}

void QMusicPlayer::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
    switch (newState) {
    case Phonon::ErrorState:
        emit mediaChanged("", currentTitle, currentID, false);
        break;

    case Phonon::PlayingState:
        stopAction->setEnabled(true);
        togglePlayPauseAction->setEnabled(true);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        togglePlayPauseAction->setText(tr("Pause"));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
        connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        break;
    case Phonon::StoppedState:
        stopAction->setEnabled(false);
        togglePlayPauseAction->setEnabled(true);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
        connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        timeLcd->display("00:00");
        break;
    case Phonon::PausedState:
        stopAction->setEnabled(true);
        togglePlayPauseAction->setEnabled(true);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
        connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        break;

    case Phonon::BufferingState:
        break;
    default:
        ;
    }

    if (notLoaded) {
        infoLabel->setText("");
        stopAction->setEnabled(false);
        togglePlayPauseAction->setEnabled(false);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        timeLcd->display("00:00");
    }

    if (mediaObject->currentSource().type() == Phonon::MediaSource::Invalid) {
        stopAction->setEnabled(false);
        togglePlayPauseAction->setEnabled(false);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
    }
}

void QMusicPlayer::tick(qint64 time)
{
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

    timeLcd->display(displayTime.toString("mm:ss"));
}

void QMusicPlayer::load(int index)
{
    bool wasPlaying = mediaObject->state() == Phonon::PlayingState;

    mediaObject->stop();
    mediaObject->clearQueue();

    if (index >= sources.size()) {
        return;
    }

    mediaObject->setCurrentSource(sources.at(index));

    if (wasPlaying) {
        mediaObject->play();
    }
    else {
        mediaObject->stop();
    }
}

void QMusicPlayer::sourceChanged(const Phonon::MediaSource &)
{
    timeLcd->display("00:00");
}

void QMusicPlayer::metaStateChanged(Phonon::State newState, Phonon::State)
{
    if (newState == Phonon::ErrorState) {
        qWarning() << "Error opening files: " << metaInformationResolver->errorString();
        infoLabel->setText("Error opening files: " + metaInformationResolver->errorString());
        while (!sources.isEmpty() &&
                !(sources.takeLast() == metaInformationResolver->currentSource())
              )
        {}  /* loop */;
        return;
    }

    if (newState != Phonon::StoppedState && newState != Phonon::PausedState) {
        return;
    }

    if (metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid) {
        return;
    }

    QMap<QString, QString> metaData = metaInformationResolver->metaData();

    QString title = metaData.value("TITLE");
    if (title == "") {
        title = metaInformationResolver->currentSource().fileName();
    }

    QStringList metaInfos;
    metaInfos << title << metaData.value("ARTIST") << metaData.value("ALBUM") << metaData.value("DATE");
    metaInfos.removeDuplicates();
    QString tmp = metaInfos.join("-");
    metaInfos = tmp.split("-", QString::SkipEmptyParts);
    infoLabel->setText(metaInfos.join(" - "));

    Phonon::MediaSource source = metaInformationResolver->currentSource();
    int index = sources.indexOf(metaInformationResolver->currentSource()) + 1;
    if (sources.size() > index) {
        metaInformationResolver->setCurrentSource(sources.at(index));
    }

    if (notLoaded) {
        infoLabel->setText("");
        stopAction->setEnabled(false);
        togglePlayPauseAction->setEnabled(false);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        timeLcd->display("00:00");
    }
}

void QMusicPlayer::aboutToFinish()
{
    mediaObject->stop();
}

void QMusicPlayer::setupActions()
{
    togglePlayPauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
    togglePlayPauseAction->setShortcut(tr("Ctrl+P"));
    togglePlayPauseAction->setDisabled(true);
    stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
    stopAction->setShortcut(tr("Ctrl+S"));
    stopAction->setDisabled(true);
    nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
    nextAction->setShortcut(tr("Ctrl+N"));
    previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
    previousAction->setShortcut(tr("Ctrl+R"));
    setSourceAction = new QAction(tr("&Set Audio..."), this);
    removeSourceAction = new QAction(tr("&Remove Audio"), this);
    loadPlayListAction = new QAction(tr("&Load Playlist..."), this);
    connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
    connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));
    connect(setSourceAction, SIGNAL(triggered()), this, SLOT(setSource()));
    connect(removeSourceAction, SIGNAL(triggered()), this, SLOT(removeSource()));
    connect(loadPlayListAction, SIGNAL(triggered()), this, SLOT(loadPlayListFile()));
}

void QMusicPlayer::setupUi()
{
    QToolButton* options = new QToolButton;
    options->setIcon(QIcon(":/images/options.png"));
    options->setPopupMode(QToolButton::InstantPopup);
    options->setStyleSheet("QToolButton::menu-indicator{image: none;}");
    QMenu* menu = new QMenu;
    menu->addAction(setSourceAction);
    menu->addAction(removeSourceAction);
    menu->addSeparator();
    menu->addAction(loadPlayListAction);
    menu->addSeparator();
    if (playListManagerDock()) {
        menu->addAction(playListManagerDock()->toggleViewAction());
    }
    options->setMenu(menu);

    addWidget(options);
    addSeparator();
    addAction(togglePlayPauseAction);
    addAction(stopAction);
    addSeparator();

    seekSlider = new Phonon::SeekSlider(this);
    seekSlider->setMediaObject(mediaObject);

    infoLabel = new ScrollText(this);
    infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    infoLabel->resize(seekSlider->width(), infoLabel->height());

    volumeSlider = new Phonon::VolumeSlider(this);
    volumeSlider->setAudioOutput(audioOutput);

    volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QPalette palette;
    palette.setBrush(QPalette::Light, Qt::darkGray);

    timeLcd = new QLCDNumber;
    timeLcd->setPalette(palette);

    QVBoxLayout* infoLayout = new QVBoxLayout;
    infoLayout->addWidget(seekSlider);
    infoLayout->addWidget(infoLabel/*tmpLabel*/, 0, 0 /* Qt::AlignCenter*/);
    infoLayout->setSpacing(0);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* infoWidget = new QWidget;
    infoWidget->setLayout(infoLayout);

    addWidget(infoWidget);
    addWidget(timeLcd);
    addWidget(volumeSlider);


    setLayoutDirection(Qt::LeftToRight);
}

void QMusicPlayer::readPlayerSettings(QSettings* settingsObject)
{
    settingsObject->beginGroup("QMusicPlayer");
    listOfPlayList = settingsObject->value("List Of PlayList").toHash();
    if (audioOutput) {
        audioOutput->setMuted(settingsObject->value("muted", false).toBool());
        audioOutput->setVolume(settingsObject->value("volume", 0.4).toReal());
    }
    settingsObject->endGroup();
}

void QMusicPlayer::savePlayerSettings(QSettings* settingsObject)
{
    settingsObject->beginGroup("QMusicPlayer");
    settingsObject->setValue("List Of PlayList", listOfPlayList);
    if (audioOutput) {
        settingsObject->setValue("muted", audioOutput->isMuted());
        settingsObject->setValue("volume", audioOutput->volume());
    }
    settingsObject->endGroup();
}

void QMusicPlayer::loadPlayList(const QString &fileName)
{
    /*******************************************************************************/
    //tags for Version-0.1 of Saaghar media playlist
    //#SAAGHAR!PLAYLIST!            //start of playlist
    //#SAAGHAR!PLAYLIST!TITLE!      //title tag
    //#SAAGHAR!ITEMS!               //start of items
    //#SAAGHAR!TITLE!               //item's title tag
    //#SAAGHAR!ID!                  //item's id tag
    //#SAAGHAR!PATH!                //item's full path
    //#SAAGHAR!MD5SUM!              //item's MD5SUM hash
    //<last property>               //item's path relative to playlist file
    /*******************************************************************************/
    QFile file(fileName);
    if (!file.exists()) {
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::information(this->parentWidget(), tr("Read Error!"), tr("Can't load playlist!\nError: %1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    QString line = out.readLine();
    if (!line.startsWith("#SAAGHAR!PLAYLIST!")) {
        return;
    }

    line = out.readLine();
    while (!line.startsWith("#SAAGHAR!PLAYLIST!TITLE!")) {
        if (out.atEnd()) {
            return;
        }
        line = out.readLine();
    }

    QString playListName = line.remove("#SAAGHAR!PLAYLIST!TITLE!");
    if (playListName.isEmpty()) {
        return;
    }
    SaagharPlayList* playList = new SaagharPlayList;
    playList->PATH = fileName;
    listOfPlayList.insert(playListName, fileName);

    while (!line.startsWith("#SAAGHAR!ITEMS!")) { //start of items
        if (out.atEnd()) {
            return;
        }
        line = out.readLine();
    }

    QFileInfo playListFileInfo(fileName);
    QDir playListDir(playListFileInfo.absolutePath());

    int ID;
    QString TITLE;
    QString FULL_PATH;
    QString MD5SUM;
    bool ok = false;
    while (!out.atEnd()) {
        line = out.readLine();
        if (line.startsWith("#")) {
            if (!line.startsWith("#SAAGHAR!")) {
                continue;
            }
            else {
                line.remove("#SAAGHAR!");
                if (line.startsWith("TITLE!")) {
                    line.remove("TITLE!");
                    TITLE = line;
                }
                else if (line.startsWith("ID!")) {
                    line.remove("ID!");
                    if (line.isEmpty()) {
                        continue;
                    }
                    ID = line.toInt(&ok);
                    if (!ok || ID < 0) {
                        continue;
                    }
                }
                else if (line.startsWith("PATH!")) {
                    line.remove("PATH!");
                    FULL_PATH = line;
                }
                else if (line.startsWith("MD5SUM!")) {
                    line.remove("MD5SUM!");
                    MD5SUM = line;
                }
            }
        }
        else {
            line = line.trimmed();
            if (ID != -1 && (!line.isEmpty() || !FULL_PATH.isEmpty())) {
                //at least one of path and id is necessary!!
                QString absoluteMediaPath = playListDir.absoluteFilePath(line);
                bool mediaExists = false;
                if (QFile::exists(absoluteMediaPath)) {
                    absoluteMediaPath = QDir::cleanPath(absoluteMediaPath);
                    mediaExists = true;
                }
                else if (QFile::exists(FULL_PATH)) {
                    absoluteMediaPath = FULL_PATH;
                    mediaExists = true;
                }

                if (mediaExists) {
                    SaagharMediaTag* mediaTag = new SaagharMediaTag;
                    mediaTag->time = 0;
                    mediaTag->PATH = absoluteMediaPath;
                    mediaTag->TITLE = TITLE;
                    mediaTag->MD5SUM = MD5SUM;
                    playList->mediaItems.insert(ID, mediaTag);
                }
                FULL_PATH.clear();
                TITLE.clear();
                MD5SUM.clear();
                ID = -1;
            }
        }
    }

    file.close();
    qDebug() << "size=" << playList->mediaItems.size() << "name=" << playListName;
    pushPlayList(playList, playListName);

    //playListManager->setPlayLists(hashPlayLists);
    playListManager->setCurrentPlayList(playListName);
}

void QMusicPlayer::savePlayList(const QString &fileName, const QString &playListName, const QString &)
{
    if (playListName.isEmpty()) {
        return;
    }

    SaagharPlayList* playList = playListManager->playListByName(playListName);

    if (!playList) {
        return;
    }

    QString playListFileName = fileName;
    if (playListFileName.isEmpty()) {
        playListFileName = playList->PATH;
    }

    listOfPlayList.insert(playListName, playListFileName);

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::information(this->parentWidget(), tr("Save Error!"), tr("Can't save playlist!\nError: %1").arg(file.errorString()));
        return;
    }

    QFileInfo playListFileInfo(fileName);
    QDir playListInfo(playListFileInfo.absolutePath());
    QTextStream out(&file);
    out.setCodec("UTF-8");
    QString playListContent = QString("#SAAGHAR!PLAYLIST!V%1\n#SAAGHAR!PLAYLIST!TITLE!%2\n##################\n#SAAGHAR!ITEMS!\n").arg("0.1").arg(playListName);
    QHash<int, SaagharMediaTag*>::const_iterator it = playList->mediaItems.constBegin();  //playList.constBegin();
    while (it != playList->mediaItems.constEnd()) {
        QString relativePath = playListInfo.relativeFilePath(it.value()->PATH);
        QString md5sumStr;
        if (!it.value()->MD5SUM.isEmpty()) {
            md5sumStr = QString("#SAAGHAR!MD5SUM!%1\n").arg(it.value()->MD5SUM);
        }
        QString itemStr = QString(
                              "#SAAGHAR!TITLE!%1\n"
                              "#SAAGHAR!ID!%2\n"
                              "%3"
                              "#SAAGHAR!PATH!%4\n"
                              "%5\n")
                          .arg(it.value()->TITLE).arg(it.key()).arg(md5sumStr).arg(it.value()->PATH).arg(relativePath);
        playListContent += itemStr;
        ++it;
    }
    qDebug() << "======================================";
    qDebug() << "playListContent=\n" << playListContent;
    qDebug() << "======================================";
    out << playListContent;
    file.close();
}

void QMusicPlayer::loadAllPlayLists()
{
    QHash<QString, QVariant>::const_iterator playListsIterator = listOfPlayList.constBegin();
    while (playListsIterator != listOfPlayList.constEnd()) {
        if (!playListsIterator.key().isEmpty()) {
            loadPlayList(playListsIterator.value().toString());
        }
        ++playListsIterator;
    }
}

void QMusicPlayer::saveAllPlayLists(const QString &format)
{
    QHash<QString, QVariant>::const_iterator playListsIterator = listOfPlayList.constBegin();
    while (playListsIterator != listOfPlayList.constEnd()) {
        if (!playListsIterator.key().isEmpty()) {
            savePlayList(playListsIterator.value().toString(), playListsIterator.key(), format);
        }
        ++playListsIterator;
    }
}

/*static*/
void QMusicPlayer::removeFromPlayList(int mediaID, QString playListName)
{
    if (!playListName.isEmpty()) {
        SaagharPlayList* playList = playListManager->playListByName(playListName);
        if (playList) {
            qDebug() << "removeFromPlayList" << mediaID << "removed";
            playList->mediaItems.remove(mediaID);
        }
        return;
    }

    while (playListContains(mediaID, &playListName)) {
        SaagharPlayList* playList = playListManager->playListByName(playListName);
        if (playList) {
            qDebug() << "removeFromPlayList" << mediaID << "removed";
            playList->mediaItems.remove(mediaID);
        }
    }
}

/*static*/
void QMusicPlayer::insertToPlayList(int mediaID, const QString &mediaPath, const QString &mediaTitle,
                                    int mediaCurrentTime, QString playListName)
{
    if (playListName.isEmpty()) {
        playListName = playListManager->currentPlayListName();
    }

    SaagharPlayList* playList = playListManager->playListByName(playListName);

    QFile file(mediaPath);
    if (!file.exists() || mediaID < 0) {
        return;
    }

    SaagharMediaTag* mediaTag = new SaagharMediaTag;
    mediaTag->TITLE = mediaTitle;
    mediaTag->PATH = mediaPath;
    mediaTag->time = mediaCurrentTime;

    /**********************************************************/
//compute media file MD5SUM, reserved for future versions
//  QFile mediaFile(mediaPath);
//  if (mediaFile.open(QFile::ReadOnly))
//      mediaTag->MD5SUM = QString(
//      QCryptographicHash::hash(mediaFile.readAll(),
//      QCryptographicHash::Md5).toHex());
    /**********************************************************/

    if (!playList) {
        playList = new SaagharPlayList;
        playList->PATH = listOfPlayList.value(playListName).toString();
        playList->mediaItems.insert(mediaID, mediaTag);
        pushPlayList(playList, playListName);
    }
    else {
        playList->mediaItems.insert(mediaID, mediaTag);
    }
}

/*static*/
void QMusicPlayer::getFromPlayList(int mediaID, QString* mediaPath, QString* mediaTitle,
                                   int* mediaCurrentTime, QString* playListName)
{
    QString temp;
    bool playListNameIsNull = false;
    if (!playListName) {
        playListNameIsNull = true;
        playListName = &temp;
    }

    if (mediaID < 0 || !mediaPath || !playListContains(mediaID, playListName)) {
        return;
    }

    SaagharMediaTag* mediaTag =  playListManager->playListByName(*playListName)->mediaItems.value(mediaID);
    if (!mediaTag) {
        mediaPath->clear();
        mediaTitle->clear();
        *mediaCurrentTime = 0;
        if (playListNameIsNull) {
            playListName = 0;
        }
        else {
            playListName->clear();
        }
        return;
    }

    *mediaPath = mediaTag->PATH;
    if (mediaTitle) {
        *mediaTitle = mediaTag->TITLE;
    }
    if (mediaCurrentTime) {
        *mediaCurrentTime = mediaTag->time;
    }
    if (playListNameIsNull) {
        playListName = 0;
    }
}

bool QMusicPlayer::playListContains(int mediaID, QString* playListName)
{//playListName default can be NULL for test purposes we don't set it
    QStringList names = hashPlayLists.keys();
    if (names.removeAll(playListManager->currentPlayListName()) > 0) {
        names.prepend(playListManager->currentPlayListName());
    }

    for (int i = 0; i < names.size(); ++i) {
        SaagharPlayList* playList = playListManager->playListByName(names.at(i));

        qDebug() << "playListName=" << names.at(i) << "playListContains:\nmediaID=" << mediaID << "PlayLists=" << hashPlayLists.values() << "IDs=" << hashPlayLists.keys();
        if (!playList) {
            continue;
        }
        qDebug() << "mediaID=" << mediaID << "pointer=" << playList << "*po=" << playList->mediaItems.keys();
        bool result = playList->mediaItems.contains(mediaID);
        if (result) {
            if (playListName) {
                *playListName = names.at(i);
            }
            qDebug() << "::playListContains--TRUE";
            return true;
        }
    }

    return false;
}

QDockWidget* QMusicPlayer::playListManagerDock()
{
    if (!playListManager) {
        return 0;
    }
    if (dockList) {
        return dockList;
    }
    dockList = new QDockWidget;
    dockList->setObjectName("PlayListManagerDock");
    dockList->setStyleSheet("QDockWidget::title { background: transparent; text-align: left; padding: 0 10 0 10;}"
                            "QDockWidget::close-button, QDockWidget::float-button { background: transparent;}");
    dockList->setWindowTitle(tr("PlayList"));
    dockList->setWidget(playListManager);
    return dockList;
}

void QMusicPlayer::playMedia(int mediaID)
{
    disconnect(this, SIGNAL(mediaChanged(QString,QString,int,bool)), playListManager, SLOT(currentMediaChanged(QString,QString,int,bool)));
    emit requestPageContainedMedia(mediaID, true);
    mediaObject->play();
    connect(this, SIGNAL(mediaChanged(QString,QString,int,bool)), playListManager, SLOT(currentMediaChanged(QString,QString,int,bool)));
}

void QMusicPlayer::resizeEvent(QResizeEvent* e)
{
    QToolBar::resizeEvent(e);

    infoLabel->resize(seekSlider->width(), infoLabel->height());
}

/*static*/
QStringList QMusicPlayer::commonSupportedMedia(const QString &type)
{
    QStringList supportedExtentions;
    const QStringList commonMediaExtentions = QStringList()
            << "mp3" << "wav" << "wma" << "ogg" << "mp4" << "mpg" << "mid"
            << "asf" << "3gp" << "wmv" << "avi";

    const QString sep = "|";
    QString supportedMimeTypes = Phonon::BackendCapabilities::availableMimeTypes().join(sep);
    for (int i = 0; i < commonMediaExtentions.size(); ++i) {
        QString extention = commonMediaExtentions.at(i);
        if (supportedMimeTypes.contains(QRegExp(QString("(%1/[^%2]*%3[^%2]*)").arg(type).arg(sep).arg(extention)))) {
            if (type == "audio" && (extention == "mpg" || extention == "3gp")) {
                continue;
            }

            supportedExtentions << "*." + extention;
        }
    }
    qDebug() << "commonSupportedMedia-" << type << "=" << supportedExtentions;
    return supportedExtentions;
}

void QMusicPlayer::stop()
{
    if (mediaObject) {
        mediaObject->stop();
    }
    infoLabel->setText("");
}

/*******************************
// class PlayListManager
********************************/

#include <QTreeWidget>
#include <QVBoxLayout>

PlayListManager::PlayListManager(QWidget* parent) : QWidget(parent)
{
    setObjectName("PlayListManager");

    mediaList = new QTreeWidget;
    mediaList->setLayoutDirection(Qt::RightToLeft);
    mediaList->headerItem()->setHidden(true);
    previousItem = 0;
    mediaList->setColumnCount(1);
    hLayout = new QVBoxLayout;

    hLayout->addWidget(mediaList);
    this->setLayout(hLayout);
    connect(mediaList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemPlayRequested(QTreeWidgetItem*,int)));
}

void PlayListManager::setPlayLists(const QHash<QString, QMusicPlayer::SaagharPlayList*> &playLists, bool /*justMediaList*/)
{
    if (playLists.isEmpty()) {
        return;
    }
    //if (justMediaList) //not implemented!
//    itemsAsTopItem = playLists.size() == 1 ? true : false;
    //temp simpler playlist management for v2.0
    previousItem = 0;
    mediaList->clear();
    qDebug() << Q_FUNC_INFO;
//    itemsAsTopItem = true;
    QHash<QString, QMusicPlayer::SaagharPlayList*>::const_iterator playListIterator = playLists.constBegin();
    while (playListIterator != playLists.constEnd()) {
        QMusicPlayer::SaagharPlayList* playList = playListIterator.value();
        if (!playList) {
            ++playListIterator;
            continue;
        }

        QTreeWidgetItem* playListRoot = 0;
        if (!itemsAsTopItem) {
            playListRoot = new QTreeWidgetItem();
            mediaList->addTopLevelItem(playListRoot);
            playListRoot->setText(0, playListIterator.key());
        }

        QHash<int, QMusicPlayer::SaagharMediaTag*> items = playList->mediaItems;
        QHash<int, QMusicPlayer::SaagharMediaTag*>::const_iterator mediaIterator = items.constBegin();
        while (mediaIterator != items.constEnd()) {
            QMusicPlayer::SaagharMediaTag* mediaTag = mediaIterator.value();
            if (!mediaTag) {
                ++mediaIterator;
                continue;
            }

            QTreeWidgetItem* mediaItem = new QTreeWidgetItem();
            mediaItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
            mediaItem->setText(0, mediaTag->TITLE);
//          mediaItem->setText(1, mediaTag->PATH);
            mediaItem->setData(0, Qt::UserRole, mediaIterator.key());

            if (!itemsAsTopItem) {
                playListRoot->addChild(mediaItem);
            }
            else {
                mediaList->addTopLevelItem(mediaItem);
            }
            ++mediaIterator;
        }
        ++playListIterator;
    }
}

void PlayListManager::setCurrentPlayList(const QString &playListName)
{
    if (playListName.isEmpty() || playListName == m_currentPlayList) {
        return;
    }

    m_currentPlayList = playListName;

    QMusicPlayer::SaagharPlayList* playList = playListByName(playListName);
    if (!playList) {
        return;
    }

    previousItem = 0;
    mediaList->clear();
    QHash<int, QMusicPlayer::SaagharMediaTag*> items = playList->mediaItems;
    QHash<int, QMusicPlayer::SaagharMediaTag*>::const_iterator mediaIterator = items.constBegin();
    while (mediaIterator != items.constEnd()) {
        QMusicPlayer::SaagharMediaTag* mediaTag = mediaIterator.value();
        if (!mediaTag) {
            ++mediaIterator;
            continue;
        }

        QTreeWidgetItem* mediaItem = new QTreeWidgetItem(mediaList);
        mediaItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
        mediaItem->setText(0, mediaTag->TITLE);
        //          mediaItem->setText(1, mediaTag->PATH);
        mediaItem->setData(0, Qt::UserRole, mediaIterator.key());

        mediaList->addTopLevelItem(mediaItem);
        ++mediaIterator;
    }
}

QMusicPlayer::SaagharPlayList* PlayListManager::playListByName(QString playListName)
{
    if (playListName.isEmpty()) {
        playListName = m_currentPlayList;
    }

    return QMusicPlayer::playListsHash().value(playListName);
}

void PlayListManager::currentMediaChanged(const QString &fileName, const QString &title, int mediaID, bool removeRequest)
{
    if (mediaID <= 0) { // fileName.isEmpty() for delete request!!
        return;
    }

    for (int i = 0; i < mediaList->topLevelItemCount(); ++i) {
        QTreeWidgetItem* rootItem = mediaList->topLevelItem(i);
        if (!rootItem) {
            continue;
        }
        int end = rootItem->childCount();
        if (itemsAsTopItem) {
            end = 1;
        }
        for (int j = 0; j < end; ++j) {
            QTreeWidgetItem* childItem;
            if (itemsAsTopItem) {
                childItem = rootItem;
            }
            else {
                childItem = rootItem->child(j);
            }

            if (!childItem) {
                continue;
            }
            int childID = childItem->data(0, Qt::UserRole).toInt();
            if (childID != mediaID) {
                continue;
            }
            else {
                if (removeRequest) {
                    if (previousItem == childItem) {
                        previousItem = 0;
                    }
                    rootItem->takeChild(j);
                    delete childItem;
                    childItem = 0;
                }
                else if (!fileName.isEmpty()) {
                    if (!title.isEmpty()) {
                        childItem->setText(0, title);
                    }
//                  childItem->setText(1, fileName);
                    if (playListMediaObject && playListMediaObject->state() == Phonon::PlayingState) {
                        childItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPlay));
                    }
                    if (mediaList->currentItem()) {
                        mediaList->currentItem()->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
                    }
                    mediaList->setCurrentItem(childItem, 0);
                    previousItem = childItem;
                }
                return;
            }
        }
    }

    if (!fileName.isEmpty()) {
        QTreeWidgetItem* rootItem = 0;
        if (!itemsAsTopItem) {
            rootItem = mediaList->topLevelItem(0);
        }

        if (rootItem || itemsAsTopItem) {
            QTreeWidgetItem* newChild = new QTreeWidgetItem;
            newChild->setText(0, title);
//          newChild->setText(1, fileName);
            newChild->setData(0, Qt::UserRole, mediaID);
            if (playListMediaObject && playListMediaObject->state() == Phonon::PlayingState) {
                newChild->setIcon(0, style()->standardIcon(QStyle::SP_MediaPlay));
            }

            if (rootItem) {
                rootItem->addChild(newChild);
            }
            else { //itemsAsTopItem == true
                mediaList->addTopLevelItem(newChild);
            }

            if (mediaList->currentItem()) {
                mediaList->currentItem()->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
            }
            mediaList->setCurrentItem(newChild, 0);
            previousItem = newChild;
        }
    }
}

void PlayListManager::itemPlayRequested(QTreeWidgetItem* item, int /*col*/)
{
    if (!item) {
        return;
    }

    if (item->childCount() > 0) {
        return;    //root item
    }

    if (previousItem) {
        previousItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
    }

    item->setIcon(0, style()->standardIcon(QStyle::SP_MediaPlay));
    int id = item->data(0, Qt::UserRole).toInt();
    previousItem = item;
    emit mediaPlayRequested(id);
}

void PlayListManager::setMediaObject(Phonon::MediaObject* MediaObject)
{
    playListMediaObject = MediaObject;
    connect(playListMediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(mediaObjectStateChanged(Phonon::State,Phonon::State)));
}

void PlayListManager::mediaObjectStateChanged(Phonon::State newState, Phonon::State)
{
    if (previousItem) {
        if (newState == Phonon::PlayingState) {
            previousItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPlay));
        }
        else {
            previousItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
        }
    }
}


/***********************************************************
// class ScrollText by Sebastian Lehmann <http://l3.ms>
// link: http://stackoverflow.com/a/10655396
************************************************************/
#include <QPainter>

ScrollText::ScrollText(QWidget* parent) :
    QWidget(parent), scrollPos(0)
{
#if QT_VERSION < 0x040700
    staticText.hide();
#endif

    staticText.setTextFormat(Qt::PlainText);

    setFixedHeight(fontMetrics().height());
    leftMargin = height() / 3;

    setSeparator("   ---   ");

    connect(&timer, SIGNAL(timeout()), this, SLOT(timer_timeout()));
    timer.setInterval(60);
}

QString ScrollText::text() const
{
    return _text;
}

void ScrollText::setText(QString text)
{
    _text = text;
    updateText();
    update();
}

QString ScrollText::separator() const
{
    return _separator;
}

void ScrollText::setSeparator(QString separator)
{
    _separator = separator;
    updateText();
    update();
}

void ScrollText::updateText()
{
    timer.stop();

    singleTextWidth = fontMetrics().width(_text);
    scrollEnabled = (singleTextWidth > width() - leftMargin);

    if (scrollEnabled) {
        scrollPos = -64;
        staticText.setText(_text + _separator);
        timer.start();
    }
    else {
        staticText.setText(_text);
    }
#if QT_VERSION >= 0x040700
    staticText.prepare(QTransform(), font());
#endif
    wholeTextSize = QSize(fontMetrics().width(staticText.text()), fontMetrics().height());
}

void ScrollText::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    if (scrollEnabled) {
        buffer.fill(qRgba(0, 0, 0, 0));
        QPainter pb(&buffer);
        pb.setPen(p.pen());
        pb.setFont(p.font());

        int x = qMin(-scrollPos, 0) + leftMargin;
        while (x < width()) {
#if QT_VERSION >= 0x040700
            pb.drawStaticText(QPointF(x, (height() - wholeTextSize.height()) / 2), staticText);
#else
            pb.drawText(QPointF(x, height() - 2 - ((height() - wholeTextSize.height()) / 2)), staticText.text());
#endif
            x += wholeTextSize.width();
        }

        //Apply Alpha Channel
        pb.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pb.setClipRect(width() - 15, 0, 15, height());
        pb.drawImage(0, 0, alphaChannel);
        pb.setClipRect(0, 0, 15, height());
        //initial situation: don't apply alpha channel in the left half of the image at all; apply it more and more until scrollPos gets positive
        if (scrollPos < 0) {
            pb.setOpacity((qreal)(qMax(-8, scrollPos) + 8) / 8.0);
        }
        pb.drawImage(0, 0, alphaChannel);

        p.drawImage(0, 0, buffer);
    }
    else {
#if QT_VERSION >= 0x040700
        p.drawStaticText(QPointF(leftMargin + (width() - (wholeTextSize.width() + leftMargin)) / 2, (height() - wholeTextSize.height()) / 2), staticText);
#else
        p.drawText(QPointF(leftMargin + (width() - (wholeTextSize.width() + leftMargin)) / 2, height() - 2 - ((height() - wholeTextSize.height()) / 2)), staticText.text());
#endif
    }
}

void ScrollText::resizeEvent(QResizeEvent*)
{
    //When the widget is resized, we need to update the alpha channel.
    alphaChannel = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);

    //Create Alpha Channel:
    if (width() > 64) {
        //create first scanline
        QRgb* scanline1 = (QRgb*)alphaChannel.scanLine(0);
        for (int x = 1; x < 16; ++x) {
            scanline1[x - 1] = scanline1[width() - x] = qRgba(0, 0, 0, x << 4);
        }
        for (int x = 15; x < width() - 15; ++x) {
            scanline1[x] = qRgb(0, 0, 0);
        }
        //copy scanline to the other ones
        for (int y = 1; y < height(); ++y) {
            memcpy(alphaChannel.scanLine(y), (uchar*)scanline1, width() * 4);
        }
    }
    else {
        alphaChannel.fill(qRgb(0, 0, 0));
    }


    //Update scrolling state
    bool newScrollEnabled = (singleTextWidth > width() - leftMargin);
    if (newScrollEnabled != scrollEnabled) {
        updateText();
    }
}

void ScrollText::timer_timeout()
{
    scrollPos = (scrollPos + 2)
                % wholeTextSize.width();
    update();
}

// //for future
// //Windows 7 volume notification code
//static const GUID AudioSessionVolumeCtx =
//{ 0x2715279f, 0x4139, 0x4ba0, { 0x9c, 0xb1, 0xb3, 0x51, 0xf1, 0xb5, 0x8a, 0x4a } };


//CAudioSessionVolume::CAudioSessionVolume(
//  UINT uNotificationMessage,
//  HWND hwndNotification
//  )
//  : m_cRef(1),
//    m_uNotificationMessage(uNotificationMessage),
//    m_hwndNotification(hwndNotification),
//    m_bNotificationsEnabled(FALSE),
//    m_pAudioSession(NULL),
//    m_pSimpleAudioVolume(NULL)
//{
//}

//CAudioSessionVolume::~CAudioSessionVolume()
//{
//  EnableNotifications(FALSE);

//  SafeRelease(&m_pAudioSession);
//  SafeRelease(&m_pSimpleAudioVolume);
//};


////  Creates an instance of the CAudioSessionVolume object.

///* static */
//HRESULT CAudioSessionVolume::CreateInstance(
//  UINT uNotificationMessage,
//  HWND hwndNotification,
//  CAudioSessionVolume **ppAudioSessionVolume
//  )
//{

//  CAudioSessionVolume *pAudioSessionVolume = new (std::nothrow)
//      CAudioSessionVolume(uNotificationMessage, hwndNotification);

//  if (pAudioSessionVolume == NULL)
//  {
//      return E_OUTOFMEMORY;
//  }

//  HRESULT hr = pAudioSessionVolume->Initialize();
//  if (SUCCEEDED(hr))
//  {
//      *ppAudioSessionVolume = pAudioSessionVolume;
//  }
//  else
//  {
//      pAudioSessionVolume->Release();
//  }

//  return hr;
//}


////  Initializes the CAudioSessionVolume object.

//HRESULT CAudioSessionVolume::Initialize()
//{
//  HRESULT hr = S_OK;

//  IMMDeviceEnumerator *pDeviceEnumerator = NULL;
//  IMMDevice *pDevice = NULL;
//  IAudioSessionManager *pAudioSessionManager = NULL;

//  // Get the enumerator for the audio endpoint devices.
//  hr = CoCreateInstance(
//      __uuidof(MMDeviceEnumerator),
//      NULL,
//      CLSCTX_INPROC_SERVER,
//      IID_PPV_ARGS(&pDeviceEnumerator)
//      );

//  if (FAILED(hr))
//  {
//      goto done;
//  }

//  // Get the default audio endpoint that the SAR will use.
//  hr = pDeviceEnumerator->GetDefaultAudioEndpoint(
//      eRender,
//      eConsole,   // The SAR uses 'eConsole' by default.
//      &pDevice
//      );

//  if (FAILED(hr))
//  {
//      goto done;
//  }

//  // Get the session manager for this device.
//  hr = pDevice->Activate(
//      __uuidof(IAudioSessionManager),
//      CLSCTX_INPROC_SERVER,
//      NULL,
//      (void**) &pAudioSessionManager
//      );

//  if (FAILED(hr))
//  {
//      goto done;
//  }

//  // Get the audio session.
//  hr = pAudioSessionManager->GetAudioSessionControl(
//      &GUID_NULL,     // Get the default audio session.
//      FALSE,          // The session is not cross-process.
//      &m_pAudioSession
//      );


//  if (FAILED(hr))
//  {
//      goto done;
//  }

//  hr = pAudioSessionManager->GetSimpleAudioVolume(
//      &GUID_NULL, 0, &m_pSimpleAudioVolume
//      );

//done:
//  SafeRelease(&pDeviceEnumerator);
//  SafeRelease(&pDevice);
//  SafeRelease(&pAudioSessionManager);
//  return hr;
//}

//STDMETHODIMP CAudioSessionVolume::QueryInterface(REFIID riid, void **ppv)
//{
//  static const QITAB qit[] =
//  {
//      QITABENT(CAudioSessionVolume, IAudioSessionEvents),
//      { 0 },
//  };
//  return QISearch(this, qit, riid, ppv);
//}

//STDMETHODIMP_(ULONG) CAudioSessionVolume::AddRef()
//{
//  return InterlockedIncrement(&m_cRef);
//}

//STDMETHODIMP_(ULONG) CAudioSessionVolume::Release()
//{
//  LONG cRef = InterlockedDecrement( &m_cRef );
//  if (cRef == 0)
//  {
//      delete this;
//  }
//  return cRef;
//}


//// Enables or disables notifications from the audio session. For example, the
//// application is notified if the user mutes the audio through the system
//// volume-control program (Sndvol).

//HRESULT CAudioSessionVolume::EnableNotifications(BOOL bEnable)
//{
//  HRESULT hr = S_OK;

//  if (m_hwndNotification == NULL || m_pAudioSession == NULL)
//  {
//      return E_FAIL;
//  }

//  if (m_bNotificationsEnabled == bEnable)
//  {
//      // No change.
//      return S_OK;
//  }

//  if (bEnable)
//  {
//      hr = m_pAudioSession->RegisterAudioSessionNotification(this);
//  }
//  else
//  {
//      hr = m_pAudioSession->UnregisterAudioSessionNotification(this);
//  }

//  if (SUCCEEDED(hr))
//  {
//      m_bNotificationsEnabled = bEnable;
//  }

//  return hr;
//}


//// Gets the session volume level.

//HRESULT CAudioSessionVolume::GetVolume(float *pflVolume)
//{
//  if ( m_pSimpleAudioVolume == NULL)
//  {
//      return E_FAIL;
//  }
//  else
//  {
//      return m_pSimpleAudioVolume->GetMasterVolume(pflVolume);
//  }
//}

////  Sets the session volume level.
////
////  flVolume: Ranges from 0 (silent) to 1 (full volume)

//HRESULT CAudioSessionVolume::SetVolume(float flVolume)
//{
//  if (m_pSimpleAudioVolume == NULL)
//  {
//      return E_FAIL;
//  }
//  else
//  {
//      return m_pSimpleAudioVolume->SetMasterVolume(
//          flVolume,
//          &AudioSessionVolumeCtx  // Event context.
//          );
//  }
//}


////  Gets the muting state of the session.

//HRESULT CAudioSessionVolume::GetMute(BOOL *pbMute)
//{
//  if (m_pSimpleAudioVolume == NULL)
//  {
//      return E_FAIL;
//  }
//  else
//  {
//      return m_pSimpleAudioVolume->GetMute(pbMute);
//  }
//}

////  Mutes or unmutes the session audio.

//HRESULT CAudioSessionVolume::SetMute(BOOL bMute)
//{
//  if (m_pSimpleAudioVolume == NULL)
//  {
//      return E_FAIL;
//  }
//  else
//  {
//      return m_pSimpleAudioVolume->SetMute(
//          bMute,
//          &AudioSessionVolumeCtx  // Event context.
//          );
//  }
//}

////  Sets the display name for the session audio.

//HRESULT CAudioSessionVolume::SetDisplayName(const WCHAR *wszName)
//{
//  if (m_pAudioSession == NULL)
//  {
//      return E_FAIL;
//  }
//  else
//  {
//      return m_pAudioSession->SetDisplayName(wszName, NULL);
//  }
//}


////  Called when the session volume level or muting state changes.
////  (Implements IAudioSessionEvents::OnSimpleVolumeChanged.)

//HRESULT CAudioSessionVolume::OnSimpleVolumeChanged(
//  float NewVolume,
//  BOOL NewMute,
//  LPCGUID EventContext
//  )
//{
//  // Check if we should post a message to the application.

//  if ( m_bNotificationsEnabled &&
//      (*EventContext != AudioSessionVolumeCtx) &&
//      (m_hwndNotification != NULL)
//      )
//  {
//      // Notifications are enabled, AND
//      // We did not trigger the event ourselves, AND
//      // We have a valid window handle.

//      // Post the message.
//      ::PostMessage(
//          m_hwndNotification,
//          m_uNotificationMessage,
//          *((WPARAM*)(&NewVolume)),  // Coerce the float.
//          (LPARAM)NewMute
//          );
//  }
//  return S_OK;
//}
