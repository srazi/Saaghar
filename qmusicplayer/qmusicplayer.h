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

#ifndef QMUSICPLAYER_H
#define QMUSICPLAYER_H

#include <QToolBar>
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>
#include <QList>

QT_BEGIN_NAMESPACE
class QAction;
//class QTableWidget;
class QLCDNumber;
class QLabel;
class QSettings;
class QDockWidget;
QT_END_NAMESPACE

//![0]
class ScrollText;
class PlayListManager;

class QMusicPlayer : public QToolBar
{
//![0]
	Q_OBJECT

public:
	struct SaagharMediaTag {
		int time;
		QString TITLE;
		QString PATH;
		QString RELATIVE_PATH;
		QString MD5SUM;
	};

	struct SaagharPlayList {
		QString PATH;
		QHash<int, SaagharMediaTag *> mediaItems;
//		~SaagharPlayList()
//		{
//			PATH.clear();
//			mediaItems.clear();
//		}
	};

	QMusicPlayer(QWidget *parent = 0);
	QString source();
	void setSource(const QString &fileName, const QString &title = "", int mediaID = -1);
	void readPlayerSettings(QSettings *settingsObject);
	void savePlayerSettings(QSettings *settingsObject);
	qint64 currentTime();
	void setCurrentTime(qint64 time);
	void loadPlayList(const QString &fileName, const QString &playListName = "default", const QString &format="M3U8");
	void savePlayList(const QString &fileName, const QString &playListName = "default", const QString &format="M3U8");

	static void playListItemInsertRemove(int mediaID, const QString &mediaPath, const QString &mediaTitle = "", const QString &mediaRelativePath = "", int mediaCurrentTime = 0, const QString &playListName = "default");
	static bool playListContains(int mediaID, const QString &playListName = "default");
	static void getFromPlayList(int mediaID, QString *mediaPath, QString *mediaTitle = 0, QString *mediaRelativePath = 0, int *mediaCurrentTime = 0, const QString &playListName = "default");

	static QHash<QString, QVariant> listOfPlayList;
	static QStringList commonSupportedMedia(const QString &type = "");//"" or "audio" or "video"
	//static QHash<int, SaagharMediaTag> d efaultPlayList;

	QDockWidget *playListManagerDock();

private slots:
	void playMedia(int mediaID/*, const QString &fileName, const QString &title = ""*/);
	void removeSource();
	void setSource();
	void seekableChanged(bool seekable);

//![1]
	void stateChanged(Phonon::State newState, Phonon::State oldState);
	void tick(qint64 time);
	void sourceChanged(const Phonon::MediaSource &source);
	void metaStateChanged(Phonon::State newState, Phonon::State oldState);
	void aboutToFinish();
	void load(int index);
//![1]

private:
	bool notLoaded;
	int currentID;
	QString currentTitle;
	QDockWidget	*dockList;
	PlayListManager *playListManager;
	inline static SaagharPlayList *playListByName(const QString &playListName = "default")
		{return hashPlayLists.value(playListName);}
	inline static void pushPlayList(SaagharPlayList *playList,const QString &playListName = "default")
		{hashPlayLists.insert(playListName, playList);}

	static QHash<QString, QHash<int, SaagharMediaTag> > playLists;
	static QHash<QString, SaagharPlayList *> hashPlayLists;
	static SaagharPlayList hashDefaultPlayList;
	qint64 _newTime;
	void setupActions();
	//void setupMenus();
	void setupUi();

//![2]
	Phonon::SeekSlider *seekSlider;
	Phonon::MediaObject *mediaObject;
	Phonon::MediaObject *metaInformationResolver;
	Phonon::AudioOutput *audioOutput;
	Phonon::VolumeSlider *volumeSlider;
	QList<Phonon::MediaSource> sources;
//![2]

	QAction *togglePlayPauseAction;
	QAction *stopAction;
	QAction *nextAction;
	QAction *previousAction;
	QAction *setSourceAction;
	QAction *removeSourceAction;
	QAction *repeatAction;
	QAction *autoPlayAction;

	QLCDNumber *timeLcd;
	//QLabel *infoLabel;
	ScrollText *infoLabel;
	QString startDir;
	//QTableWidget *musicTable;

protected:
	virtual void resizeEvent(QResizeEvent *e);

signals:
	void mediaChanged(const QString &,const QString &,int);//fileName, title, id
	void requestPageContainedMedia(/*const QString &,const QString &,*/int,bool);//fileName, title, id
};

class QTreeWidget;
class QVBoxLayout;
class QTreeWidgetItem;

class PlayListManager : public QWidget
{
	Q_OBJECT

public:
	PlayListManager(QWidget *parent = 0);
	void setPlayLists(const QHash<QString, QMusicPlayer::SaagharPlayList *> &playLists, bool justMediaList = false);
	void setMediaObject(Phonon::MediaObject *MediaObject);

private:
	QTreeWidgetItem *previousItem;
	QVBoxLayout *hLayout;
	QTreeWidget *mediaList;
	bool itemsAsTopItem;
	Phonon::MediaObject *playListMediaObject;

private slots:
	void mediaObjectStateChanged(Phonon::State newState, Phonon::State oldState);
	void itemPlayRequested(QTreeWidgetItem*,int);
	void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void currentMediaChanged(const QString &fileName, const QString &title, int mediaID);

signals:
	void mediaPlayRequested(int/*, const QString &, const QString &*/);//mediaID, fileName, title!
};

/////////////////////////////////
#include <QWidget>
#include <QStaticText>
#include <QTimer>


class ScrollText : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText)
	Q_PROPERTY(QString separator READ separator WRITE setSeparator)

public:
	explicit ScrollText(QWidget *parent = 0);

public slots:
	QString text() const;
	void setText(QString text);

	QString separator() const;
	void setSeparator(QString separator);


protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *);

private:
	void updateText();
	QString _text;
	QString _separator;
	QStaticText staticText;
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
#endif // QMUSICPLAYER_H
