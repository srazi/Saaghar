/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2013-2014 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 *  This file was taken from QupZilla                                      *
 *  Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>                *
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
#ifndef MACTOOLBUTTON_H
#define MACTOOLBUTTON_H

#include <QObject>

#ifdef Q_OS_MAC
#include <QPushButton>

class MacToolButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)

public:
    explicit MacToolButton(QWidget* parent = 0);

    void setIconSize(const QSize &size);

    void setAutoRaise(bool enable);
    bool autoRaise() const;

private:
    bool m_autoRise;
    QSize m_buttonFixedSize;
};
#else
#include <QToolButton>

class MacToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit MacToolButton(QWidget* parent = 0);
};
#endif
#endif // MACTOOLBUTTON_H
