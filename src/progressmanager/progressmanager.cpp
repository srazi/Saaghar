/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2015-2016 by S. Razi Alavizadeh                          *
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

/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "futureprogress.h"
#include "progressmanager_p.h"
#include "progressbar.h"
#include "progressview.h"
#include "saagharapplication.h"

#include <QAction>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <QVariant>

#include <math.h>

static const char kSettingsGroup[] = "Progress";
static const char kDetailsPinned[] = "DetailsPinned";

static int countOfVisibleProgresses()
{
    return sApp->tasksThreads() + 1;
}

/*!
    \mainclass
    \class ProgressManager
    \brief The ProgressManager class is used to show a user interface
    for running tasks in Qt Creator.

    It tracks the progress of a task that it is told
    about, and shows a progress indicator in the lower right
    of Qt Creator's main window to the user.
    The progress indicator also allows the user to cancel the task.

    You get the single instance of this class via the
    ProgressManager::instance() function.

    \section1 Registering a task
    The ProgressManager API uses QtConcurrent as the basis for defining
    tasks. A task consists of the following properties:

    \table
    \header
        \li Property
        \li Type
        \li Description
    \row
        \li Task abstraction
        \li \c QFuture<void>
        \li A \c QFuture object that represents the task which is
           responsible for reporting the state of the task. See below
           for coding patterns how to create this object for your
           specific task.
    \row
        \li Title
        \li \c QString
        \li A very short title describing your task. This is shown
           as a title over the progress bar.
    \row
        \li Type
        \li \c QString
        \li A string identifier that is used to group different tasks that
           belong together.
           For example, all the search operations use the same type
           identifier.
    \row
        \li Flags
        \li \l ProgressManager::ProgressFlags
        \li Additional flags that specify how the progress bar should
           be presented to the user.
    \endtable

    To register a task you create your \c QFuture<void> object, and call
    addTask(). This function returns a
    \l{FutureProgress}{FutureProgress}
    object that you can use to further customize the progress bar's appearance.
    See the \l{FutureProgress}{FutureProgress} documentation for
    details.

    In the following you will learn about two common patterns how to
    create the \c QFuture<void> object for your task.

    \section2 Create a threaded task with QtConcurrent
    The first option is to directly use QtConcurrent to actually
    start a task concurrently in a different thread.
    QtConcurrent has several different functions to run e.g.
    a class function in a different thread. Qt Creator itself
    adds a few more in \c{src/libs/qtconcurrent/runextensions.h}.
    The QtConcurrent functions to run a concurrent task return a
    \c QFuture object. This is what you want to give the
    ProgressManager in the addTask() function.

    Have a look at e.g ILocatorFilter. Locator filters implement
    a function \c refresh which takes a \c QFutureInterface object
    as a parameter. These functions look something like:
    \code
    void Filter::refresh(QFutureInterface<void> &future) {
        future.setProgressRange(0, MAX);
        ...
        while (!future.isCanceled()) {
            // Do a part of the long stuff
            ...
            future.setProgressValue(currentProgress);
            ...
        }
    }
    \endcode

    The actual refresh, which calls all the filters' refresh functions
    in a different thread, looks like this:
    \code
    QFuture<void> task = QtConcurrent::run(&ILocatorFilter::refresh, filters);
    FutureProgress *progress = ProgressManager::addTask(task, tr("Indexing"),
                                                                    Locator::Constants::TASK_INDEX);
    \endcode
    First, we tell QtConcurrent to start a thread which calls all the filters'
    refresh function. After that we register the returned QFuture object
    with the ProgressManager.

    \section2 Manually create QtConcurrent objects for your thread
    If your task has its own means to create and run a thread,
    you need to create the necessary objects yourselves, and
    report the start/stop state.

    \code
    // We are already running in a different thread here
    QFutureInterface<void> *progressObject = new QFutureInterface<void>;
    progressObject->setProgressRange(0, MAX);
    ProgressManager::addTask(progressObject->future(), tr("DoIt"), MYTASKTYPE);
    progressObject->reportStarted();
    // Do something
    ...
    progressObject->setProgressValue(currentProgress);
    ...
    // We have done what we needed to do
    progressObject->reportFinished();
    delete progressObject;
    \endcode
    In the first line we create the QFutureInterface object that will be
    our way for reporting the task's state.
    The first thing we report is the expected range of the progress values.
    We register the task with the ProgressManager, using the internal
    QFuture object that has been created for our QFutureInterface object.
    Next we report that the task has begun and start doing our actual
    work, regularly reporting the progress via the functions
    in QFutureInterface. After the long taking operation has finished,
    we report so through the QFutureInterface object, and delete it
    afterwards.

    \section1 Customizing progress appearance

    You can set a custom widget to show below the progress bar itself,
    using the FutureProgress object returned by the addTask() function.
    Also use this object to get notified when the user clicks on the
    progress indicator.
*/

