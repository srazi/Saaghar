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

#ifndef PROGRESSMANAGER_H
#define PROGRESSMANAGER_H

#include <QObject>
#include <QFuture>

class FutureProgress;
class ProgressManagerPrivate;

class ProgressManager : public QObject
{
    Q_OBJECT
public:
    enum ProgressFlag {
        KeepOnFinish = 0x01,
        ShowInApplicationIcon = 0x02,
        PrependInsteadAppend = 0x03
    };
    Q_DECLARE_FLAGS(ProgressFlags, ProgressFlag)

    enum Position {
        AppBottomRight,
        AppBottomLeft,
        DesktopBottomRight,
        DesktopTopRight,
        Disabled = -1
    };

    static ProgressManager* instance();

    static FutureProgress* addTask(const QFuture<void> &future, const QString &title,
                                   const QString &type, ProgressFlags flags = ProgressFlags());
    static FutureProgress* addTimedTask(const QFutureInterface<void> &fi, const QString &title,
                                        const QString &type, int expectedSeconds, ProgressFlags flags = ProgressFlags());
    static void setApplicationLabel(const QString &text);

public slots:
    static void cancelTasks(const QString &type);

signals:
    void taskStarted(const QString &type);
    void allTasksFinished(const QString &type);
    void allTasksCanceled();

private:
    ProgressManager();
    ~ProgressManager();

    friend class ProgressManagerPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ProgressManager::ProgressFlags)

#endif //PROGRESSMANAGER_H
