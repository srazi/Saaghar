/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2015 by S. Razi Alavizadeh                               *
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

#include "progressview.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QTimer>
#include <QVBoxLayout>

ProgressView::ProgressView(QWidget* parent)
    : QWidget(parent),
      m_referenceWidget(0),
      m_hovered(false),
      m_lastVisibleState(true),
      m_forceHidden(false),
      m_repositioning(false)
{
    m_progressWidget = new QWidget(this);
    m_layout = new QVBoxLayout;
    m_progressWidget->setLayout(m_layout);
    m_topLayout = new QVBoxLayout;
    setLayout(m_topLayout);
    m_topLayout->setContentsMargins(0, 0, 0, 0);
    m_topLayout->setSpacing(0);
    m_topLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_topLayout->addWidget(m_progressWidget);
    m_layout->setContentsMargins(0, 0, 0, 1);
    m_layout->setSpacing(0);
    m_layout->setSizeConstraint(QLayout::SetFixedSize);
    setWindowTitle(tr("Processes"));
    setWindowFlags(Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    setWindowOpacity(0.75);
}

ProgressView::~ProgressView()
{
}

void ProgressView::addProgressWidget(QWidget* widget)
{
    setUpdatesEnabled(false);
    m_layout->insertWidget(0, widget);
    setUpdatesEnabled(true);
}

void ProgressView::removeProgressWidget(QWidget* widget)
{
    setUpdatesEnabled(false);
    m_layout->removeWidget(widget);
    setUpdatesEnabled(true);
}

void ProgressView::setProgressWidgetVisible(bool visible)
{
    m_lastVisibleState = visible;
    m_progressWidget->setVisible(visible);
}

void ProgressView::setPosition(const ProgressManager::Position &position)
{
    if (position != m_position) {
        m_position = position;
        doReposition();
    }
}

int ProgressView::progressCount() const
{
    return m_layout->count();
}

void ProgressView::addSummeryProgressWidget(QWidget* widget)
{
    m_topLayout->insertWidget(1, widget);
}

void ProgressView::removeSummeryProgressWidget(QWidget* widget)
{
    m_topLayout->removeWidget(widget);
}

bool ProgressView::isHovered() const
{
    return m_hovered;
}

void ProgressView::setReferenceWidget(QWidget* widget)
{
    if (m_referenceWidget) {
        m_referenceWidget->removeEventFilter(this);
    }
    m_referenceWidget = widget;

    if (m_referenceWidget) {
        m_referenceWidget->installEventFilter(this);
    }
    doReposition();
}

void ProgressView::setVisible(bool visible)
{
    if (m_forceHidden) {
        m_progressWidget->setVisible(false);
        QWidget::setVisible(false);
        return;
    }

    m_progressWidget->setVisible(visible);
    QWidget::setVisible(visible);
}

bool ProgressView::event(QEvent* event)
{
    if (event->type() == QEvent::ParentAboutToChange && parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }
    else if (event->type() == QEvent::ParentChange && parentWidget()) {
        parentWidget()->installEventFilter(this);
    }
    else if (event->type() == QEvent::Resize) {
        doReposition();
    }
    else if (event->type() == QEvent::Enter) {
        m_hovered = true;
        setWindowOpacity(0.9);
        emit hoveredChanged(m_hovered);
    }
    else if (event->type() == QEvent::Leave) {
        m_hovered = false;
        setWindowOpacity(0.75);
        emit hoveredChanged(m_hovered);
    }
    return QWidget::event(event);
}

bool ProgressView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_referenceWidget && m_referenceWidget) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            doReposition();
        }
        else if (event->type() == QEvent::WindowStateChange) {
            if (m_referenceWidget->isMinimized()) {
                m_forceHidden = true;
                hide();
            }
            else {
                m_forceHidden = false;
                show();
            }
        }
    }

    return false;
}

void ProgressView::toggleAllProgressView()
{
    setProgressWidgetVisible(!m_progressWidget->isVisible());
}

void ProgressView::reposition()
{
    m_repositioning = false;

    if (!m_referenceWidget) {
        return;
    }

    static int frameWidth = -1;
    static int titleBarHeight;

    if (frameWidth == -1 && m_referenceWidget) {
        frameWidth = qMax(0, (m_referenceWidget->frameGeometry().width() - m_referenceWidget->width()) / 2);
        titleBarHeight = qMax(0, m_referenceWidget->frameGeometry().height() - m_referenceWidget->height() - frameWidth);
    }

    const QRect screenRect = qApp->desktop()->availableGeometry(this);
    QRect geoRect = this->rect();
    // for now we always assume parent is 0
    switch (m_position) {
    case ProgressManager::AppBottomLeft: {
        if (!m_referenceWidget) {
            return;
        }
        int w = geoRect.width();
        int h = geoRect.height();
        geoRect.setRect(m_referenceWidget->x() + frameWidth,
                        m_referenceWidget->y() + m_referenceWidget->frameGeometry().height() - h - frameWidth,
                        w, h);
//        if (h > m_referenceWidget->height()) {
//            m_progressWidget->hide();
//        }
//        else if (m_lastVisibleState && h < m_progressWidget->sizeHint().height() + height() - 10) {
//            m_progressWidget->show();
//        }
    }
    break;
    case ProgressManager::AppBottomRight: {
        if (!m_referenceWidget) {
            return;
        }
        int w = geoRect.width();
        int h = geoRect.height();
        geoRect.setRect(m_referenceWidget->x() + m_referenceWidget->frameGeometry().width() - w - frameWidth,
                        m_referenceWidget->y() + m_referenceWidget->frameGeometry().height() - h - frameWidth,
                        w, h);
//        if (h > m_referenceWidget->height()) {
//            m_progressWidget->hide();
//        }
//        else if (m_lastVisibleState && h < m_progressWidget->sizeHint().height() + height() - 10) {
//            m_progressWidget->show();
//        }
    }
    break;
    case ProgressManager::DesktopBottomRight:
        geoRect.setRect(screenRect.x() + screenRect.width() - geoRect.width(),
                        screenRect.height() - geoRect.height(),
                        qMin(geoRect.width(), m_referenceWidget->width()),
                        geoRect.height());
        break;
    case ProgressManager::DesktopTopRight:
        geoRect.setRect(screenRect.x() + screenRect.width() - geoRect.width(),
                        screenRect.y(),
                        geoRect.width(),
                        geoRect.height());
        break;
    default:
        return;
    }

    setGeometry(geoRect);
}

void ProgressView::doReposition()
{
    if (!m_repositioning) {
        m_repositioning = true;
        reposition();
    }
}