/*!
    \enum ProgressManager::ProgressFlag
    Additional flags that specify details in behavior. The
    default for a task is to not have any of these flags set.
    \value KeepOnFinish
        The progress indicator stays visible after the task has finished.
    \value ShowInApplicationIcon
        The progress indicator for this task is additionally
        shown in the application icon in the system's task bar or dock, on
        platforms that support that (at the moment Windows 7 and Mac OS X).
*/

/*!
    \fn ProgressManager::ProgressManager(QObject *parent = 0)
    \internal
*/

/*!
    \fn ProgressManager::~ProgressManager()
    \internal
*/

/*!
    \fn FutureProgress *ProgressManager::addTask(const QFuture<void> &future, const QString &title, const QString &type, ProgressFlags flags = 0)

    Shows a progress indicator for task given by the QFuture object \a future.
    The progress indicator shows the specified \a title along with the progress bar.
    The \a type of a task will specify a logical grouping with other
    running tasks. Via the \a flags parameter you can e.g. let the
    progress indicator stay visible after the task has finished.
    Returns an object that represents the created progress indicator,
    which can be used to further customize. The FutureProgress object's
    life is managed by the ProgressManager and is guaranteed to live only until
    the next event loop cycle, or until the next call of addTask.
    If you want to use the returned FutureProgress later than directly after calling this function,
    you will need to use protective functions (like wrapping the returned object in QPointer and
    checking for 0 whenever you use it).
*/

/*!
    \fn void ProgressManager::setApplicationLabel(const QString &text)

    Shows the given \a text in a platform dependent way in the application
    icon in the system's task bar or dock. This is used
    to show the number of build errors on Windows 7 and Mac OS X.
*/

/*!
    \fn void ProgressManager::cancelTasks(Id type)

    Schedules a cancel for all running tasks of the given \a type.
    Please note that the cancel functionality depends on the
    running task to actually check the \c QFutureInterface::isCanceled
    property.
*/

/*!
    \fn void ProgressManager::taskStarted(Id type)

    Sent whenever a task of a given \a type is started.
*/

/*!
    \fn void ProgressManager::allTasksFinished(Id type)

    Sent when all tasks of a \a type have finished.
*/

