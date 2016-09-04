/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2015 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).       *
 * All rights reserved.                                                    *
 * Contact: Nokia Corporation (qt-info@nokia.com)                          *
 *                                                                         *
 * This file is part of the examples of the Qt Toolkit.                    *
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
#include "lyricsmanager.h"
#include "settingsmanager.h"

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
#include <QInputDialog>
#include <QComboBox>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QUuid>
#include <QCryptographicHash>

#define TICK_INTERVAL 100

#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

QHash<QString, QVariant> QMusicPlayer::albumsPathList = QHash<QString, QVariant>();
QHash<QString, QMusicPlayer::SaagharAlbum*> QMusicPlayer::albumsMediaHash = QHash<QString, QMusicPlayer::SaagharAlbum*>();


const bool itemsAsTopItem = true;

QMusicPlayer::QMusicPlayer(QWidget* parent)
    : QToolBar(parent)
    , albumManager(new AlbumManager(this, parent))
    , m_lyricSyncer(0)
    , m_lyricReader(new LyricsManager(this))
    , m_lastVorder(-2)
    , m_lyricSyncerRuninng(false)
{
    setObjectName("QMusicPlayer");

    setStyleSheet("background-image:url(\":/resources/images/transp.png\"); border:none;");

    notLoaded = true;
    dockList = 0;

    connect(albumManager, SIGNAL(mediaPlayRequested(int)), this, SLOT(playMedia(int)));
    connect(this, SIGNAL(mediaChanged(QString,QString,int,bool)), albumManager, SLOT(currentMediaChanged(QString,QString,int,bool)));

    _newTime = -1;

#if QT_VERSION < 0x050000
    startDir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
#else
    startDir = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).isEmpty() ?
               QDir::homePath() : QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0);
#endif

#if USE_PHONON
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    QList<Phonon::AudioOutputDevice> audioOutputDevices = Phonon::BackendCapabilities::availableAudioOutputDevices();
    foreach (const Phonon::AudioOutputDevice newAudioOutput, audioOutputDevices) {
        if (newAudioOutput.name().contains("waveout", Qt::CaseInsensitive)) {
            audioOutput->setOutputDevice(newAudioOutput);
            break;
        }
    }

#ifdef SAAGHAR_DEBUG
    qDebug() << "Audio Output Devices:" << audioOutputDevices
             << "\nSelected Output Device:" << audioOutput->outputDevice().name();
#endif

    mediaObject = new Phonon::MediaObject(this);
    metaInformationResolver = new Phonon::MediaObject(this);
    Phonon::createPath(mediaObject, audioOutput);
#else
    mediaObject = new QMediaPlayer(this, QMediaPlayer::StreamPlayback);
    metaInformationResolver = new QMediaPlayer(this, QMediaPlayer::StreamPlayback);
#endif

    albumManager->setMediaObject(mediaObject);

    setupActions();
    setupUi();
    timeLcd->display("00:00");

    createConnections();

    playRequestedByUser(false);
    stateChange();
}

void QMusicPlayer::seekOnStateChange()
{
    if (_newTime > 0) {
#ifdef USE_PHONON
        mediaObject->seek(_newTime);
#else
        mediaObject->setPosition(_newTime);
#endif
        _newTime = -1;
    }
}

qint64 QMusicPlayer::currentTime()
{
#ifdef USE_PHONON
    return mediaObject->currentTime();
#else
    return mediaObject->position();
#endif
}

void QMusicPlayer::setCurrentTime(qint64 time)
{
    _newTime = time;

    seekOnStateChange();
}

QString QMusicPlayer::source()
{
#ifdef USE_PHONON
    return metaInformationResolver->currentSource().fileName();
#else
    return metaInformationResolver->currentMedia().canonicalUrl().toLocalFile();
#endif
}

void QMusicPlayer::setSource(const QString &fileName, const QString &title, int mediaID, bool newSource)
{
    if (m_lyricSyncerRuninng) {
        stopLyricSyncer();
    }
    _newTime = -1;

#ifdef USE_PHONON
    Phonon::MediaSource source(fileName);
#else
    QMediaContent source(fileName);
#endif

    sources.clear();
    infoLabel->setText("");

    if (!fileName.isEmpty()) {
        notLoaded = false;
        sources.append(source);
#ifdef USE_PHONON
        metaInformationResolver->setCurrentSource(source);
#else
        metaInformationResolver->setMedia(source);
#endif
        load(0);
        if (mediaID > 0) {
            QString albumName;
            albumContains(mediaID, &albumName);
            if (!albumName.isEmpty() && !newSource) {
                albumManager->setCurrentMedia(mediaID, fileName);
                albumManager->setCurrentAlbum(albumName);
            }
        }

        stateChange();
    }
    else {
        notLoaded = true;
#ifdef USE_PHONON
        mediaObject->clear();
        metaInformationResolver->clear();
#else
        mediaObject->setMedia(QMediaContent());
        metaInformationResolver->setMedia(QMediaContent());
#endif
    }

    if (!title.isEmpty()) {
        currentTitle = title;
    }
    currentID = mediaID;
    emit mediaChanged(fileName, currentTitle, currentID, false);

    QString gLyricName = fileName.left(fileName.lastIndexOf(QLatin1Char('.'))) + QLatin1String(".xml");

    QFile file(gLyricName);
    setLyricSyncerState(true, !file.exists());

#ifdef USE_PHONON
    if (!fileName.isEmpty() && m_lyricReader->read(&file, "GANJOOR_XML")) {
        mediaObject->setTickInterval(TICK_INTERVAL);
        connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(showTextByTime(qint64)));
        connect(this, SIGNAL(highlightedTextChange(QString)), infoLabel, SLOT(setText(QString)));
    }
    else {
        mediaObject->setTickInterval(0);
        disconnect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(showTextByTime(qint64)));
        disconnect(this, SIGNAL(highlightedTextChange(QString)), infoLabel, SLOT(setText(QString)));
    }
#else
    if (!fileName.isEmpty() && m_lyricReader->read(&file, "GANJOOR_XML")) {
        mediaObject->setNotifyInterval(TICK_INTERVAL);
        connect(mediaObject, SIGNAL(positionChanged(qint64)), this, SLOT(showTextByTime(qint64)));
        connect(this, SIGNAL(highlightedTextChange(QString)), infoLabel, SLOT(setText(QString)));
    }
    else {
        mediaObject->setNotifyInterval(0);
        disconnect(mediaObject, SIGNAL(positionChanged(qint64)), this, SLOT(showTextByTime(qint64)));
        disconnect(this, SIGNAL(highlightedTextChange(QString)), infoLabel, SLOT(setText(QString)));
    }
#endif
}

