/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2012 by S. Razi Alavizadeh                          *
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

namespace Ui {
	class DataBaseUpdater;
}

class Downloader;

class DataBaseUpdater : public QDialog
{
	Q_OBJECT

public:
	DataBaseUpdater(QWidget *parent = 0, Qt::WindowFlags f = 0);
	QStringList repositories();
	void setRepositories(const QStringList &urls);
	void readRepositories();

	bool read(QIODevice *device);
	static QString downloadLocation;

private slots:
	void initDownload();
	void forceStopDownload();
	void doStopDownload();
	void getDownloadLocation();
	void itemDataChanged(QTreeWidgetItem *item, int column);

private:
	bool installCompleted;
	void installItemToDB(const QString &fileName, const QString &path, const QString &fileType);
	QString getTempDir(const QString &path = "", bool makeDir = false);
	QString randomFolder;
	Downloader *downloaderObject;
	QString sessionDownloadFolder;
	void downloadItem(QTreeWidgetItem *item, bool install = false);
	bool downloadAboutToStart;
	bool downloadStarted;
	QHash <int, QString> insertedToList;
	Ui::DataBaseUpdater *ui;
	void setupUi();
	QTreeWidget *repoSelectTree;
	QTreeWidgetItem *oldRootItem;
	QTreeWidgetItem *newRootItem;
	QStringList repositoriesUrls;
	void parseElement(const QDomElement &element);
	QDomDocument domDocument;

protected:
	void closeEvent(QCloseEvent *);

signals:
	void installRequest(const QString &, bool *);
};

#endif