static void drawArrow(QStyle::PrimitiveElement element, QPainter* painter, const QStyleOption* option)
{
    // From windowsstyle but modified to enable AA
    if (option->rect.width() <= 1 || option->rect.height() <= 1) {
        return;
    }

    QRect r = option->rect;
    int size = qMin(r.height(), r.width());
    QPixmap pixmap;
    QString pixmapName;

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    pixmapName.asprintf(
#else
    pixmapName.sprintf(
#endif
                "arrow-%s-%d-%d-%d-%lld",
                       "$qt_ia",
                       uint(option->state), element,
                       size, option->palette.cacheKey());
    if (!QPixmapCache::find(pixmapName, &pixmap)) {
        int border = size / 5;
        int sqsize = 2 * (size / 2);
        QImage image(sqsize, sqsize, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter imagePainter(&image);
        imagePainter.setRenderHint(QPainter::Antialiasing, true);
        imagePainter.translate(0.5, 0.5);
        QPolygon a;
        switch (element) {
        case QStyle::PE_IndicatorArrowUp:
            a.setPoints(3, border, sqsize / 2,  sqsize / 2, border,  sqsize - border, sqsize / 2);
            break;
        case QStyle::PE_IndicatorArrowDown:
            a.setPoints(3, border, sqsize / 2,  sqsize / 2, sqsize - border,  sqsize - border, sqsize / 2);
            break;
        case QStyle::PE_IndicatorArrowRight:
            a.setPoints(3, sqsize - border, sqsize / 2,  sqsize / 2, border,  sqsize / 2, sqsize - border);
            break;
        case QStyle::PE_IndicatorArrowLeft:
            a.setPoints(3, border, sqsize / 2,  sqsize / 2, border,  sqsize / 2, sqsize - border);
            break;
        default:
            break;
        }

        int bsx = 0;
        int bsy = 0;

        if (option->state & QStyle::State_Sunken) {
            bsx = qApp->style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal);
            bsy = qApp->style()->pixelMetric(QStyle::PM_ButtonShiftVertical);
        }

        QRect bounds = a.boundingRect();
        int sx = sqsize / 2 - bounds.center().x() - 1;
        int sy = sqsize / 2 - bounds.center().y() - 1;
        imagePainter.translate(sx + bsx, sy + bsy);

        if (!(option->state & QStyle::State_Enabled)) {
            QColor foreGround(150, 150, 150, 150);
            imagePainter.setBrush(option->palette.mid().color());
            imagePainter.setPen(option->palette.mid().color());
        }
        else {
            QColor shadow(0, 0, 0, 100);
            imagePainter.translate(0, 1);
            imagePainter.setPen(shadow);
            imagePainter.setBrush(shadow);
            QColor foreGround(255, 255, 255, 210);
            imagePainter.drawPolygon(a);
            imagePainter.translate(0, -1);
            imagePainter.setPen(foreGround);
            imagePainter.setBrush(foreGround);
        }
        imagePainter.drawPolygon(a);
        imagePainter.end();
        pixmap = QPixmap::fromImage(image);
        QPixmapCache::insert(pixmapName, pixmap);
    }
    int xOffset = r.x() + (r.width() - size) / 2;
    int yOffset = r.y() + (r.height() - size) / 2;
    painter->drawPixmap(xOffset, yOffset, pixmap);
}

static ProgressManagerPrivate* m_instance = 0;

ProgressManagerPrivate::ProgressManagerPrivate(QWidget* parent)
    : m_applicationTask(0),
      m_currentStatusDetailsWidget(0),
      m_opacityEffect(new QGraphicsOpacityEffect(this)),
      m_progressViewPinned(true),
      m_hovered(false),
      m_allTasksCount(0),
      m_hasParent(false)
{
    m_instance = this;

    if (parent) {
        m_hasParent = true;
    }

    m_progressView = new ProgressView(parent);
    if (parent) {
        connect(parent, SIGNAL(destroyed(QObject*)), this, SLOT(deleteParentProgressView()));
    }
    m_progressView->setAttribute(Qt::WA_ShowWithoutActivating);
    // withDelay, so the statusBarWidget has the chance to get the enter event
    connect(m_progressView, SIGNAL(hoveredChanged(bool)), this, SLOT(updateVisibilityWithDelay()));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cancelAllRunningTasks()));
}

ProgressManagerPrivate::~ProgressManagerPrivate()
{
    stopFadeOfSummaryProgress();
    qDeleteAll(m_taskList);
    m_taskList.clear();

    // if there is parent it is responsible for deleting m_progressView that is parent of m_statusBarWidget
    if (!m_hasParent) {
        delete m_statusBarWidget;
    }

    cleanup();
    m_instance = 0;
}

