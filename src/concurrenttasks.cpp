/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2015 by S. Razi Alavizadeh                          *
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

#include "concurrenttasks.h"
#include "tools.h"
#include "databaseelements.h"
#include "databasebrowser.h"

#include <QDebug>
#include <QApplication>
#include <QThreadPool>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMutexLocker>

#ifdef SAAGHAR_DEBUG
#include <QDateTime>
#endif

QThreadPool* ConcurrentTask::s_concurrentTasksPool = 0;
QHash<QThread*, QSqlDatabase> ConcurrentTask::s_databases = QHash<QThread*, QSqlDatabase>();
bool ConcurrentTask::s_cancel = false;

ConcurrentTask::ConcurrentTask(QObject *parent) :
    QObject(parent), QRunnable()
{
    // deleted by parent/child system of Qt
    setAutoDelete(false);
    qRegisterMetaType< SearchResults >("SearchResults");
}

ConcurrentTask::~ConcurrentTask()
{
}

void ConcurrentTask::start(const QString &type, const QVariantHash &argumants)
{
    m_type = type;
    m_options = argumants;
    s_cancel = false;

    concurrentTasksPool()->start(this, 5);
}

#ifdef SAAGHAR_DEBUG
void ConcurrentTask::directStart(const QString &type, const QVariantHash &argumants)
{
    m_type = type;
    m_options = argumants;
    qint64 start = QDateTime::currentMSecsSinceEpoch();

    QVariant result = startSearch(m_options);

    qint64 end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __LINE__ << __FUNCTION__ << (end - start) << "NON THREADED" ;

    emit concurrentResultReady(m_type, result);


    concurrentTasksPool()->start(this);
}
#endif

void ConcurrentTask::run()
{
#ifdef SAAGHAR_DEBUG
    qint64 start = QDateTime::currentMSecsSinceEpoch();
#endif

    if (ConcurrentTask::s_cancel) {
        return;
    }

    QVariant result = startSearch(m_options);

#ifdef SAAGHAR_DEBUG
    qint64 end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __LINE__ << __FUNCTION__ << (end - start);
#endif

    emit concurrentResultReady(m_type, result);
}

void ConcurrentTask::finish()
{
    if (s_concurrentTasksPool) {
        s_cancel = true;
        s_concurrentTasksPool->waitForDone();
        delete s_concurrentTasksPool;
        s_concurrentTasksPool = 0;
    }
}

QThreadPool *ConcurrentTask::concurrentTasksPool()
{
    if (!s_concurrentTasksPool) {
        s_concurrentTasksPool = new QThreadPool(QThreadPool::globalInstance());

        s_concurrentTasksPool->setMaxThreadCount(QThread::idealThreadCount() > 1 ? QThread::idealThreadCount() - 1 : 1);
    }

    return s_concurrentTasksPool;
}

#define TEST_CANCEL if (ConcurrentTask::s_cancel) return QVariant()

