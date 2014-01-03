/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2013 by S. Razi Alavizadeh                          *
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

#ifndef QGANJOORDBBROWSER_H
#define QGANJOORDBBROWSER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QProgressDialog>
#include <QVariant>

#include "databaseupdater.h"
#include "qganjoordbstuff.h"
#include "settings.h"
#include "qtwin.h"

#include <QDebug>

#ifdef EMBEDDED_SQLITE
#include "sqlite-driver/qsql_sqlite.h"
#include "sqlite3.h"
#endif

class QTreeWidgetItem;

class QGanjoorDbBrowser : public QObject
{
    Q_OBJECT
public:
    QGanjoorDbBrowser(QString sqliteDbCompletePath = "ganjoor.s3db", QWidget* splashScreen = 0);
    ~QGanjoorDbBrowser();
//      static QString qStringMacHelper(const QString &str);
    static QString getLongPathName(const QString &fileName);
    static QString simpleCleanString(const QString &text);
    static QString cleanString(const QString &text, const QStringList &excludeList = QStringList() << " ");
    static QString cleanStringFast(const QString &text, const QStringList &excludeList = QStringList() << " ");
    static QString justifiedText(const QString &text, const QFontMetrics &fontmetric, int width);
    static QString snippedText(const QString &text, const QString &str, int from = 0, int maxNumOfWords = 10, bool elided = true, Qt::TextElideMode elideMode = Qt::ElideRight);
    static int getRandomNumber(int minBound, int maxBound);

    bool isConnected(const QString &connectionID = "");
    bool isValid(QString connectionID = "");

    bool isRhyme(const QList<GanjoorVerse*> &verses, const QString &phrase, int verseOrder = -1);
    bool isRadif(const QList<GanjoorVerse*> &verses, const QString &phrase, int verseOrder = -1);

    QVariantList importGanjoorBookmarks();
    QString getBeyt(int poemID, int firstMesraID,  const QString &separator = "       "/*7 spaces*/);

    QList<GanjoorPoet*> getDataBasePoets(const QString fileName);
    QList<GanjoorPoet*> getConflictingPoets(const QString fileName);
    QList<GanjoorPoet*> getPoets(const QString &connectionID = "", bool sort = true);
    QList<GanjoorCat*> getSubCategories(int CatID);
    QList<GanjoorCat> getParentCategories(GanjoorCat Cat);
    QList<GanjoorPoem*> getPoems(int CatID);
    QList<GanjoorVerse*> getVerses(int PoemID);
    QList<GanjoorVerse*> getVerses(int PoemID, int Count);

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
    QString getPoemMediaSource(int PoemID);
    void setPoemMediaSource(int PoemID, const QString &fileName);
    //Search
    //QList<int> getPoemIDsContainingPhrase(QString phrase, int PageStart, int Count, int PoetID);
    //QString getFirstVerseContainingPhrase(int PoemID, QString phrase);
    //new Search Method
    //QList<int> getPoemIDsContainingPhrase_NewMethod(const QString &phrase, int PoetID, bool skipNonAlphabet);
    //QStringList getVerseListContainingPhrase(int PoemID, const QString &phrase);
    //another new approch
    QMap<int, QString> getPoemIDsByPhrase(int PoetID, const QStringList &phraseList, const QStringList &excludedList = QStringList(), bool* canceled = 0, int resultCount = 0, bool slowSearch = false);

    //Faal
    int getRandomPoemID(int* CatID);
    void removePoetFromDataBase(int PoetID);
    bool importDataBase(const QString filename);
    //QSqlDatabase dBConnection;

    QList<QTreeWidgetItem*> loadOutlineFromDataBase(int parentID = 0);

    //STATIC Variables
    static QString dBName;
    static DataBaseUpdater* dbUpdater;
    static QStringList dataBasePath;
    static const QStringList someSymbols;
    static const QStringList Ve_Variant;
    static const QStringList Ye_Variant;
    static const QStringList AE_Variant;
    static const QStringList He_Variant;

public slots:
    void addDataSets();

private:
    bool createEmptyDataBase(const QString &connectionID = "");
    bool poetHasSubCats(int poetID, const QString &connectionID = "");
    int cachedMaxCatID, cachedMaxPoemID;
    int getNewPoemID();
    int getNewPoetID();
    int getNewCatID();
    void removeCatFromDataBase(const GanjoorCat &gCat);
    static QString getIdForDataBase(const QString &sqliteDataBaseName);
    //QString dBName;
    QSqlDatabase dBConnection;
    static bool comparePoetsByName(GanjoorPoet* poet1, GanjoorPoet* poet2);
    static bool compareCategoriesByName(GanjoorCat* cat1, GanjoorCat* cat2);
    bool m_addRemoteDataSet;

signals:
    void searchStatusChanged(const QString &);

#ifdef EMBEDDED_SQLITE
private:
    static QSQLiteDriver* sqlDriver;
#endif
};
#endif // QGANJOORDBBROWSER_H