void ProgressManagerPrivate::readSettings()
{
}

void ProgressManagerPrivate::init()
{
    readSettings();

    m_statusBarWidget = new QWidget;
    m_statusBarWidget->setAttribute(Qt::WA_ShowWithoutActivating);
    QHBoxLayout* layout = new QHBoxLayout(m_statusBarWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_statusBarWidget->setLayout(layout);
    m_summaryProgressWidget = new QWidget(m_statusBarWidget);
    m_summaryProgressWidget->setAttribute(Qt::WA_ShowWithoutActivating);
    m_summaryProgressWidget->setVisible(!m_progressViewPinned);
    m_summaryProgressWidget->setGraphicsEffect(m_opacityEffect);
    m_summaryProgressLayout = new QHBoxLayout(m_summaryProgressWidget);
    m_summaryProgressLayout->setContentsMargins(0, 0, 0, 0);
    m_summaryProgressLayout->setSpacing(0);
    m_summaryProgressWidget->setLayout(m_summaryProgressLayout);

    m_summaryProgressBar = new ProgressBar(m_summaryProgressWidget);
    m_summaryProgressBar->setAttribute(Qt::WA_ShowWithoutActivating);
    m_summaryProgressBar->setMinimumWidth(70);
    m_summaryProgressBar->setTitleVisible(true);
    m_summaryProgressBar->setSeparatorVisible(false);
    m_summaryProgressBar->setCancelEnabled(true);
    connect(m_summaryProgressBar, SIGNAL(clicked()), this, SIGNAL(allTasksCanceled()));
    connect(m_summaryProgressBar, SIGNAL(clicked()), this, SLOT(cancelAllRunningTasks()));
    connect(m_summaryProgressBar, SIGNAL(barClicked()), m_progressView, SLOT(toggleAllProgressView()));

    m_summaryProgressLayout->addWidget(m_summaryProgressBar);
    layout->addWidget(m_summaryProgressWidget);
    ToggleButton* toggleButton = new ToggleButton(m_statusBarWidget);
    layout->addWidget(toggleButton);

    m_statusBarWidget->installEventFilter(this);
    //m_summaryProgressWidget->installEventFilter(this);

    toggleButton->hide();
    m_progressView->addSummeryProgressWidget(m_statusBarWidget);

//    m_progressView->setVisible(m_progressViewPinned);
//    m_statusBarWidget->show();
//    m_summaryProgressWidget->show();

    initInternal();
}

void ProgressManagerPrivate::doCancelTasks(const QString &type)
{
    bool found = false;
    QMap<QFutureWatcher<void> *, QString>::iterator task = m_runningTasks.begin();
    while (task != m_runningTasks.end()) {
        if (task.value() != type) {
            ++task;
            continue;
        }
        found = true;
        disconnect(task.key(), SIGNAL(finished()), this, SLOT(taskFinished()));
        if (m_applicationTask == task.key()) {
            disconnectApplicationTask();
        }
        task.key()->cancel();
        delete task.key();
        task = m_runningTasks.erase(task);
    }
    if (found) {
        updateSummaryProgressBar();
        emit allTasksFinished(type);
    }
}

bool ProgressManagerPrivate::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_statusBarWidget) {
        if (event->type() == QEvent::Enter) {
            m_hovered = true;
            updateVisibility();
        }
        else if (event->type() == QEvent::Leave) {
            m_hovered = false;
            // give the progress view the chance to get the mouse enter event
            updateVisibilityWithDelay();
        }
        else if (event->type() == QEvent::MouseButtonPress && !m_taskList.isEmpty()) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && !me->modifiers()) {
                FutureProgress* progress = m_currentStatusDetailsProgress;
                if (!progress) {
                    progress = m_taskList.last();
                }
                // don't send signal directly from an event filter, event filters should
                // do as little a possible
                QTimer::singleShot(0, progress, SIGNAL(clicked()));
                event->accept();
                return true;
            }
        }
        else if (event->type() == QEvent::Paint) {
            QPainter p(m_statusBarWidget);
            const QRect fillRect = m_statusBarWidget->rect();

            p.fillRect(fillRect, FutureProgress::statusBarGradient(fillRect));
        }
    }
    return false;
}

