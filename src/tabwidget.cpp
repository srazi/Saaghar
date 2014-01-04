/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2014 by S. Razi Alavizadeh                          *
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

#include "tabwidget.h"

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_tabBar(new TabBar(this))
{
    setTabBar(m_tabBar);
}

TabBar *TabWidget::getTabBar()
{
    return m_tabBar;
}


TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{
}

QSize TabBar::tabSizeHint(int index) const
{
    if (!isVisible()) {
        return QSize(-1, -1);
    }

    const int maxWidth = 250;
    const int minWidth = 100;
    const int minHeight = 27;

    QSize size = QTabBar::tabSizeHint(index);
    size.setHeight(qMax(size.height(), minHeight));

    int availableWidth = width();
    int tabCount = count();
    if (tabCount > 0) {
        int boundedWidthForTab = qBound(minWidth, availableWidth / tabCount, maxWidth);
        if (index == currentIndex()) {
            int activeTabWidth = qBound(boundedWidthForTab, availableWidth - (tabCount -1) * boundedWidthForTab, maxWidth);
            size.setWidth(activeTabWidth);
        }
        else {
            size.setWidth(boundedWidthForTab);
        }
    }

    return size;
}
