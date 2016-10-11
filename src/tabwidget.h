/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2014-2016 by S. Razi Alavizadeh                          *
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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabBar>
#include <QTabWidget>

class MacToolButton;

class TabBar : public QTabBar
{
    Q_OBJECT

public:
    TabBar(QWidget* parent = 0);
    MacToolButton* addTabButton();

private:
    QSize tabSizeHint(int index) const;
    void moveAddTabButton(int posX);

    MacToolButton* m_addTabButton;
};

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    TabWidget(QWidget* parent = 0);
    TabBar* getTabBar();

private:
    TabBar* m_tabBar;
};
#endif // TABWIDGET_H