void ProgressManagerPrivate::cancelAllRunningTasks()
{
    QMap<QFutureWatcher<void> *, QString>::const_iterator task = m_runningTasks.constBegin();
    while (task != m_runningTasks.constEnd()) {
        disconnect(task.key(), SIGNAL(finished()), this, SLOT(taskFinished()));
        if (m_applicationTask == task.key()) {
            disconnectApplicationTask();
        }
        task.key()->cancel();
        delete task.key();
        ++task;
    }
    m_runningTasks.clear();
    updateSummaryProgressBar();
}

FutureProgress* ProgressManagerPrivate::doAddTask(const QFuture<void> &future, const QString &title,
        const QString &type, ProgressFlags flags)
{
    // watch
    QFutureWatcher<void>* watcher = new QFutureWatcher<void>();

    // it's the first task or last tasks are finished/canceled
    if (m_runningTasks.isEmpty()) {
        m_allTasksCount = 0;
    }

    m_runningTasks.insert(watcher, type);
    ++m_allTasksCount;

    connect(watcher, SIGNAL(progressRangeChanged(int,int)), this, SLOT(updateSummaryProgressBar()));
    connect(watcher, SIGNAL(progressValueChanged(int)), this, SLOT(updateSummaryProgressBar()));
    connect(watcher, SIGNAL(finished()), this, SLOT(taskFinished()));
    watcher->setFuture(future);

    // handle application task
    if (flags & ShowInApplicationIcon) {
        if (m_applicationTask) {
            disconnectApplicationTask();
        }
        m_applicationTask = watcher;
        setApplicationProgressRange(future.progressMinimum(), future.progressMaximum());
        setApplicationProgressValue(future.progressValue());
        connect(m_applicationTask, SIGNAL(progressRangeChanged(int,int)),
                this, SLOT(setApplicationProgressRange(int,int)));
        connect(m_applicationTask, SIGNAL(progressValueChanged(int)),
                this, SLOT(setApplicationProgressValue(int)));
        setApplicationProgressVisible(true);
    }

    // create FutureProgress and manage task list
    removeOldTasks(type);

    FutureProgress* progress = new FutureProgress;
    progress->setTitle(title);
    progress->setFuture(future);

    if (m_progressView->progressCount() >= countOfVisibleProgresses() &&
            !progress->future().isFinished() &&
            !flags.testFlag(PrependInsteadAppend)) {
        m_queuedTaskList.append(FutureProgressPointer(progress));
    }
    else if (!progress->future().isFinished()) {
        m_progressView->addProgressWidget(progress);
    }

    m_taskList.append(progress);
    progress->setType(type);
    if (flags.testFlag(ProgressManager::KeepOnFinish)) {
        progress->setKeepOnFinish(FutureProgress::KeepOnFinishTillUserInteraction);
    }
    else {
        progress->setKeepOnFinish(FutureProgress::HideOnFinish);
    }
    connect(progress, SIGNAL(hasErrorChanged()), this, SLOT(updateSummaryProgressBar()));
    connect(progress, SIGNAL(removeMe()), this, SLOT(slotRemoveTask()));
    connect(progress, SIGNAL(fadeStarted()), this, SLOT(updateSummaryProgressBar()));
    connect(progress, SIGNAL(statusBarWidgetChanged()), this, SLOT(updateStatusDetailsWidget()));

    m_progressView->setVisible(m_progressViewPinned);
    m_statusBarWidget->show();
    m_summaryProgressWidget->show();

    updateStatusDetailsWidget();

    emit taskStarted(type);
    return progress;
}

