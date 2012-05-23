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
	DataBaseUpdater(const QList<int> &IDs, QWidget *parent = 0);
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
	QString getTempDir();
	QString randomFolder;
	Downloader *downloaderObject;
	QString sessionDownloadFolder;
	void downloadItem(QTreeWidgetItem *item);
	bool downloadAboutToStart;
	bool downloadStarted;
	QHash <int, QString> insertedToList;
	Ui::DataBaseUpdater *ui;
	QList<int> availableIDs;
	void setupUi();
	QTreeWidget *repoSelectTree;
	QTreeWidgetItem *oldRootItem;
	QTreeWidgetItem *newRootItem;
	QStringList repositoriesUrls;
	void parseElement(const QDomElement &element);
	QDomDocument domDocument;

protected:
	void closeEvent(QCloseEvent *);
};

#endif
