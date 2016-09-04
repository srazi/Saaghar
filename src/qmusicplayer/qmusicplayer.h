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

#ifndef QMUSICPLAYER_H
#define QMUSICPLAYER_H

#include <QToolBar>
#include <QList>

#ifdef USE_PHONON
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>
#else
#include <QMediaPlayer>
#endif

class QAction;
class QLCDNumber;
class QLabel;
class QSettings;
class QDockWidget;

class ScrollText;
class AlbumManager;
class LyricsManager;

#ifndef USE_PHONON
class SeekSlider;
class VolumeSlider;
#endif

class QMusicPlayer : public QToolBar
{
    Q_OBJECT

public:
    enum State {
#ifdef USE_PHONON
        StoppedState = Phonon::StoppedState,
        PlayingState = Phonon::PlayingState,
        PausedState = Phonon::PausedState,
        ErrorState = Phonon::ErrorState,
        BufferingState = Phonon::BufferingState
#else
        StoppedState = QMediaPlayer::StoppedState,
        PlayingState = QMediaPlayer::PlayingState,
        PausedState = QMediaPlayer::PausedState,
        ErrorState,
        BufferingState
#endif

    };

    struct SaagharMediaTag {
        qint64 time;
        QString TITLE;
        QString PATH;
        QString MD5SUM;
    };

    struct SaagharAlbum {
        QString PATH;
        QHash<int, SaagharMediaTag*> mediaItems;
    };

    QMusicPlayer(QWidget* parent = 0);
    QString currentFile() const;
    QString source();
    void setSource(const QString &fileName, const QString &title = "", int mediaID = -1, bool newSource = false);
    void readPlayerSettings();
    void savePlayerSettings();
    qint64 currentTime();
    void setCurrentTime(qint64 time);
    void loadAlbum(const QString &fileName, bool inserToPathList = true);
    void saveAlbum(const QString &fileName, const QString &albumName, bool inserToPathList = true, const QString &format = "M3U8");

    void loadAllAlbums();
    void saveAllAlbums(const QString &format = "M3U8");

    void insertToAlbum(int mediaID, const QString &mediaPath, const QString &mediaTitle = "",
                       qint64 mediaCurrentTime = 0, QString albumName = QString());
    void removeFromAlbum(int mediaID, QString albumName = QString());
    bool albumContains(int mediaID, QString* albumName);
    void getFromAlbum(int mediaID, QString* mediaPath, QString* mediaTitle = 0,
                      qint64* mediaCurrentTime = 0, QString* albumName = 0);

    static QHash<QString, QVariant> albumsPathList;
    static QStringList commonSupportedMedia(const QString &type = "");//"" or "audio" or "video"

    inline static QHash<QString, SaagharAlbum*> albumsHash() { return albumsMediaHash; }

    QDockWidget* albumManagerDock();

public slots:
    void stop();
    void playMedia(int mediaID);

    void newAlbum(QString fileName = QString(), QString albumName = QString());
    void loadAlbumFile();
    void renameAlbum(const QString &albumName);
    void removeAlbum(const QString &albumName);
    void saveAsAlbum(const QString &albumName, bool saveAs = true);
    void showTextByTime(qint64 time);

private slots:
    void removeSource();
    void setSource();
    void seekOnStateChange();
    void stateChange();
    void metaStateChange();

    void sourceChanged();

    void tick(qint64 time);
    void aboutToFinish();
    void load(int index);
    void playRequestedByUser(bool play = true);
    void startLyricSyncer();
    void stopLyricSyncer(bool cancel = false);
    void recordTimeForVerse(int vorder);

private:
    void setLyricSyncerState(bool startState, bool enabled = true);

    bool notLoaded;
    int currentID;
    QString currentTitle;
    QDockWidget* dockList;
    AlbumManager* albumManager;
    inline static void pushAlbum(SaagharAlbum* album, const QString &albumName)
    {albumsMediaHash.insert(albumName, album);}

    static QHash<QString, SaagharAlbum*> albumsMediaHash;