void QMusicPlayer::setSource()
{
#ifdef SAAGHAR_DEBUG
    qDebug() << "setSource=" << currentID
#ifdef USE_PHONON
             << "\n=====================\nAvailable Mime Types=\n"
             << Phonon::BackendCapabilities::availableMimeTypes()
#endif
             << "\n=====================";
#endif

    QString file = QFileDialog::getOpenFileName(this, tr("Select Music Files"), startDir,
                   "Common Supported Files (" + QMusicPlayer::commonSupportedMedia().join(" ") +
                   ");;Audio Files (" + QMusicPlayer::commonSupportedMedia("audio").join(" ") +
                   ");;Video Files (Audio Stream) (" + QMusicPlayer::commonSupportedMedia("video").join(" ") +
                   ");;All Files (*.*)");

    if (file.isEmpty()) {
        return;
    }

    QFileInfo selectedFile(file);
    startDir = selectedFile.absolutePath();

    setSource(file, currentTitle, currentID, true);
}

void QMusicPlayer::newAlbum(QString fileName, QString albumName)
{
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(window(), tr("New Saaghar Album"), startDir,
                                                "Saaghar Album (*.sal *.m3u8 *.m3u);;All Files (*.*)");
        startDir = QFileInfo(fileName).absolutePath();
    }

    bool alreadyLoaded = albumsPathList.values().contains(fileName);
    if (fileName.isEmpty() || alreadyLoaded) {
        if (alreadyLoaded) {
            QMessageBox::information(window(), tr("Warning!"), tr("The album already loaded."));
        }
        return;
    }

    QFileInfo selectedFile(fileName);
    if (albumName.isEmpty()) {
        albumName = QInputDialog::getText(window(), tr("Name Of Album"),
                                          tr("Enter name for this Album:"),
                                          QLineEdit::Normal, selectedFile.baseName());
    }

    SaagharAlbum* album = new SaagharAlbum;
    album->PATH = fileName;
    pushAlbum(album, albumName);
    saveAlbum(fileName, albumName);
    albumManager->setCurrentAlbum(albumName);
}

void QMusicPlayer::loadAlbumFile()
{
    QString file = QFileDialog::getOpenFileName(window(), tr("Select Saaghar Album"), startDir,
                   "Saaghar Album (*.sal *.m3u8 *.m3u);;All Files (*.*)");

    if (file.isEmpty()) {
        return;
    }

    QFileInfo selectedFile(file);
    startDir = selectedFile.absolutePath();

    loadAlbum(file);
}

void QMusicPlayer::renameAlbum(const QString &albumName)
{
    int index = albumManager->albumList()->findText(albumName);
    if (index < 0 || albumManager->albumList()->itemData(index).isValid()) {
        return;
    }

    QString oldName = albumManager->albumList()->itemText(index);
    QString newName = QInputDialog::getText(window(), tr("Name Of Album"),
                                            tr("Enter new name for this Album:"),
                                            QLineEdit::Normal, oldName);
    if (newName.isEmpty() || newName == oldName ||
            albumsPathList.contains(newName)) {
        return;
    }

    albumsPathList.insert(newName, albumsPathList.value(oldName));
    albumsPathList.remove(oldName);

    albumsMediaHash.insert(newName, albumsMediaHash.value(oldName));
    albumsMediaHash.remove(oldName);

    bool isCurrentIndex = false;
    if (albumManager->currentAlbumName() == oldName) {
        isCurrentIndex = true;
        albumManager->albumList()->setCurrentIndex(index);
    }

    albumManager->albumList()->removeItem(index);
    albumManager->albumList()->insertItem(index, newName);

    if (isCurrentIndex) {
        albumManager->albumList()->setCurrentIndex(index);
    }
}

