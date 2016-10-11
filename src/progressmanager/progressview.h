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

#ifndef PROGRESSVIEW_H
#define PROGRESSVIEW_H

#include "progressmanager.h"

#include <QWidget>


QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

class ProgressView : public QWidget
{
    Q_OBJECT

public:
    ProgressView(QWidget* parent = 0);
    ~ProgressView();

    void addProgressWidget(QWidget* widget);
    void removeProgressWidget(QWidget* widget);

    void setProgressWidgetVisible(bool visible);
    void setPosition(const ProgressManager::Position &position);

    int progressCount() const;

    void addSummeryProgressWidget(QWidget* widget);
    void removeSummeryProgressWidget(QWidget* widget);

    bool isHovered() const;

    void setReferenceWidget(QWidget* widget);

public slots:
    void setVisible(bool visible);

protected:
    bool event(QEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void toggleAllProgressView();
    void reposition();

signals:
    void hoveredChanged(bool hovered);

private:
    void doReposition();

    QVBoxLayout* m_layout;
    QVBoxLayout* m_topLayout;
    QWidget* m_referenceWidget;
    QWidget* m_progressWidget;
    bool m_hovered;
    bool m_lastVisibleState;
    bool m_forceHidden;
    bool m_repositioning;

    ProgressManager::Position m_position;
};

#endif // PROGRESSVIEW_H