QVariant ConcurrentTask::startSearch(const QVariantHash &options)
{
    const QString &strQuery = VAR_GET(options, strQuery).toString();
    int PoetID = VAR_GET(options, PoetID).toInt();
    const QStringList &phraseList = VAR_GET(options, phraseList).toStringList();
    const QStringList &excludedList = VAR_GET(options, excludedList).toStringList();
    const QStringList &excludeWhenCleaning = VAR_GET(options, excludeWhenCleaning).toStringList();

    SearchResults searchResults;

    TEST_CANCEL;

    QSqlDatabase threadDatabase = databaseForThread(QThread::currentThread());
    if (!threadDatabase.isOpen()) {
        return QVariant();
    }

    int andedPhraseCount = phraseList.size();
    int excludedCount = excludedList.size();
    int numOfFounded = 0;

    TEST_CANCEL;

    QSqlQuery q(databaseForThread(QThread::currentThread()));

#ifdef SAAGHAR_DEBUG
    int start = QDateTime::currentDateTime().toTime_t() * 1000 + QDateTime::currentDateTime().time().msec();
#endif

    q.exec(strQuery);
    q.first();

#ifdef SAAGHAR_DEBUG
    int end = QDateTime::currentDateTime().toTime_t() * 1000 + QDateTime::currentDateTime().time().msec();
    int miliSec = end - start;
    qDebug() << "duration=" << miliSec;
#endif

    int numOfNearResult = 0;
    int nextStep = 0;
    const int updateLenght = 217;

    int lastPoemID = -1;
    QList<GanjoorVerse*> verses;

    while (q.next()) {
        ++numOfNearResult;

        QSqlRecord qrec = q.record();
        int poemID = qrec.value(0).toInt();

//          if (idList.contains(poemID))
//              continue;//we need just first result

        QString verseText = qrec.value(1).toString();

        // assume title's order is zero!
        int verseOrder = (PoetID == -1000 ? 0 : qrec.value(2).toInt());

        QString foundVerse = Tools::cleanStringFast(verseText, excludeWhenCleaning);

        // for whole word option when word is in the start or end of verse
        foundVerse = " " + foundVerse + " ";

        //excluded list
        bool excludeCurrentVerse = false;
        for (int t = 0; t < excludedCount; ++t) {
            if (foundVerse.contains(excludedList.at(t))) {
                excludeCurrentVerse = true;
                break;
            }
        }

        if (!excludeCurrentVerse) {
            for (int t = 0; t < andedPhraseCount; ++t) {
                QString tphrase = phraseList.at(t);
                if (tphrase.contains("==")) {
                    tphrase.remove("==");
                    if (lastPoemID != poemID/* && findRhyme*/) {
                        lastPoemID = poemID;
                        int versesSize = verses.size();
                        for (int j = 0; j < versesSize; ++j) {
                            delete verses[j];
                            verses[j] = 0;
                        }

                        TEST_CANCEL;

                        verses = dbBrowser->getVerses(poemID);
                    }

                    TEST_CANCEL;

                    excludeCurrentVerse = !dbBrowser->isRadif(verses, tphrase, verseOrder);
                    break;
                }
                if (tphrase.contains("=")) {
                    tphrase.remove("=");
                    if (lastPoemID != poemID/* && findRhyme*/) {
                        lastPoemID = poemID;
                        int versesSize = verses.size();
                        for (int j = 0; j < versesSize; ++j) {
                            delete verses[j];
                            verses[j] = 0;
                        }

                        TEST_CANCEL;

                        verses = dbBrowser->getVerses(poemID);
                    }

                    TEST_CANCEL;

                    excludeCurrentVerse = !dbBrowser->isRhyme(verses, tphrase, verseOrder);
                    break;
                }
                if (!tphrase.contains("%")) {
                    //QChar(71,6): Simple He
                    //QChar(204,6): Persian Ye
                    QString YeAsKasre = QString(QChar(71, 6)) + " ";
                    if (tphrase.contains(YeAsKasre)) {
                        tphrase.replace(YeAsKasre, QString(QChar(71, 6)) + "\\s*" + QString(QChar(204, 6)) +
                                        "{0,2}\\s+");

                        QRegExp anySearch(".*" + tphrase + ".*", Qt::CaseInsensitive);

                        if (!anySearch.exactMatch(foundVerse)) {
                            excludeCurrentVerse = true;
                            break;
                        }
                    }
                    else {
                        if (!foundVerse.contains(tphrase))
                            //the verse doesn't contain an ANDed phrase
                            //maybe for ++ and +++ this should be removed
                        {
                            excludeCurrentVerse = true;
                            break;
                        }
                    }
                }
                else {
                    tphrase = tphrase.replace("%%", ".*");
                    tphrase = tphrase.replace("%", "\\S*");
                    QRegExp anySearch(".*" + tphrase + ".*", Qt::CaseInsensitive);
                    if (!anySearch.exactMatch(foundVerse)) {
                        excludeCurrentVerse = true;
                        break;
                    }
                }
            }
        }

//        if (Canceled && *Canceled) {
//            break;
//        }

        if (excludeCurrentVerse) {
            continue;
        }

        ++numOfFounded;

        if (numOfFounded > nextStep) {
            nextStep += updateLenght;
            emit searchStatusChanged(DatabaseBrowser::tr("Search Result(s): %1").arg(numOfFounded));
        }

        TEST_CANCEL;

        GanjoorPoem gPoem = dbBrowser->getPoem(poemID);
        searchResults.insertMulti(poemID, "verseText=" + verseText + "|poemTitle=" + gPoem._Title + "|poetName=" + dbBrowser->getPoetForCat(gPoem._CatID)._Name);
    }

    //for the last result
    emit searchStatusChanged(DatabaseBrowser::tr("Last-Search Result(s): %1").arg(numOfFounded));

    qDeleteAll(verses);

    return QVariant::fromValue(searchResults);
}

QSqlDatabase ConcurrentTask::databaseForThread(QThread* thread)
{
    QMutexLocker lock(&m_mutex);

    if (!s_databases.contains(thread)) {
        const QString threadStr = QString::number((quintptr) thread);

        s_databases[thread] = QSqlDatabase::cloneDatabase(dbBrowser->database(), threadStr + "/" + dbBrowser->database().connectionName());
        s_databases[thread].open();
    }

    Q_ASSERT(s_databases.value(thread).isOpen());

    return s_databases.value(thread);
}
