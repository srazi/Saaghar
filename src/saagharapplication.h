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

#ifndef SAAGHARAPPLICATION_H
#define SAAGHARAPPLICATION_H

#define sApp SaagharApplication::instance()

#include "progressmanager.h"

#include <QApplication>

class SaagharWindow;

class QThread;
class QThreadPool;

class SaagharApplication : public QApplication
{
public:
    SaagharApplication(int &argc, char **argv);
    ~SaagharApplication();

    static SaagharApplication* instance();

    ProgressManager* progressManager();
    QThreadPool* tasksThreadPool();

    void applySettings();

    int tasksThreads() { return m_tasksThreads; }
    void setPriority(QThread* thread);
    bool displayFullNotification() { return m_displayFullNotification; }
    ProgressManager::Position notificationPosition() const { return m_notificationPosition; }

private:
    void init();

    SaagharWindow* m_mainWindow;

    ProgressManagerPrivate* m_progressManager;
    QThreadPool* m_tasksThreadPool;

    int m_tasksThreads;
    bool m_displayFullNotification;
    ProgressManager::Position m_notificationPosition;
};

#endif // SAAGHARAPPLICATION_H