void QMusicPlayer::removeAlbum(const QString &albumName)
{
    if (albumsPathList.size() == 1) {
        return;
    }

    SaagharAlbum* album = albumManager->albumByName(albumName);
    if (!album) {
        return;
    }

    if (QMessageBox::information(window(), tr("Warning!"),
                                 tr("Are you sure to remove \"%1\" from album list?").arg(albumName),
                                 QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        return;
    }

    if (album->mediaItems.contains(currentID)) {
        removeSource();
    }

    delete album;
    albumsPathList.remove(albumName);
    albumsMediaHash.remove(albumName);

    int index = albumManager->albumList()->findText(albumName);
    albumManager->albumList()->removeItem(index);
}

void QMusicPlayer::saveAsAlbum(const QString &albumName, bool saveAs)
{
    if (!saveAs) {
        saveAlbum(albumsPathList.value(albumName).toString(), albumName);
    }
    else {
        QString file = QFileDialog::getSaveFileName(window(), tr("New Saaghar Album"), startDir,
                       "Saaghar Album (*.sal *.m3u8 *.m3u);;All Files (*.*)");

        bool alreadyLoaded = albumsPathList.values().contains(file);
        if (file.isEmpty() || alreadyLoaded) {
            if (alreadyLoaded) {
                QMessageBox::information(window(), tr("Warning!"), tr("The album already loaded."));
            }
            return;
        }

        startDir = QFileInfo(file).absolutePath();

        QString name = QInputDialog::getText(window(), tr("Name Of Album"),
                                             tr("Enter name for this Album:"),
                                             QLineEdit::Normal, albumName + tr("_Copy"));

        if (name.isEmpty() || name == albumName ||
                albumsPathList.contains(name)) {
            return;
        }

        SaagharAlbum* album = albumManager->albumByName(albumName);
        SaagharAlbum* copyAlbum = new SaagharAlbum;
        copyAlbum->PATH = file;
        copyAlbum->mediaItems = album->mediaItems;
        pushAlbum(copyAlbum, name);
        saveAlbum(file, name);
        albumManager->setCurrentAlbum(name);
    }
}

void QMusicPlayer::showTextByTime(qint64 time)
{
    int vorder = m_lyricReader->vorderByTime(time);

    if (m_lastVorder != vorder) {
        m_lastVorder = vorder;
        if (m_lastVorder >= 0) {
            emit showTextRequested(currentID, vorder);
        }
        else if (m_lastVorder == -1) {
            infoLabel->setText(currentTitle);
        }
        else if (m_lastVorder == -2) {
            infoLabel->setText(tr("Finished"));
        }
        else {
            QMap<QString, QString> metaData;

#ifdef USE_PHONON
            metaData = metaInformationResolver->metaData();
            Phonon::MediaSource source = metaInformationResolver->currentSource();
            const QString &fileName = metaInformationResolver->currentSource().fileName();
#else
            QMediaContent source = metaInformationResolver->currentMedia();
            const QString &fileName = source.canonicalUrl().toLocalFile();

            metaData.insert("TITLE", metaInformationResolver->metaData("Title").toString());
            metaData.insert("ARTIST", metaInformationResolver->metaData("Author").toStringList().join(QLatin1String("-")));
            metaData.insert("ALBUM", metaInformationResolver->metaData("AlbumTitle").toString());
            metaData.insert("DATE", metaInformationResolver->metaData("Date").toDate().toString());
#endif
            QString title = metaData.value("TITLE");
            if (title == "") {
                title = fileName;
            }

            QStringList metaInfos;
            metaInfos << title << metaData.value("ARTIST") << metaData.value("ALBUM") << metaData.value("DATE");
            metaInfos.removeDuplicates();
            QString tmp = metaInfos.join("-");
            metaInfos = tmp.split("-", QString::SkipEmptyParts);
            infoLabel->setText(metaInfos.join(" - "));
        }
    }
}

void QMusicPlayer::removeSource()
{
    _newTime = -1;
    sources.clear();
    infoLabel->setText("");
    notLoaded = true;

#ifdef USE_PHONON
    mediaObject->clear();
    metaInformationResolver->clear();
#else
    mediaObject->setMedia(QMediaContent());
    metaInformationResolver->setMedia(QMediaContent());
#endif

    emit mediaChanged("", "", currentID, true);

    if (m_removeAllSourceAction == sender()) {
        removeFromAlbum(currentID);
    }
    else {
        removeFromAlbum(currentID, albumManager->currentAlbumName());
    }
}



void QMusicPlayer::tick(qint64 time)
{
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

    timeLcd->display(displayTime.toString("mm:ss"));
}

void QMusicPlayer::load(int index)
{
    bool wasPlaying = mediaObject->state() == PlayingState;

    mediaObject->stop();
#ifdef USE_PHONON
    mediaObject->clearQueue();
#else
    mediaObject->setMedia(QMediaContent());
#endif

    if (index >= sources.size()) {
        return;
    }
#ifdef USE_PHONON
    mediaObject->setCurrentSource(sources.at(index));
#else
    mediaObject->setMedia(sources.at(index));
#endif

    if (wasPlaying) {
        mediaObject->play();
    }
    else {
        mediaObject->stop();
    }
}

void QMusicPlayer::playRequestedByUser(bool play)
{
    if (!mediaObject) {
        return;
    }

    QString album;
    albumContains(currentID, &album);
    albumManager->albumList()->setCurrentIndex(albumManager->albumList()->findText(album));
    notLoaded = mediaObject->media().isNull();

    if (play) {
        mediaObject->play();
    }
}

void QMusicPlayer::startLyricSyncer()
{
    QString gLyricName = source().left(source().lastIndexOf(QLatin1Char('.'))) + QLatin1String(".xml");

    QFile file(gLyricName);
    if (file.exists()) {
        setLyricSyncerState(true, false);
        return;
    }

    m_syncMap.clear();
    setLyricSyncerState(false);
    connect(this, SIGNAL(verseSelectedByUser(int)), this, SLOT(recordTimeForVerse(int)));
}

void QMusicPlayer::stopLyricSyncer(bool cancel)
{
    disconnect(this, SIGNAL(verseSelectedByUser(int)), this, SLOT(recordTimeForVerse(int)));

    QString gLyricName = source().left(source().lastIndexOf(QLatin1Char('.'))) + QLatin1String(".xml");

    QFile file(gLyricName);
    if (file.exists()) {
        setLyricSyncerState(true, false);
        return;
    }

    if (m_syncMap.isEmpty()) {
        cancel = true;
    }
    setLyricSyncerState(true, cancel);

    if (cancel) {
        return;
    }

    QDomDocument m_domDocument;
    QDomElement root = m_domDocument.createElement("DesktopGanjoorPoemAudioList");
    QDomNode poemAudio = m_domDocument.createElement("PoemAudio");
    QDomNode poemId = m_domDocument.createElement("PoemId");
    poemId.appendChild(m_domDocument.createTextNode(QString::number(currentID)));
    QDomNode id = m_domDocument.createElement("Id");
    id.appendChild(m_domDocument.createTextNode("1"));
    QDomNode filePath = m_domDocument.createElement("FilePath");
    filePath.appendChild(m_domDocument.createTextNode(source()));
    QDomNode description = m_domDocument.createElement("Description");
    description.appendChild(m_domDocument.createTextNode(currentTitle));
    QDomNode syncGuid = m_domDocument.createElement("SyncGuid");
    syncGuid.appendChild(m_domDocument.createTextNode(QUuid::createUuid().toString().remove("{").remove("}")));
    QDomNode fileCheckSum = m_domDocument.createElement("FileCheckSum");

    QFile audio(source());
    audio.open(QFile::ReadOnly);
    QString audioMD5SUM = QString::fromLatin1(QCryptographicHash::hash(audio.readAll(), QCryptographicHash::Md5).toHex());
    fileCheckSum.appendChild(m_domDocument.createTextNode(audioMD5SUM));
    QDomNode syncArray = m_domDocument.createElement("SyncArray");

    QList<int> vOrders = m_syncMap.values();
    if (!vOrders.contains(-1)) {
        qint64 first = qBound(qint64(0), qint64(m_syncMap.keys().at(0) / 2), qMin(m_syncMap.keys().at(0) - 1, qint64(1000)));
        m_syncMap.insert(first, -1);
    }

    if (!vOrders.contains(-2)) {
#ifdef USE_PHONON
        qint64 duration = mediaObject->totalTime();
#else
        qint64 duration = mediaObject->duration();
#endif
        qint64 last = m_syncMap.keys().at(m_syncMap.size() - 1) + 1;
        last = qBound(last, (duration + last) / 2, duration);
        m_syncMap.insert(last, -2);
    }

    QMap<qint64, int>::const_iterator it = m_syncMap.constBegin();
    while (it != m_syncMap.constEnd()) {
        QDomNode syncInfo = m_domDocument.createElement("SyncInfo");
        QDomNode verseOrder = m_domDocument.createElement("VerseOrder");
        verseOrder.appendChild(m_domDocument.createTextNode(QString::number(it.value())));
        QDomNode audioMiliseconds = m_domDocument.createElement("AudioMiliseconds");
        audioMiliseconds.appendChild(m_domDocument.createTextNode(QString::number(it.key())));

        syncInfo.appendChild(verseOrder);
        syncInfo.appendChild(audioMiliseconds);

        syncArray.appendChild(syncInfo);

        ++it;
    }

    poemAudio.appendChild(poemId);
    poemAudio.appendChild(id);
    poemAudio.appendChild(filePath);
    poemAudio.appendChild(description);
    poemAudio.appendChild(syncGuid);
    poemAudio.appendChild(fileCheckSum);
    poemAudio.appendChild(syncArray);

    root.appendChild(poemAudio);
    m_domDocument.appendChild(root);

    const int IndentSize = 4;
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("QMusicPlayer"), tr("Can not write the lyric file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
    }
    else {
        QTextStream out(&file);
        out.setCodec("utf-8");
        QString domString = m_domDocument.toString(IndentSize);
        domString = domString.replace("&#xd;", "");
        out << "<!-- Generated by Saaghar -->\n";
        out << domString;
    }
}

void QMusicPlayer::recordTimeForVerse(int vorder)
{
    qint64 time = qMax(currentTime(), qint64(TICK_INTERVAL + 1));
    m_syncMap.insert(time, vorder - 1);
}

void QMusicPlayer::setLyricSyncerState(bool startState, bool enabled)
{
    if (!m_lyricSyncer) {
        return;
    }

    if (startState) {
        // stop lyric syncer
        disconnect(this, SIGNAL(verseSelectedByUser(int)), this, SLOT(recordTimeForVerse(int)));
        m_lyricSyncer->setEnabled(enabled);
        m_lyricSyncer->setText(tr("&Run text/audio syncer"));
        connect(m_lyricSyncer, SIGNAL(triggered()), this, SLOT(startLyricSyncer()));
        disconnect(m_lyricSyncer, SIGNAL(triggered()), this, SLOT(stopLyricSyncer()));
        setStyleSheet("background-image:url(\":/resources/images/transp.png\"); border:none;");
        m_lyricSyncerRuninng = false;
    }
    else {
        m_lyricSyncerRuninng = true;
        m_lyricSyncer->setEnabled(true);
        m_lyricSyncer->setText(tr("&Stop text/audio syncer"));
        disconnect(m_lyricSyncer, SIGNAL(triggered()), this, SLOT(startLyricSyncer()));
        connect(m_lyricSyncer, SIGNAL(triggered()), this, SLOT(stopLyricSyncer()));
        setStyleSheet("QToolBar{background-image:url(\":/resources/images/transp.png\"); border: 3px solid red; border-radius: 5px;}");
    }
}

void QMusicPlayer::sourceChanged()
{
    timeLcd->display("00:00");
}

void QMusicPlayer::stateChange()
{
    State newState = State(mediaObject->state());

#ifndef USE_PHONON
    if (mediaObject->mediaStatus() == QMediaPlayer::EndOfMedia) {
        // TODO: was the last source?
        aboutToFinish();
    }

    if (mediaObject->error() != QMediaPlayer::NoError) {
        newState = ErrorState;
    }
#endif
    switch (newState) {
    case ErrorState:
        emit mediaChanged("", currentTitle, currentID, false);
        break;

    case PlayingState:
        stopAction->setEnabled(true);
        togglePlayPauseAction->setEnabled(true);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        togglePlayPauseAction->setText(tr("Pause"));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), this, SLOT(playRequestedByUser()));
        connect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        break;
    case StoppedState:
        stopAction->setEnabled(false);
        togglePlayPauseAction->setEnabled(true);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
        connect(togglePlayPauseAction, SIGNAL(triggered()), this, SLOT(playRequestedByUser()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        timeLcd->display("00:00");
        // clear highlight
        emit showTextRequested(currentID, -3);

        break;
    case PausedState:
        stopAction->setEnabled(true);
        togglePlayPauseAction->setEnabled(true);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
        connect(togglePlayPauseAction, SIGNAL(triggered()), this, SLOT(playRequestedByUser()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        break;

    case BufferingState:
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
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), this, SLOT(playRequestedByUser()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        timeLcd->display("00:00");
    }
#ifdef USE_PHONON
    if (mediaObject->currentSource().type() == Phonon::MediaSource::Invalid) {
#else
    if (mediaObject->mediaStatus() == QMediaPlayer::InvalidMedia) {
#endif
        stopAction->setEnabled(false);
        togglePlayPauseAction->setEnabled(false);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
    }
}

void QMusicPlayer::metaStateChange()
{
    State newState = State(metaInformationResolver->state());

    if (m_lyricSyncerRuninng && (newState == StoppedState || newState == ErrorState)) {
        stopLyricSyncer();
    }

    if (newState == ErrorState) {
#ifdef SAAGHAR_DEBUG
        qWarning() << "Error opening files: " << metaInformationResolver->errorString();
#endif

#ifdef USE_PHONON
        Phonon::MediaSource source = metaInformationResolver->currentSource();
#else
        QMediaContent source = metaInformationResolver->currentMedia();
#endif

        infoLabel->setText("Error opening files: " + metaInformationResolver->errorString());
        while (!sources.isEmpty() && !(sources.takeLast() == source))
        {}  /* loop */;
        return;
    }

    if (newState != StoppedState && newState != PausedState) {
        return;
    }
#ifdef USE_PHONON
    if (metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid) {
#else
    if (metaInformationResolver->mediaStatus() == QMediaPlayer::InvalidMedia) {
#endif
        setLyricSyncerState(true, false);
        return;
    }

    QMap<QString, QString> metaData;

#ifdef USE_PHONON
    metaData = metaInformationResolver->metaData();
    Phonon::MediaSource source = metaInformationResolver->currentSource();
    const QString &fileName = metaInformationResolver->currentSource().fileName();
#else
    QMediaContent source = metaInformationResolver->currentMedia();
    const QString &fileName = source.canonicalUrl().toLocalFile();

    metaData.insert("TITLE", metaInformationResolver->metaData("Title").toString());
    metaData.insert("ARTIST", metaInformationResolver->metaData("Author").toStringList().join(QLatin1String("-")));
    metaData.insert("ALBUM", metaInformationResolver->metaData("AlbumTitle").toString());
    metaData.insert("DATE", metaInformationResolver->metaData("Date").toDate().toString());
#endif

    QString title = metaData.value("TITLE");
    if (title == "") {
        title = fileName;
    }

    QStringList metaInfos;
    metaInfos << title << metaData.value("ARTIST") << metaData.value("ALBUM") << metaData.value("DATE");
    metaInfos.removeDuplicates();
    QString tmp = metaInfos.join("-");
    metaInfos = tmp.split("-", QString::SkipEmptyParts);
    infoLabel->setText(metaInfos.join(" - "));

    int index = sources.indexOf(source) + 1;
    if (sources.size() > index) {
#ifdef USE_PHONON
        metaInformationResolver->setCurrentSource(sources.at(index));
#else
        metaInformationResolver->setMedia(sources.at(index));
#endif
    }

    if (notLoaded) {
        infoLabel->setText("");
        stopAction->setEnabled(false);
        togglePlayPauseAction->setEnabled(false);
        togglePlayPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        togglePlayPauseAction->setText(tr("Play"));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), this, SLOT(playRequestedByUser()));
        disconnect(togglePlayPauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()));
        timeLcd->display("00:00");
    }
}

void QMusicPlayer::aboutToFinish()
{
    if (m_lyricSyncerRuninng) {
        stopLyricSyncer();
    }

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
    m_removeAllSourceAction = new QAction(tr("Remove Audio From All Album"), this);
    loadAlbumAction = new QAction(tr("&Load Album..."), this);
    m_lyricSyncer = new QAction(tr("&Run text/audio syncer"), this);
    m_lyricSyncer->setDisabled(true);

    connect(togglePlayPauseAction, SIGNAL(triggered()), this, SLOT(playRequestedByUser()));
    connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));
    connect(setSourceAction, SIGNAL(triggered()), this, SLOT(setSource()));
    connect(removeSourceAction, SIGNAL(triggered()), this, SLOT(removeSource()));
    connect(m_removeAllSourceAction, SIGNAL(triggered()), this, SLOT(removeSource()));
    connect(loadAlbumAction, SIGNAL(triggered()), this, SLOT(loadAlbumFile()));
    connect(m_lyricSyncer, SIGNAL(triggered()), this, SLOT(startLyricSyncer()));
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
    menu->addAction(m_removeAllSourceAction);
    menu->addSeparator();
    menu->addAction(m_lyricSyncer);
    menu->addSeparator();
    menu->addAction(loadAlbumAction);
    menu->addSeparator();
    if (albumManagerDock()) {
        menu->addAction(albumManagerDock()->toggleViewAction());
    }
    options->setMenu(menu);

    addWidget(options);
    addSeparator();
    addAction(togglePlayPauseAction);
    addAction(stopAction);
    addSeparator();

    infoLabel = new ScrollText(this);
    infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

#ifdef USE_PHONON
    seekSlider = new Phonon::SeekSlider(this);
    seekSlider->setMediaObject(mediaObject);

    volumeSlider = new Phonon::VolumeSlider(this);
    volumeSlider->setAudioOutput(audioOutput);
#else
    seekSlider = new SeekSlider(mediaObject, this);
    connect(this, SIGNAL(orientationChanged(Qt::Orientation)), seekSlider, SLOT(playerOrientationChanged(Qt::Orientation)));
    seekSlider->playerOrientationChanged(orientation());

    volumeSlider = new VolumeSlider(mediaObject, this);
    connect(this, SIGNAL(orientationChanged(Qt::Orientation)), volumeSlider, SLOT(playerOrientationChanged(Qt::Orientation)));
    volumeSlider->playerOrientationChanged(orientation());
#endif

    infoLabel->resize(seekSlider->width(), infoLabel->height());
    volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);


    QPalette palette;
    palette.setBrush(QPalette::Light, Qt::darkGray);

    timeLcd = new QLCDNumber;
    timeLcd->setPalette(palette);

    QVBoxLayout* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(0);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* infoWidget = new QWidget;
    infoWidget->setLayout(infoLayout);


    infoLayout->addWidget(seekSlider);
    infoLayout->addWidget(infoLabel/*tmpLabel*/, 0, 0 /* Qt::AlignCenter*/);

    addWidget(infoWidget);
    addWidget(timeLcd);
    addWidget(volumeSlider);

    layout()->setAlignment(options, Qt::AlignCenter);
    layout()->setAlignment(timeLcd, Qt::AlignCenter);
    layout()->setAlignment(volumeSlider, Qt::AlignCenter);

    setLayoutDirection(Qt::LeftToRight);
}

