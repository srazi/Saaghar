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

#ifndef CONCURRENTTASKS_H
#define CONCURRENTTASKS_H

#include <QMutex>
#include <QRunnable>
#include <QtConcurrentRun>
#include <QVariant>

#if QT_VERSION < 0x050000
#include <QWeakPointer>
#else
#include <QPointer>
#endif

class ConcurrentTask : public QObject, QRunnable
{
    Q_OBJECT

public:
    explicit ConcurrentTask(QObject* parent = 0);
    ~ConcurrentTask();

    void start(const QString &type, const QVariantHash &argumants = QVariantHash());

#ifdef SAAGHAR_DEBUG
    void directStart(const QString &type, const QVariantHash &argumants = QVariantHash());
#endif

    void run();

    QVariant startSearch(const QVariantHash &options);

public slots:
    void setCanceled();

private:
    bool isCanceled();

    QMutex m_mutex;
    QString m_type;
    QVariantHash m_options;
    bool m_cancel;

    bool m_displayFullNotification;

    QFutureInterface<void> *m_progressObject;

signals:
    void concurrentResultReady(const QString &type, const QVariant &results);
    void searchStatusChanged(const QString &);
};

class ConcurrentTaskManager : public QObject
{
    Q_OBJECT

public:
    static ConcurrentTaskManager* instance();
    ~ConcurrentTaskManager();

    void addConcurrentTask(ConcurrentTask* task);

    bool isAllTaskCanceled();
    void switchToStartState();

public slots:
    void finish();

private:
    Q_DISABLE_COPY(ConcurrentTaskManager)
    ConcurrentTaskManager(QObject* parent = 0);
    static ConcurrentTaskManager* s_instance;

#if QT_VERSION < 0x050000
    typedef QWeakPointer<ConcurrentTask> TaskPointer;
#else
    typedef QPointer<ConcurrentTask> TaskPointer;
#endif

    QList<TaskPointer> m_tasks;
    bool m_cancel;
};

#endif // CONCURRENTTASKS_H