    qint64 _newTime;
    void setupActions();
    void setupUi();
    void createConnections();

#ifdef USE_PHONON
    Phonon::SeekSlider* seekSlider;
    Phonon::MediaObject* mediaObject;
    Phonon::MediaObject* metaInformationResolver;
    Phonon::AudioOutput* audioOutput;
    Phonon::VolumeSlider* volumeSlider;
    QList<Phonon::MediaSource> sources;
#else
    SeekSlider* seekSlider;
    VolumeSlider* volumeSlider;

    QMediaPlayer* mediaObject;
    QMediaPlayer* metaInformationResolver;
    QList<QMediaContent> sources;
#endif

    QAction* togglePlayPauseAction;
    QAction* stopAction;
    QAction* nextAction;
    QAction* previousAction;
    QAction* setSourceAction;
    QAction* removeSourceAction;
    QAction* m_removeAllSourceAction;
    QAction* m_lyricSyncer;
    QAction* loadAlbumAction;
    QAction* repeatAction;
    QAction* autoPlayAction;

    QLCDNumber* timeLcd;
    ScrollText* infoLabel;
    QString startDir;

    LyricsManager* m_lyricReader;
    int m_lastVorder; // -1: start, -2: end, -3: undefined

    QMap<qint64, int> m_syncMap;
    bool m_lyricSyncerRuninng;

protected:
    virtual void resizeEvent(QResizeEvent* e);

signals:
    void mediaChanged(const QString &, const QString &, int, bool); //fileName, title, id, removeRequest
    void requestPageContainedMedia(int, bool); //pageId, newPage
    void showTextRequested(int, int);
    void highlightedTextChange(const QString &);
    void verseSelectedByUser(int);
};

class QTreeWidget;
class QVBoxLayout;
class QTreeWidgetItem;
class QComboBox;

class AlbumManager : public QWidget
{
    Q_OBJECT

public:
    AlbumManager(QMusicPlayer* musicPlayer = 0, QWidget* parent = 0);
    void setAlbums(const QHash<QString, QMusicPlayer::SaagharAlbum*> &albums, bool justMediaList = false);

#ifdef USE_PHONON
    void setMediaObject(Phonon::MediaObject* MediaObject);
#else
    void setMediaObject(QMediaPlayer* MediaObject);
#endif

    inline QString currentFile() { return m_currentFile; }
    inline QString currentAlbumName() { return m_currentAlbum; }
    void setCurrentAlbum(const QString &albumName);
    QMusicPlayer::SaagharAlbum* albumByName(QString albumName = QString());
    void setCurrentMedia(int currentID, const QString &currentFile);
    inline QComboBox* albumList() { return m_albumList; }

private:
    void setupUi();

    QTreeWidgetItem* previousItem;
    QTreeWidget* mediaList;
    QComboBox* m_albumList;
#ifdef USE_PHONON
    Phonon::MediaObject* albumMediaObject;
#else
    QMediaPlayer* albumMediaObject;
#endif

    QString m_currentAlbum;
    int m_currentID;
    QString m_currentFile;
    QMusicPlayer* m_musicPlayer;

private slots:
    void mediaObjectStateChanged();
    void itemPlayRequested(QTreeWidgetItem*, int);
    void currentMediaChanged(const QString &fileName, const QString &title, int mediaID, bool removeRequest = false);
    void currentAlbumChanged(int index);

signals:
    void mediaPlayRequested(int);//mediaID
};

/////////////////////////////////
#include <QWidget>
#if QT_VERSION >= 0x040700
#include <QStaticText>
#else
#include <QLabel>
#endif
#include <QTimer>


class ScrollText : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString separator READ separator WRITE setSeparator)

public:
    explicit ScrollText(QWidget* parent = 0);

public slots:
    QString text() const;
    void setText(QString text);

    QString separator() const;
    void setSeparator(QString separator);


protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void resizeEvent(QResizeEvent*);

private:
    void updateText(bool keepScrolling = false);
    QString _text;
    QString _separator;
