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
#include "databasebrowser.h"
#include "tools.h"
#include "progressmanager.h"
#include "saagharapplication.h"
#include "futureprogress.h"

#include <QMetaType>
#include <QThread>
#include <QThreadPool>

#ifdef SAAGHAR_DEBUG
#include <QDateTime>
#endif

#define TASK_CANCELED if (isCanceled()) return QVariant();

QThreadPool* ConcurrentTask::s_concurrentTasksPool = 0;
QList<QWeakPointer<ConcurrentTask> > ConcurrentTask::s_tasks;
bool ConcurrentTask::s_cancel = false;

ConcurrentTask::ConcurrentTask(QObject *parent)
    : QObject(parent),
      QRunnable(),
      m_cancel(false),
      m_progressObject(0)
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
    if (s_cancel) {
        return;
    }

    m_type = type;
    m_options = argumants;

    m_progressObject = new QFutureInterface<void>;

    FutureProgress* fp = sApp->progressManager()->addTimedTask(*m_progressObject, VAR_GET(m_options, taskTitle).toString(),
                                          m_type, 5,
                                          ProgressManager::ShowInApplicationIcon);

    connect(fp, SIGNAL(canceled()), this, SLOT(setCanceled()));

    s_tasks.append(this);
    concurrentTasksPool()->start(this);
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
}
#endif

void ConcurrentTask::run()
{
#ifdef SAAGHAR_DEBUG
    qint64 start = QDateTime::currentMSecsSinceEpoch();
#endif

    QThread::Priority prio = QThread::currentThread()->priority();
    prio = prio == QThread::InheritPriority ? QThread::NormalPriority : prio;
    QThread::currentThread()->setPriority(QThread::LowPriority);

    m_progressObject->reportStarted();

    QVariant result = startSearch(m_options);

    m_progressObject->reportFinished();
    delete m_progressObject;
    m_progressObject = 0;

    QThread::currentThread()->setPriority(prio);

#ifdef SAAGHAR_DEBUG
    qint64 end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __LINE__ << __FUNCTION__ << (end - start);
#endif

    emit concurrentResultReady(m_type, result);
}

void ConcurrentTask::finish()
{
    s_cancel = true;

    foreach (QWeakPointer<ConcurrentTask> wp, s_tasks) {
        if (wp && wp.data()) {
            wp.data()->setCanceled();
        }
    }

    if (s_concurrentTasksPool) {
        s_concurrentTasksPool->waitForDone();

        delete s_concurrentTasksPool;
        s_concurrentTasksPool = 0;
    }
}

QThreadPool *ConcurrentTask::concurrentTasksPool()
{
    if (!s_concurrentTasksPool) {
        s_concurrentTasksPool = new QThreadPool(QThreadPool::globalInstance());

        s_concurrentTasksPool->setMaxThreadCount(qBound(2, QThread::idealThreadCount() * 2, 12));
    }

    return s_concurrentTasksPool;
}

QVariant ConcurrentTask::startSearch(const QVariantHash &options)
{
    TASK_CANCELED;

    const QString &strQuery = VAR_GET(options, strQuery).toString();
    int PoetID = VAR_GET(options, PoetID).toInt();
    const QStringList &phraseList = VAR_GET(options, phraseList).toStringList();
    const QStringList &excludedList = VAR_GET(options, excludedList).toStringList();
    const QStringList &excludeWhenCleaning = VAR_GET(options, excludeWhenCleaning).toStringList();

    SearchResults searchResults;

    QSqlDatabase threadDatabase = dbBrowser->databaseForThread(QThread::currentThread());
    if (!threadDatabase.isOpen()) {
        qDebug() << QString("ConcurrentTask::startSearch: A database for thread %1 could not be opened!").arg(QString::number((quintptr)QThread::currentThread()));
        return QVariant();
    }

    int andedPhraseCount = phraseList.size();
    int excludedCount = excludedList.size();
    int numOfFounded = 0;

    TASK_CANCELED;

    QSqlQuery q(threadDatabase);

#ifdef SAAGHAR_DEBUG
    int start = QDateTime::currentDateTime().toTime_t() * 1000 + QDateTime::currentDateTime().time().msec();
#endif

    q.exec(strQuery);

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

    TASK_CANCELED

    while (q.next()) {
        TASK_CANCELED

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

                        TASK_CANCELED;

                        verses = dbBrowser->getVerses(poemID);
                    }

                    TASK_CANCELED;

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

                        TASK_CANCELED;

                        verses = dbBrowser->getVerses(poemID);
                    }

                    TASK_CANCELED;

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

        if (excludeCurrentVerse) {
            continue;
        }

        TASK_CANCELED;

        ++numOfFounded;

        if (numOfFounded > nextStep) {
            nextStep += updateLenght;
            emit searchStatusChanged(DatabaseBrowser::tr("Search Result(s): %1").arg(numOfFounded));
        }

        GanjoorPoem gPoem = dbBrowser->getPoem(poemID);
        searchResults.insertMulti(poemID, "verseText=" + verseText + "|poemTitle=" + gPoem._Title + "|poetName=" + dbBrowser->getPoetForCat(gPoem._CatID)._Name);
    }

    //for the last result
    emit searchStatusChanged(DatabaseBrowser::tr("Last-Search Result(s): %1").arg(numOfFounded));

    qDeleteAll(verses);

    TASK_CANCELED;

    return QVariant::fromValue(searchResults);
}

bool ConcurrentTask::isCanceled()
{
    return m_cancel;
}

void ConcurrentTask::setCanceled()
{
    m_mutex.lock();
    m_cancel = true;
    m_mutex.unlock();
}