ProgressView* ProgressManagerPrivate::progressView()
{
    return m_progressView;
}

void ProgressManagerPrivate::taskFinished()
{
    QObject* taskObject = sender();
    Q_ASSERT(taskObject);
    QFutureWatcher<void>* task = static_cast<QFutureWatcher<void> *>(taskObject);
    if (m_applicationTask == task) {
        disconnectApplicationTask();
    }
    const QString &type = m_runningTasks.value(task);
    m_runningTasks.remove(task);
    delete task;
    updateSummaryProgressBar();

    if (!m_runningTasks.key(type, 0)) {
        emit allTasksFinished(type);
    }
}

void ProgressManagerPrivate::disconnectApplicationTask()
{
    disconnect(m_applicationTask, SIGNAL(progressRangeChanged(int,int)),
               this, SLOT(setApplicationProgressRange(int,int)));
    disconnect(m_applicationTask, SIGNAL(progressValueChanged(int)),
               this, SLOT(setApplicationProgressValue(int)));
    setApplicationProgressVisible(false);
    m_applicationTask = 0;
}

void ProgressManagerPrivate::updateSummaryProgressBar()
{
    m_summaryProgressBar->setError(hasError());
    updateVisibility();
    if (m_runningTasks.isEmpty()) {
        m_summaryProgressBar->setFinished(true);
        if (m_taskList.isEmpty() || isLastFading()) {
            fadeAwaySummaryProgress();
        }
        return;
    }

    stopFadeOfSummaryProgress();

    m_summaryProgressBar->setFinished(false);
    QMapIterator<QFutureWatcher<void> *, QString> it(m_runningTasks);
    static const int TASK_RANGE = 100;
    int value = 0;
    while (it.hasNext()) {
        it.next();
        QFutureWatcher<void>* watcher = it.key();
        int min = watcher->progressMinimum();
        int range = watcher->progressMaximum() - min;
        if (range > 0) {
            value += TASK_RANGE * (watcher->progressValue() - min) / range;
        }
    }

    const int finishedCount = m_allTasksCount - m_runningTasks.size();
    value += finishedCount > 0 ? (TASK_RANGE * finishedCount) : 0;

    m_summaryProgressBar->setRange(0, TASK_RANGE * m_runningTasks.size());
    m_summaryProgressBar->setValue(value);
}