#if QT_VERSION >= 0x040700
    QStaticText staticText;
#else
    QLabel staticText;
#endif
    int singleTextWidth;
    QSize wholeTextSize;
    int leftMargin;
    bool scrollEnabled;
    int scrollPos;
    QImage alphaChannel;
    QImage buffer;
    QTimer timer;

private slots:
    virtual void timer_timeout();
};

#ifndef USE_PHONON
#include <QSlider>

class QBoxLayout;
class QToolButton;

class SeekSlider : public QSlider
{
    Q_OBJECT

public:
    explicit SeekSlider(QMediaPlayer* mediaPlayer, QWidget* parent = 0);

private slots:
    void setPosition(qint64 position);
    void seekPosition(int value);

public slots:
    void playerOrientationChanged(Qt::Orientation orintation);

private:
    QMediaPlayer* m_mediaPlayer;
    bool m_seeking;
};

class VolumeSlider : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeSlider(QMediaPlayer* mediaPlayer, QWidget* parent = 0);

public slots:
    void playerOrientationChanged(Qt::Orientation orintation);

private slots:
    void volumeMute(bool mute);

private:
    QBoxLayout* m_layout;
    QSlider* m_slider;
    QToolButton* m_muteButton;
    QMediaPlayer* m_mediaPlayer;
};
#endif

// //link: http://msdn.microsoft.com/en-us/library/dd374921%28VS.85%29.aspx
// //for future
// //Windows 7 volume notification code
//class CAudioSessionVolume : public IAudioSessionEvents
//{
//public:
//  // Static method to create an instance of the object.
//  static HRESULT CreateInstance(
//      UINT uNotificationMessage,
//      HWND hwndNotification,
//      CAudioSessionVolume **ppAudioSessionVolume
//  );

//  // IUnknown methods.
//  STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
//  STDMETHODIMP_(ULONG) AddRef();
//  STDMETHODIMP_(ULONG) Release();

//  // IAudioSessionEvents methods.

//  STDMETHODIMP OnSimpleVolumeChanged(
//      float NewVolume,
//      BOOL NewMute,
//      LPCGUID EventContext
//      );

//  // The remaining audio session events do not require any action.
//  STDMETHODIMP OnDisplayNameChanged(LPCWSTR,LPCGUID)
//  {
//      return S_OK;
//  }

//  STDMETHODIMP OnIconPathChanged(LPCWSTR,LPCGUID)
//  {
//      return S_OK;
//  }

//  STDMETHODIMP OnChannelVolumeChanged(DWORD,float[],DWORD,LPCGUID)
//  {
//      return S_OK;
//  }

//  STDMETHODIMP OnGroupingParamChanged(LPCGUID,LPCGUID)
//  {
//      return S_OK;
//  }

//  STDMETHODIMP OnStateChanged(AudioSessionState)
//  {
//      return S_OK;
//  }

//  STDMETHODIMP OnSessionDisconnected(AudioSessionDisconnectReason)
//  {
//      return S_OK;
//  }

//  // Other methods
//  HRESULT EnableNotifications(BOOL bEnable);
//  HRESULT GetVolume(float *pflVolume);
//  HRESULT SetVolume(float flVolume);
//  HRESULT GetMute(BOOL *pbMute);
//  HRESULT SetMute(BOOL bMute);
//  HRESULT SetDisplayName(const WCHAR *wszName);

//protected:
//  CAudioSessionVolume(UINT uNotificationMessage, HWND hwndNotification);
//  ~CAudioSessionVolume();

//  HRESULT Initialize();

//protected:
//  LONG m_cRef;                        // Reference count.
//  UINT m_uNotificationMessage;        // Window message to send when an audio event occurs.
//  HWND m_hwndNotification;            // Window to receives messages.
//  BOOL m_bNotificationsEnabled;       // Are audio notifications enabled?

//  IAudioSessionControl    *m_pAudioSession;
//  ISimpleAudioVolume      *m_pSimpleAudioVolume;
//};
#endif // QMUSICPLAYER_H