void QMusicPlayer::createConnections()
{
#if USE_PHONON
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(sourceChanged()));
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(seekOnStateChange()));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChange()));
    connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(metaStateChange()));
    connect(mediaObject, SIGNAL(finished()), this, SLOT(aboutToFinish()));
#else
    connect(mediaObject, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(sourceChanged()));
    connect(mediaObject, SIGNAL(positionChanged(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(seekOnStateChange()));
    connect(mediaObject, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChange()));
    connect(metaInformationResolver, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(metaStateChange()));
#endif
}

void QMusicPlayer::readPlayerSettings()
{
    albumsPathList = VAR("QMusicPlayer/ListOfAlbum").toHash();

    QStringList keys = albumsPathList.uniqueKeys();
    for (int i = 0; i < keys.size(); ++i) {
        if (!QFile::exists(albumsPathList.value(keys.at(i)).toString())) {
            albumsPathList.remove(keys.at(i));
        }
    }

#ifdef USE_PHONON
    if (audioOutput) {
        audioOutput->setMuted(VARB("QMusicPlayer/Muted"));
        audioOutput->setVolume(VAR("QMusicPlayer/Volume").toReal());
    }
#else
    mediaObject->setMuted(VARB("QMusicPlayer/Muted"));
    mediaObject->setVolume(int(VAR("QMusicPlayer/Volume").toReal() * 100));
#endif
}