void ProgressManagerPrivate::fadeAwaySummaryProgress()
{
    stopFadeOfSummaryProgress();
    m_opacityAnimation = new QPropertyAnimation(m_opacityEffect, "opacity");
    m_opacityAnimation->setDuration(ProgressFadeAnimationDuration);
    m_opacityAnimation->setEndValue(0.);
    connect(m_opacityAnimation, SIGNAL(finished()), this, SLOT(summaryProgressFinishedFading()));
    m_opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ProgressManagerPrivate::stopFadeOfSummaryProgress()
{
    if (m_opacityAnimation) {
        m_opacityAnimation->stop();
        m_opacityEffect->setOpacity(1.);
        delete m_opacityAnimation;
    }
}

bool ProgressManagerPrivate::hasError() const
{
    foreach (FutureProgress* progress, m_taskList)
        if (progress->hasError()) {
            return true;
        }
    return false;
}

bool ProgressManagerPrivate::isLastFading() const
{
    if (m_taskList.isEmpty()) {
        return false;
    }
    foreach (FutureProgress* progress, m_taskList) {
        if (!progress->isFading()) { // we still have progress bars that are not fading
            return false;
        }
    }
    return true;
}

void ProgressManagerPrivate::slotRemoveTask()
{
    FutureProgress* progress = qobject_cast<FutureProgress*>(sender());
    Q_ASSERT(progress);
    const QString &type = progress->type();
    removeTask(progress);
    removeOldTasks(type, true);

    const int max = ::countOfVisibleProgresses();
    while (!m_queuedTaskList.isEmpty() && m_progressView->progressCount() < max) {
        FutureProgressPointer progressPointer = m_queuedTaskList.takeFirst();
        progress = progressPointer.data();

        if (progressPointer && progress && !progress->future().isFinished()) {
            m_progressView->addProgressWidget(progress);
        }
    }
}

void ProgressManagerPrivate::deleteParentProgressView()
{
    if (m_progressView) {
        m_progressView.data()->setParent(0, m_progressView.data()->windowFlags());
    }
}

void ProgressManagerPrivate::removeOldTasks(const QString &type, bool keepOne)
{
    bool firstFound = !keepOne; // start with false if we want to keep one
    QList<FutureProgress*>::iterator i = m_taskList.end();
    while (i != m_taskList.begin()) {
        --i;
        if ((*i)->type() == type) {
            if (firstFound && ((*i)->future().isFinished() || (*i)->future().isCanceled())) {
                deleteTask(*i);
                i = m_taskList.erase(i);
            }
            firstFound = true;
        }
    }
    updateSummaryProgressBar();
    updateStatusDetailsWidget();
}

void ProgressManagerPrivate::removeOneOldTask()
{
    if (m_taskList.isEmpty()) {
        return;
    }
    // look for oldest ended process
    for (QList<FutureProgress*>::iterator i = m_taskList.begin(); i != m_taskList.end(); ++i) {
        if ((*i)->future().isFinished()) {
            deleteTask(*i);
            i = m_taskList.erase(i);
            return;
        }
    }
    // no ended process, look for a task type with multiple running tasks and remove the oldest one
    for (QList<FutureProgress*>::iterator i = m_taskList.begin(); i != m_taskList.end(); ++i) {
        const QString &type = (*i)->type();

        int taskCount = 0;
        foreach (FutureProgress* p, m_taskList)
            if (p->type() == type) {
                ++taskCount;
            }

        if (taskCount > 1) { // don't care for optimizations it's only a handful of entries
            deleteTask(*i);
            i = m_taskList.erase(i);
            return;
        }
    }

    // no ended process, no type with multiple processes, just remove the oldest task
    FutureProgress* task = m_taskList.takeFirst();
    deleteTask(task);
    updateSummaryProgressBar();
    updateStatusDetailsWidget();
}

void ProgressManagerPrivate::removeTask(FutureProgress* task)
{
    if (m_taskList.removeAll(task) == 0) {
        m_queuedTaskList.removeAll(task);
    }

    deleteTask(task);

    updateSummaryProgressBar();
    updateStatusDetailsWidget();
}

void ProgressManagerPrivate::deleteTask(FutureProgress* progress)
{
    m_progressView->removeProgressWidget(progress);
    progress->hide();
    progress->deleteLater();
}

void ProgressManagerPrivate::updateVisibility()
{
    //m_progressView->setVisible(m_progressViewPinned || m_hovered || m_progressView->isHovered());
    m_summaryProgressWidget->setVisible((!m_runningTasks.isEmpty() || !m_taskList.isEmpty())
                                        && m_progressViewPinned && m_runningTasks.size() > 1);

    m_summaryProgressBar->setTitle(tr("All Tasks (%1):").arg(m_runningTasks.size()));
}

void ProgressManagerPrivate::updateVisibilityWithDelay()
{
    QTimer::singleShot(150, this, SLOT(updateVisibility()));
}

void ProgressManagerPrivate::updateStatusDetailsWidget()
{
    QWidget* candidateWidget = 0;
    // get newest progress with a status bar widget
    QList<FutureProgress*>::iterator i = m_taskList.end();
    while (i != m_taskList.begin()) {
        --i;
        candidateWidget = (*i)->statusBarWidget();
        if (candidateWidget) {
            m_currentStatusDetailsProgress = *i;
            break;
        }
    }

    if (candidateWidget == m_currentStatusDetailsWidget) {
        return;
    }

    if (m_currentStatusDetailsWidget) {
        m_currentStatusDetailsWidget->hide();
        m_summaryProgressLayout->removeWidget(m_currentStatusDetailsWidget);
    }

    if (candidateWidget) {
        m_summaryProgressLayout->insertWidget(0, candidateWidget);
        candidateWidget->show();
    }

    m_currentStatusDetailsWidget = candidateWidget;
}

void ProgressManagerPrivate::summaryProgressFinishedFading()
{
    m_summaryProgressWidget->setVisible(false);
    m_opacityEffect->setOpacity(1.);
}

void ProgressManagerPrivate::progressDetailsToggled(bool checked)
{
    m_progressViewPinned = checked;
    updateVisibility();
}

ToggleButton::ToggleButton(QWidget* parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

QSize ToggleButton::sizeHint() const
{
    return QSize(13, 12); // Uneven width, because the arrow's width is also uneven.
}

void ToggleButton::paintEvent(QPaintEvent* event)
{
    QToolButton::paintEvent(event);
    QPainter p(this);
    QStyleOption arrowOpt;
    arrowOpt.initFrom(this);
    arrowOpt.rect.adjust(2, 0, -1, -2);

    drawArrow(QStyle::PE_IndicatorArrowUp, &p, &arrowOpt);
}


ProgressManager::ProgressManager()
{
}

ProgressManager::~ProgressManager()
{
}

ProgressManager* ProgressManager::instance()
{
    return m_instance;
}

FutureProgress* ProgressManager::addTask(const QFuture<void> &future, const QString &title, const QString &type, ProgressFlags flags)
{
    return m_instance->doAddTask(future, title, type, flags);
}

/*!
    Shows a progress indicator for task given by the QFuture given by
    the QFutureInterface \a futureInterface.
    The progress indicator shows the specified \a title along with the progress bar.
    The progress indicator will increase monotonically with time, at \a expectedSeconds
    it will reach about 80%, and continue to increase with a decreasingly slower rate.

    \sa addTask
*/

FutureProgress* ProgressManager::addTimedTask(const QFutureInterface<void> &futureInterface, const QString &title,
        const QString &type, int expectedSeconds, ProgressFlags flags)
{
    QFutureInterface<void> dummy(futureInterface); // Need mutable to access .future()
    FutureProgress* fp = m_instance->doAddTask(dummy.future(), title, type, flags);
    (void) new ProgressTimer(fp, futureInterface, expectedSeconds);

    return fp;
}

void ProgressManager::setApplicationLabel(const QString &text)
{
    m_instance->doSetApplicationLabel(text);
}

void ProgressManager::cancelTasks(const QString &type)
{
    if (m_instance) {
        m_instance->doCancelTasks(type);
    }
}


ProgressTimer::ProgressTimer(QObject* parent,
                             const QFutureInterface<void> &futureInterface,
                             int expectedSeconds)
    : QTimer(parent),
      m_futureInterface(futureInterface),
      m_expectedTime(expectedSeconds),
      m_currentTime(0)
{
    m_futureWatcher.setFuture(m_futureInterface.future());

    m_futureInterface.setProgressRange(0, 100);
    m_futureInterface.setProgressValue(0);

    setInterval(1000); // 1 second
    connect(this, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    connect(&m_futureWatcher, SIGNAL(started()), this, SLOT(start()));
}

void ProgressTimer::handleTimeout()
{
    ++m_currentTime;

    // This maps expectation to atan(1) to Pi/4 ~= 0.78, i.e. snaps
    // from 78% to 100% when expectations are met at the time the
    // future finishes. That's not bad for a random choice.
    const double mapped = atan2(double(m_currentTime), double(m_expectedTime));
    const double progress = 100 * 2 * mapped / 3.14;
    m_futureInterface.setProgressValue(int(progress));
}

