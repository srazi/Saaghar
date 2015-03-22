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

#include "saagharapplication.h"
#include "saagharwindow.h"
#include "tools.h"
#include "progressmanager_p.h"
#include "progressmanager.h"
#include "progressview.h"
#include "settings.h"
#include "databasebrowser.h"

#include <QExtendedSplashScreen>

//#include<QMessageBox>
#include <QPointer>
#include <QThreadPool>

#ifdef Q_OS_WIN
#ifdef STATIC
#include <QtPlugin>
Q_IMPORT_PLUGIN(qsqlite)
#endif
#endif

namespace {
    QThread::Priority m_tasksPriority = QThread::LowPriority;
    static const int NORMAL_TASKS_THREADS = QThread::idealThreadCount();
}

SaagharApplication::SaagharApplication(int &argc, char **argv)
    : QApplication(argc, argv, true),
      m_mainWindow(0),
      m_progressManager(0),
      m_tasksThreadPool(0),
      m_databaseBrowser(0),
      m_tasksThreads(NORMAL_TASKS_THREADS),
      m_displayFullNotification(true),
      m_notificationPosition(ProgressManager::DesktopBottomRight)
{
    init();

    // TODO: centralize settings loding/saving/applying
    applySettings();
}

SaagharApplication::~SaagharApplication()
{
    delete m_mainWindow;
    delete m_progressManager;
    delete m_databaseBrowser;
}

SaagharApplication *SaagharApplication::instance()
{
    return static_cast<SaagharApplication*>(QCoreApplication::instance());
}

ProgressManager* SaagharApplication::progressManager()
{
    if (!m_progressManager) {
        m_progressManager = new ProgressManagerPrivate;

        m_progressManager->init();
        m_progressManager->progressView()->setReferenceWidget(m_mainWindow);
        m_progressManager->progressView()->setProgressWidgetVisible(m_displayFullNotification);
        m_progressManager->progressView()->setPosition(m_notificationPosition);
    }

    return m_progressManager;
}

QThreadPool *SaagharApplication::tasksThreadPool()
{
    if (!m_tasksThreadPool) {
        m_tasksThreadPool = new QThreadPool(this);

        m_tasksThreadPool->setMaxThreadCount(m_tasksThreads);
    }

    return m_tasksThreadPool;
}

DatabaseBrowser *SaagharApplication::databaseBrowser()
{
    if (!m_databaseBrowser) {
        m_databaseBrowser = DatabaseBrowser::instance();
    }

    return m_databaseBrowser;
}

void SaagharApplication::applySettings()
{
    m_notificationPosition = ProgressManager::Position(Settings::READ("TaskManager/Notification", ProgressManager::DesktopBottomRight).toInt());
    const QString mode = Settings::READ("TaskManager/Mode", "NORMAL").toString();

    if (mode == "SLOW") {
        m_tasksThreads = QThread::idealThreadCount() > 1 ? (QThread::idealThreadCount() - 1) : 1;
        m_tasksPriority = QThread::LowPriority;
        m_displayFullNotification = true;
    }
    else if (mode == "FAST") {
        m_tasksThreads = QThread::idealThreadCount();
        m_tasksPriority = QThread::NormalPriority;
        m_displayFullNotification = false;
    }
    else { // fallback to "NORMAL"
        m_tasksThreads = NORMAL_TASKS_THREADS;
        m_tasksPriority = QThread::LowPriority;
        m_displayFullNotification = true;
    }

    if (m_tasksThreadPool) {
        m_tasksThreadPool->setMaxThreadCount(m_tasksThreads);
    }

    if (m_progressManager) {
        if (sApp->notificationPosition() == ProgressManager::Disabled) {
            delete m_progressManager;
            m_progressManager = 0;
        }
        else {
            m_progressManager->progressView()->setProgressWidgetVisible(m_displayFullNotification);
            m_progressManager->progressView()->setPosition(m_notificationPosition);
        }
    }
}

void SaagharApplication::setPriority(QThread* thread)
{
    if (thread) {
        thread->setPriority(m_tasksPriority);
    }
}

void SaagharApplication::init()
{
    //'At Development Stage' message
    //QMessageBox::information(0, QObject::tr("At Development Stage"), QObject::tr("This is an experimental version! Don\'t release it!\nWWW: http://saaghar.pozh.org"));

    QPixmap pixmap(":/resources/images/saaghar-splash.png");
    QPixmap mask(":/resources/images/saaghar-splash-mask.png");
    QExtendedSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.setMessageOptions(QRect(QPoint(120, 525), QSize(310, qMin(splash.fontMetrics().height() + 2, 18))),
                             Qt::AlignLeft | Qt::AlignVCenter, QColor(7, 12, 150));
    splash.setProgressBar(mask, 0, 10, Qt::Vertical);
    Tools::setSplashScreen(&splash);

    m_mainWindow = new SaagharWindow;
    m_mainWindow->show();

    splash.finish(m_mainWindow);
}