void QMusicPlayer::savePlayerSettings()
{
    VAR_DECL("QMusicPlayer/ListOfAlbum", albumsPathList);

#ifdef USE_PHONON
    if (audioOutput) {
        VAR_DECL("QMusicPlayer/Muted", audioOutput->isMuted());
        VAR_DECL("QMusicPlayer/Volume", audioOutput->volume());
    }
#else
    VAR_DECL("QMusicPlayer/Muted", mediaObject->isMuted());
    VAR_DECL("QMusicPlayer/Volume", ((qreal)mediaObject->volume() / 100.0));
#endif
}

void QMusicPlayer::loadAlbum(const QString &fileName, bool inserToPathList)
{
    /*******************************************************************************/
    //tags for Version-0.1 of Saaghar media album
    //#SAAGHAR!ALBUM!       //start of album
    //#ALBUM!TITLE!         //title tag
    //#ITEMS!               //start of items
    //#TITLE!               //item's title tag
    //#ID!                  //item's id tag
    //#PATH!                //item's full path
    //#MD5SUM!              //item's MD5SUM hash
    //<last property>               //item's path relative to album file
    /*******************************************************************************/
    QFile file(fileName);
    if (!file.exists()) {
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::information(this->parentWidget(), tr("Read Error!"), tr("Can't load album!\nError: %1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    QString line = out.readLine();
    if (!line.startsWith("#SAAGHAR!ALBUM!")) {
        return;
    }

    line = out.readLine();
    while (!line.startsWith("#ALBUM!TITLE!")) {
        if (out.atEnd()) {
            return;
        }
        line = out.readLine();
    }

    QString albumName = line.remove("#ALBUM!TITLE!");
    if (albumName.isEmpty()) {
        return;
    }
    SaagharAlbum* album = new SaagharAlbum;
    album->PATH = fileName;
    if (inserToPathList) {
        albumsPathList.insert(albumName, fileName);
    }

    while (!line.startsWith("#ITEMS!")) { //start of items
        if (out.atEnd()) {
            return;
        }
        line = out.readLine();
    }

    QFileInfo albumFileInfo(fileName);
    QDir albumDir(albumFileInfo.absolutePath());

    int ID;
    QString TITLE;
    QString FULL_PATH;
    QString MD5SUM;
    bool ok = false;
    while (!out.atEnd()) {
        line = out.readLine();
        if (line.startsWith("#")) {
            line.remove("#");
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
            else if (line.startsWith("EXTINF:")) {
                line.remove("EXTINF:");
            }
        }
        else {
            line = line.trimmed();
            if (ID != -1 && (!line.isEmpty() || !FULL_PATH.isEmpty())) {
                //at least one of path and id is necessary!!
                QString absoluteMediaPath = albumDir.absoluteFilePath(line);
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
                    album->mediaItems.insert(ID, mediaTag);
                }
                FULL_PATH.clear();
                TITLE.clear();
                MD5SUM.clear();
                ID = -1;
            }
        }
    }

    file.close();
    pushAlbum(album, albumName);
    albumManager->setCurrentAlbum(albumName);
}

void QMusicPlayer::saveAlbum(const QString &fileName, const QString &albumName, bool inserToPathList, const QString &)
{
    if (albumName.isEmpty()) {
        return;
    }

    SaagharAlbum* album = albumManager->albumByName(albumName);

    if (!album) {
        return;
    }

    QString albumFileName = fileName;
    if (albumFileName.isEmpty()) {
        albumFileName = album->PATH;
    }

    if (inserToPathList) {
        albumsPathList.insert(albumName, albumFileName);
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::information(this->parentWidget(), tr("Save Error!"), tr("Can't save album!\nError: %1").arg(file.errorString()));
        return;
    }

    QFileInfo albumFileInfo(fileName);
    QDir albumInfo(albumFileInfo.absolutePath());
    QTextStream out(&file);
    out.setCodec("UTF-8");
    QString albumContent = QString("#SAAGHAR!ALBUM!V%1\n#ALBUM!TITLE!%2\n##################\n#ITEMS!\n").arg("0.1").arg(albumName);
    QHash<int, SaagharMediaTag*>::const_iterator it = album->mediaItems.constBegin();
    while (it != album->mediaItems.constEnd()) {
        QString relativePath = albumInfo.relativeFilePath(it.value()->PATH);
        QString md5sumStr;
        if (!it.value()->MD5SUM.isEmpty()) {
            md5sumStr = QString("#MD5SUM!%1\n").arg(it.value()->MD5SUM);
        }
        QString itemStr = QString(
                              "#TITLE!%1\n"
                              "#ID!%2\n"
                              "%3"
                              "#PATH!%4\n"
                              "#EXTINF:0,%1\n"
                              "%5\n")
                          .arg(it.value()->TITLE).arg(it.key())
                          .arg(md5sumStr).arg(it.value()->PATH)
                          .arg(relativePath);
        albumContent += itemStr;
        ++it;
    }

    out << albumContent;
    file.close();
}

void QMusicPlayer::loadAllAlbums()
{
    QHash<QString, QVariant>::const_iterator albumsIterator = albumsPathList.constBegin();
    while (albumsIterator != albumsPathList.constEnd()) {
        if (!albumsIterator.key().isEmpty()) {
            loadAlbum(albumsIterator.value().toString(), false);
        }
        ++albumsIterator;
    }
}

void QMusicPlayer::saveAllAlbums(const QString &format)
{
    QHash<QString, QVariant>::const_iterator albumsIterator = albumsPathList.constBegin();
    while (albumsIterator != albumsPathList.constEnd()) {
        if (!albumsIterator.key().isEmpty()) {
            saveAlbum(albumsIterator.value().toString(), albumsIterator.key(), false, format);
        }
        ++albumsIterator;
    }
}

/*static*/
void QMusicPlayer::removeFromAlbum(int mediaID, QString albumName)
{
    if (!albumName.isEmpty()) {
        SaagharAlbum* album = albumManager->albumByName(albumName);
        if (album) {
            album->mediaItems.remove(mediaID);
        }
        return;
    }

    while (albumContains(mediaID, &albumName)) {
        SaagharAlbum* album = albumManager->albumByName(albumName);
        if (album) {
            album->mediaItems.remove(mediaID);
        }
    }
}

/*static*/
void QMusicPlayer::insertToAlbum(int mediaID, const QString &mediaPath, const QString &mediaTitle,
                                 qint64 mediaCurrentTime, QString albumName)
{
    if (albumName.isEmpty()) {
        albumName = albumManager->currentAlbumName();
    }

    SaagharAlbum* album = albumManager->albumByName(albumName);

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

    if (!album) {
        album = new SaagharAlbum;
        album->PATH = albumsPathList.value(albumName).toString();
        album->mediaItems.insert(mediaID, mediaTag);
        pushAlbum(album, albumName);
    }
    else {
        album->mediaItems.insert(mediaID, mediaTag);
    }
}

/*static*/
void QMusicPlayer::getFromAlbum(int mediaID, QString* mediaPath, QString* mediaTitle,
                                qint64* mediaCurrentTime, QString* albumName)
{
    QString temp;
    bool albumNameIsNull = false;
    if (!albumName) {
        albumNameIsNull = true;
        albumName = &temp;
    }

    if (mediaID < 0 || !mediaPath || !albumContains(mediaID, albumName)) {
        return;
    }

    SaagharMediaTag* mediaTag =  albumManager->albumByName(*albumName)->mediaItems.value(mediaID);
    if (!mediaTag) {
        mediaPath->clear();
        mediaTitle->clear();
        if (mediaCurrentTime) {
            *mediaCurrentTime = 0;
        }
        if (albumNameIsNull) {
            albumName = 0;
        }
        else {
            albumName->clear();
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
    if (albumNameIsNull) {
        albumName = 0;
    }
}

bool QMusicPlayer::albumContains(int mediaID, QString* albumName)
{
    //albumName default can be NULL for test purposes we don't set it
    QStringList names = albumsMediaHash.keys();
    if (names.removeAll(albumManager->currentAlbumName()) > 0) {
        names.prepend(albumManager->currentAlbumName());
    }

    for (int i = 0; i < names.size(); ++i) {
        SaagharAlbum* album = albumManager->albumByName(names.at(i));

        if (!album) {
            continue;
        }

        bool result = album->mediaItems.contains(mediaID);
        if (result) {
            if (albumName) {
                *albumName = names.at(i);
            }

            return true;
        }
    }

    return false;
}

QDockWidget* QMusicPlayer::albumManagerDock()
{
    if (!albumManager) {
        return 0;
    }
    if (dockList) {
        return dockList;
    }
    dockList = new QDockWidget;
    dockList->setObjectName("AlbumManagerDock");
    dockList->setStyleSheet("QDockWidget::title { background: transparent; text-align: left; padding: 0 10 0 10;}"
                            "QDockWidget::close-button, QDockWidget::float-button { background: transparent;}");
    dockList->setWindowTitle(tr("Albums"));
    dockList->setWidget(albumManager);
    return dockList;
}

void QMusicPlayer::playMedia(int mediaID)
{
    disconnect(this, SIGNAL(mediaChanged(QString,QString,int,bool)), albumManager, SLOT(currentMediaChanged(QString,QString,int,bool)));
    emit requestPageContainedMedia(mediaID, false);
    mediaObject->play();
    connect(this, SIGNAL(mediaChanged(QString,QString,int,bool)), albumManager, SLOT(currentMediaChanged(QString,QString,int,bool)));
}

void QMusicPlayer::resizeEvent(QResizeEvent* e)
{
    QToolBar::resizeEvent(e);
#ifdef USE_PHONON
    infoLabel->resize(seekSlider->width(), infoLabel->height());
#endif
}

/*static*/
QStringList QMusicPlayer::commonSupportedMedia(const QString &type)
{
    QStringList supportedExtentions;
    const QStringList commonMediaExtentions = QStringList()
            << "mp3" << "wav" << "wma" << "ogg" << "mp4" << "mpg" << "mid"
            << "asf" << "3gp" << "wmv" << "avi";

    const QString sep = "|";
#ifdef USE_PHONON
    QString supportedMimeTypes = Phonon::BackendCapabilities::availableMimeTypes().join(sep);
#else
    // TODO: obsolete
    QString supportedMimeTypes = QMediaPlayer::supportedMimeTypes().join(sep);
    //qDebug() << QMediaPlayer::supportedMimeTypes();
#endif
    for (int i = 0; i < commonMediaExtentions.size(); ++i) {
        QString extention = commonMediaExtentions.at(i);
        if (supportedMimeTypes.contains(QRegExp(QString("(%1/[^%2]*%3[^%2]*)").arg(type).arg(sep).arg(extention)))) {
            if (type == "audio" && (extention == "mpg" || extention == "3gp")) {
                continue;
            }

            supportedExtentions << "*." + extention;
        }
    }

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
// class AlbumManager
********************************/

AlbumManager::AlbumManager(QMusicPlayer* musicPlayer, QWidget* parent)
    : QWidget(parent)
    , previousItem(0)
    , mediaList(new QTreeWidget(this))
    , m_albumList(new QComboBox(this))
    , m_musicPlayer(musicPlayer)
{
    setObjectName("AlbumManager");
    setupUi();

    connect(mediaList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemPlayRequested(QTreeWidgetItem*,int)));
    connect(m_albumList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentAlbumChanged(int)));
}

void AlbumManager::setupUi()
{
    mediaList->setIndentation(5);
    mediaList->setLayoutDirection(Qt::RightToLeft);
    mediaList->setTextElideMode(Qt::ElideMiddle);
    mediaList->headerItem()->setHidden(true);
    mediaList->setColumnCount(1);

    m_albumList->insertSeparator(0);
    m_albumList->addItem(tr("Save this album"), "SAVE_ALBUM");
    m_albumList->addItem(tr("Load Album..."), "LOAD_ALBUM");
    m_albumList->addItem(tr("New Album..."), "NEW_ALBUM");
    m_albumList->insertSeparator(4);
    m_albumList->addItem(tr("Rename this album..."), "RENAME_ALBUM");
    m_albumList->addItem(tr("Remove this album..."), "REMOVE_ALBUM");
    m_albumList->addItem(tr("Save as this album..."), "SAVE_AS_ALBUM");

    QToolButton* toggleMusicPlayer = new QToolButton(this);
    toggleMusicPlayer->setAutoRaise(true);
    toggleMusicPlayer->setCheckable(true);
    toggleMusicPlayer->setDefaultAction(m_musicPlayer->toggleViewAction());

    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(m_albumList);
    hLayout->addWidget(toggleMusicPlayer);

    mainLayout->addItem(hLayout);
    mainLayout->addWidget(mediaList);
    this->setLayout(mainLayout);
}

void AlbumManager::setAlbums(const QHash<QString, QMusicPlayer::SaagharAlbum*> &albums, bool /*justMediaList*/)
{
    if (albums.isEmpty()) {
        return;
    }

    previousItem = 0;
    mediaList->clear();

    QHash<QString, QMusicPlayer::SaagharAlbum*>::const_iterator albumIterator = albums.constBegin();
    while (albumIterator != albums.constEnd()) {
        QMusicPlayer::SaagharAlbum* album = albumIterator.value();
        if (!album) {
            ++albumIterator;
            continue;
        }

        QTreeWidgetItem* albumRoot = 0;
        if (!itemsAsTopItem) {
            albumRoot = new QTreeWidgetItem();
            mediaList->addTopLevelItem(albumRoot);
            albumRoot->setText(0, albumIterator.key());
        }

        QHash<int, QMusicPlayer::SaagharMediaTag*> items = album->mediaItems;
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
                albumRoot->addChild(mediaItem);
            }
            else {
                mediaList->addTopLevelItem(mediaItem);
            }
            ++mediaIterator;
        }
        ++albumIterator;
    }
}

void AlbumManager::setCurrentAlbum(const QString &albumName)
{
    if (albumName.isEmpty() || albumName == m_currentAlbum) {
        return;
    }

    m_currentAlbum = albumName;

    int index = m_albumList->findText(albumName);
    disconnect(m_albumList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentAlbumChanged(int)));
    if (index != -1) {
        m_albumList->setCurrentIndex(index);
    }
    else {
        m_albumList->insertItem(0, albumName);
        m_albumList->setCurrentIndex(0);
    }
    connect(m_albumList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentAlbumChanged(int)));

    QMusicPlayer::SaagharAlbum* album = albumByName(albumName);
    if (!album) {
        return;
    }

    previousItem = 0;
    mediaList->clear();
    QFontMetrics fontMetric(mediaList->font());
    QHash<int, QMusicPlayer::SaagharMediaTag*> items = album->mediaItems;
    QHash<int, QMusicPlayer::SaagharMediaTag*>::const_iterator mediaIterator = items.constBegin();
    while (mediaIterator != items.constEnd()) {
        QMusicPlayer::SaagharMediaTag* mediaTag = mediaIterator.value();
        if (!mediaTag) {
            ++mediaIterator;
            continue;
        }

        QTreeWidgetItem* mediaItem = new QTreeWidgetItem(mediaList);
        int tagMediaID = mediaIterator.key();

        if (m_currentID > 0 && m_currentID == tagMediaID && m_musicPlayer->source().isEmpty()) {
            disconnect(m_musicPlayer, SIGNAL(mediaChanged(QString,QString,int,bool)), this, SLOT(currentMediaChanged(QString,QString,int,bool)));
            m_musicPlayer->setSource(mediaTag->PATH, mediaTag->TITLE, m_currentID);
            connect(m_musicPlayer, SIGNAL(mediaChanged(QString,QString,int,bool)), this, SLOT(currentMediaChanged(QString,QString,int,bool)));
        }

        bool isPaused = m_currentFile.isEmpty() ||
                        m_currentID < 0 ||
                        mediaTag->PATH != m_currentFile ||
                        tagMediaID != m_currentID;
        if (isPaused) {
            mediaItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
        }
        else {
            if (albumMediaObject->state() != QMusicPlayer::PlayingState) {
                mediaItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPause));
            }
            else {
                mediaItem->setIcon(0, style()->standardIcon(QStyle::SP_MediaPlay));
            }
            previousItem = mediaItem;
        }

        mediaItem->setText(0, mediaTag->TITLE.mid(mediaTag->TITLE.lastIndexOf(">") + 1));
        mediaItem->setToolTip(0, fontMetric.elidedText(mediaTag->TITLE, Qt::ElideRight, 400) + "\n" + mediaTag->PATH);
        //          mediaItem->setText(1, mediaTag->PATH);
        mediaItem->setData(0, Qt::UserRole, tagMediaID);

        mediaList->addTopLevelItem(mediaItem);
        ++mediaIterator;
    }
}

