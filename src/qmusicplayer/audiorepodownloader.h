/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2016 by S. Razi Alavizadeh                               *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 3 of the License,         *
 *  (at your option) any later version                                     *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details                            *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program; if not, see http://www.gnu.org/licenses/      *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDIOREPODOWNLOADER_H
#define AUDIOREPODOWNLOADER_H

#include "ui_audiorepodownloader.h"
#include "qmusicplayer.h"

#include <QDomDocument>
#include <QHash>
#include <QIcon>
#include <QTreeWidget>

namespace Ui
{
class AudioRepoDownloader;
}

class SaagharWindow;
class Downloader;

class AudioRepoDownloader : public QDialog
{
    Q_OBJECT

public:
    AudioRepoDownloader(QWidget* parent = 0, Qt::WindowFlags f = 0);
    static QStringList repositories();
    static void setRepositories(const QStringList &urls);

    bool read(QIODevice* device);
    bool read(const QByteArray &data);
    static QString downloadLocation;
    static bool keepDownloadedFiles;

public slots:
    void readRepository(const QString &url = QString());

private slots:
    virtual void reject();
    void initDownload();
    void forceStopDownload();
    bool doStopDownload();
    void getDownloadLocation();
    void itemDataChanged(QTreeWidgetItem* item, int column);
    void switchGroupingType();
    void lighRefreshTree();

private:
    enum GroupType {
        GroupByPoet,
        GroupByVoice
    };

    enum AudioItemRole {
        AudioPostIDRole = Qt::UserRole + 1,
        AudioOrderRole = Qt::UserRole + 2,
        AudioChecksumRole = Qt::UserRole + 3,
        AudioSrcRole = Qt::UserRole + 4,
        AudioArtistRole = Qt::UserRole + 5,
        AudioSrcUrlRole = Qt::UserRole + 6,
        AudioArtistUrlRole = Qt::UserRole + 7,
        AudioXmlRole = Qt::UserRole + 8,
        AudioMP3Role = Qt::UserRole + 9,
        AudioGuidRole = Qt::UserRole + 10,
        AudioTitleRole = Qt::UserRole + 11
    };

    void addRemoveRepository();
    void resizeColumnsToContents();
    bool parseDocument();
    void setupTreeRootItems();
    QString getTempDir(const QString &path = "", bool makeDir = false);
    bool downloadItem(QTreeWidgetItem* item, bool addToAlbum = false);
    void setupUi();
    void parseElement(const QDomElement &element);
    void fillRepositoryList();

    void setDisabledAll(bool disable);
    bool loadPresentAudios();
    void downloadCheckedItem(QTreeWidgetItem* rootItem);

    QHash<QString, QPair<QTreeWidgetItem*, QTreeWidgetItem*> > itemsCache;
    QHash<int, QString> poemToPoetCache;

    const static QStringList defaultRepositories;
    static QStringList repositoriesUrls;

    QString randomFolder;
    Downloader* downloaderObject;
    QString sessionDownloadFolder;
    bool downloadAboutToStart;
    bool downloadStarted;
    QHash <QString, QString> insertedToList;
    Ui::AudioRepoDownloader* ui;
    QTreeWidget* repoSelectTree;
    QTreeWidgetItem* oldRootItem;
    QTreeWidgetItem* newRootItem;
    QDomDocument domDocument;

    QMusicPlayer::SaagharAlbum* m_saagharAlbum;
    GroupType m_groupType;

protected:
    void closeEvent(QCloseEvent*);
};

#endif // AUDIOREPODOWNLOADER_H
