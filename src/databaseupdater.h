/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2016 by S. Razi Alavizadeh                          *
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

#ifndef DATABASEUPDATER_H
#define DATABASEUPDATER_H

#include "ui_databaseupdater.h"

#include <QDomDocument>
#include <QHash>
#include <QIcon>
#include <QTreeWidget>

namespace Ui
{
class DataBaseUpdater;
}

class SaagharWindow;
class Downloader;

class DataBaseUpdater : public QDialog
{
    Q_OBJECT

public:
    DataBaseUpdater(QWidget* parent = 0, Qt::WindowFlags f = 0);
    static QStringList repositories();
    static void setRepositories(const QStringList &urls);

    void installItemToDB(const QString &fileName, const QString &filePath, const QString &fileType);
    void installItemToDB(const QString &fullFilePath, const QString &fileType = "");
    bool read(QIODevice* device);
    bool read(const QByteArray &data);
    static QString downloadLocation;
    static bool keepDownloadedFiles;

public slots:
    void readRepository(const QString &url = "");

private slots:
    virtual void reject();
    void initDownload();
    void forceStopDownload();
    bool doStopDownload();
    void getDownloadLocation();
    void itemDataChanged(QTreeWidgetItem* item, int column);

private:
    void addRemoveRepository();
    void resizeColumnsToContents();
    bool parseDocument();
    void setupTreeRootItems();
    QString getTempDir(const QString &path = "", bool makeDir = false);
    void downloadItem(QTreeWidgetItem* item, bool install = false);
    void setupUi();
    void parseElement(const QDomElement &element);
    void fillRepositoryList();
    void importDataBase(const QString &fileName, bool* ok = 0);

    QHash<QString, QPair<QTreeWidgetItem*, QTreeWidgetItem*> > itemsCache;
    const static QStringList defaultRepositories;
    static QStringList repositoriesUrls;
    bool installCompleted;
    QString randomFolder;
    Downloader* downloaderObject;
    QString sessionDownloadFolder;
    bool downloadAboutToStart;
    bool downloadStarted;
    QHash <int, QString> insertedToList;
    Ui::DataBaseUpdater* ui;
    QTreeWidget* repoSelectTree;
    QTreeWidgetItem* oldRootItem;
    QTreeWidgetItem* newRootItem;
    QDomDocument domDocument;

protected:
    void closeEvent(QCloseEvent*);
#if QT_VERSION >= 0x050000
    void paintEvent(QPaintEvent* event);
#endif
};

#endif
