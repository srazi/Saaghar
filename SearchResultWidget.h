/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
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

#ifndef SEARCHRESULTWIDGET_H
#define SEARCHRESULTWIDGET_H

#include <QMainWindow>
#include <QHash>
#include <QTableWidget>
#include <QToolButton>
#include <QAction>

const int ITEM_SEARCH_DATA = Qt::UserRole+10;


class SearchResultWidget : public QWidget
{ 
	Q_OBJECT

public:
	SearchResultWidget(QWidget *parent = 0, const QString &searchPhrase = "", int count = 0, const QString &poetName = "");
	~SearchResultWidget();

	bool init(QMainWindow *qmw, const QString &iconThemePath);
	//QHash<int, QString> resultList;//random order
	QMap<int, QString> resultList;
	QTableWidget *searchTable;

private:
	QWidget *searchResultContents;
	void setupUi(QMainWindow *qmw, const QString &iconThemePath);
	void showSearchResult(int start);
	static int maxItemPerPage;
	QString phrase;
	bool navButtonsNeeded;
	bool moreThanOnePage;
	QString sectionName;
	QToolButton *searchNextPage, *searchPreviousPage;
	QAction *actSearchNextPage, *actSearchPreviousPage;

private slots:
	void searchPageNavigationClicked(QAction *action);
	void setMaxItemPerPage(int value);
};

#endif // SEARCHRESULTWIDGET_H