QMusicPlayer::SaagharAlbum* AlbumManager::albumByName(QString albumName)
{
    if (albumName.isEmpty()) {
        albumName = m_currentAlbum;
    }

    return QMusicPlayer::albumsHash().value(albumName);
}

void AlbumManager::setCurrentMedia(int currentID, const QString &currentFile)
{
    m_currentID = currentID;
    m_currentFile = currentFile;
}

void AlbumManager::currentMediaChanged(const QString &fileName, const QString &title, int mediaID, bool removeRequest)
{
    if (mediaID <= 0) { // fileName.isEmpty() for delete request!!
        return;
    }

    setCurrentMedia(mediaID, fileName);
    QFontMetrics fontMetric(mediaList->font());
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
                        childItem->setText(0, title.mid(title.lastIndexOf(">") + 1));
                        childItem->setToolTip(0, fontMetric.elidedText(title, Qt::ElideRight, 400) + "\n" + fileName);
                    }
//                  childItem->setText(1, fileName);
                    if (albumMediaObject && albumMediaObject->state() == QMusicPlayer::PlayingState) {
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
            newChild->setText(0, title.mid(title.lastIndexOf(">") + 1));
            newChild->setToolTip(0, fontMetric.elidedText(title, Qt::ElideRight, 400) + "\n" + fileName);
//          newChild->setText(1, fileName);
            newChild->setData(0, Qt::UserRole, mediaID);
            if (albumMediaObject && albumMediaObject->state() == QMusicPlayer::PlayingState) {
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

void AlbumManager::currentAlbumChanged(int index)
{
    QVariant v = m_albumList->itemData(index);
    if (!v.isValid()) {
        setCurrentAlbum(m_albumList->itemText(index));
    }
    else {
        disconnect(m_albumList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentAlbumChanged(int)));
        m_albumList->setCurrentIndex(m_albumList->findText(m_currentAlbum));
        connect(m_albumList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentAlbumChanged(int)));

        QString command = v.toString();

        if (command == "SAVE_ALBUM") {
            m_musicPlayer->saveAsAlbum(m_currentAlbum, false);
        }
        else if (command == "LOAD_ALBUM") {
            m_musicPlayer->loadAlbumFile();
        }
        else if (command == "NEW_ALBUM") {
            m_musicPlayer->newAlbum();
        }
        else if (command == "RENAME_ALBUM") {
            m_musicPlayer->renameAlbum(m_currentAlbum);
        }
        else if (command == "REMOVE_ALBUM") {
            m_musicPlayer->removeAlbum(m_currentAlbum);
        }
        else if (command == "SAVE_AS_ALBUM") {
            m_musicPlayer->saveAsAlbum(m_currentAlbum);
        }
    }
}

