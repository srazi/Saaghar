/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://www.pojh.co.cc>       *
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
 
#ifndef QGANJOORDBBROWSER_H
#define QGANJOORDBBROWSER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "QGanjoorDbStuff.h"

#include <QDebug>

class QGanjoorDbBrowser : public QObject
{
	Q_OBJECT
    public:
		QGanjoorDbBrowser(QString sqliteDbCompletePath = "ganjoor.s3db");
		~QGanjoorDbBrowser();
		bool isConnected();

		QList<GanjoorPoet *> getPoets();
		QList<GanjoorCat *> getSubCategories(int CatID);
		QList<GanjoorCat> getParentCategories(GanjoorCat Cat);
		QList<GanjoorPoem *> getPoems(int CatID);
		QList<GanjoorVerse *> getVerses(int PoemID);
		QList<GanjoorVerse *> getVerses(int PoemID, int Count);

		QString getFirstMesra(int PoemID);//just first Mesra
		GanjoorCat getCategory(int CatID);
		GanjoorPoem getPoem(int PoemID);
		GanjoorPoem getNextPoem(int PoemID, int CatID);
		GanjoorPoem getNextPoem(GanjoorPoem poem);
		GanjoorPoem getPreviousPoem(int PoemID, int CatID);
		GanjoorPoem getPreviousPoem(GanjoorPoem poem);
		GanjoorPoet getPoetForCat(int CatID);
		GanjoorPoet getPoet(int PoetID);
		GanjoorPoet getPoet(QString PoetName);
		QString getPoetDescription(int PoetID);
		//Search
		QList<int> getPoemIDsContainingPhrase(QString phrase, int PageStart, int Count, int PoetID);
		QString getFirstVerseContainingPhrase(int PoemID, QString phrase);
		//Faal
		int getRandomPoemID(int CatID);

    private:
		QString dBName;
		QSqlDatabase dBConnection;
		static bool comparePoetsByName(GanjoorPoet *poet1, GanjoorPoet *poet2);
		static bool compareCategoriesByName(GanjoorCat *cat1, GanjoorCat *cat2);
};
#endif // QGANJOORDBBROWSER_H