void AlbumManager::itemPlayRequested(QTreeWidgetItem* item, int /*col*/)
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
#ifdef USE_PHONON
void AlbumManager::setMediaObject(Phonon::MediaObject* MediaObject)
{
    albumMediaObject = MediaObject;
    connect(albumMediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(mediaObjectStateChanged()));
}
#else
void AlbumManager::setMediaObject(QMediaPlayer* MediaObject)
{
    albumMediaObject = MediaObject;

    connect(albumMediaObject, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(mediaObjectStateChanged()));
}
#endif

void AlbumManager::mediaObjectStateChanged()
{
    if (previousItem) {
        if (albumMediaObject->state() == QMusicPlayer::PlayingState) {
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
// modified by S. Razi Alavizadeh to support RTL strings
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
    updateText(sender());
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

void ScrollText::updateText(bool keepScrolling)
{
    timer.stop();

    singleTextWidth = fontMetrics().width(_text);
    scrollEnabled = (singleTextWidth > width() - leftMargin);

    if (scrollEnabled) {
        if (!keepScrolling) {
            scrollPos = -64;
        }
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
        bool textIsRTL = _text.isRightToLeft();

        int x = qMin(-scrollPos, 0) + leftMargin;
        while (x < width()) {
            int visualX = textIsRTL ? (width() - x - wholeTextSize.width()) : x;
#if QT_VERSION >= 0x040700
            pb.drawStaticText(QPointF(visualX, (height() - wholeTextSize.height()) / 2), staticText);
#else
            pb.drawText(QPointF(visualX, height() - 2 - ((height() - wholeTextSize.height()) / 2)), staticText.text());
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

SeekSlider::SeekSlider(QMediaPlayer *mediaPlayer, QWidget *parent)
    : QSlider(parent),
      m_mediaPlayer(mediaPlayer),
      m_seeking(false)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setRange(0, 1000);
    setTickInterval(10);
    setTickPosition(QSlider::TicksRight);

    connect(m_mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(setPosition(qint64)));
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(seekPosition(int)));
}

void SeekSlider::setPosition(qint64 position)
{
    if (m_seeking) {
        return;
    }
    m_seeking = true;
    setValue((position*1000)/(1+m_mediaPlayer->duration()));
    m_seeking = false;
}

void SeekSlider::seekPosition(int value)
{
    if (m_seeking) {
        return;
    }

    m_seeking = true;
    m_mediaPlayer->setPosition((m_mediaPlayer->duration()/1000)*value);
    m_seeking = false;
}

void SeekSlider::playerOrientationChanged(Qt::Orientation orintation)
{
    setOrientation(orintation);
}

VolumeSlider::VolumeSlider(QMediaPlayer *mediaPlayer, QWidget *parent)
    : QWidget(parent),
      m_mediaPlayer(mediaPlayer)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_muteButton = new QToolButton(this);
    m_muteButton->setCheckable(true);

    volumeMute(m_mediaPlayer->isMuted());

    m_slider = new QSlider(this);
    m_slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_slider->setRange(0, 100);

    m_layout = new QBoxLayout(QBoxLayout::LeftToRight);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_muteButton, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_slider, 0, Qt::AlignVCenter);
    setLayout(m_layout);

    connect(m_muteButton, SIGNAL(toggled(bool)), m_mediaPlayer, SLOT(setMuted(bool)));
    connect(m_mediaPlayer, SIGNAL(mutedChanged(bool)), this, SLOT(volumeMute(bool)));

    connect(m_mediaPlayer, SIGNAL(volumeChanged(int)), m_slider, SLOT(setValue(int)));
    connect(m_slider, SIGNAL(valueChanged(int)), m_mediaPlayer, SLOT(setVolume(int)));
}

void VolumeSlider::playerOrientationChanged(Qt::Orientation orintation)
{
    m_layout->setDirection(orintation == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::BottomToTop);
    m_slider->setOrientation(orintation);
}

void VolumeSlider::volumeMute(bool mute)
{
    m_muteButton->setChecked(mute);
    m_muteButton->setIcon(style()->standardIcon(mute ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
}
