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

#include "tabwidget.h"
#include "mactoolbutton.h"

TabWidget::TabWidget(QWidget* parent)
    : QTabWidget(parent)
    , m_tabBar(new TabBar(this))
{
    setTabBar(m_tabBar);
    setElideMode(Qt::ElideMiddle);
}

TabBar* TabWidget::getTabBar()
{
    return m_tabBar;
}


TabBar::TabBar(QWidget* parent)
    : QTabBar(parent)
    , m_addTabButton(new MacToolButton(this))
{
    // on Mac its default state is off
    setUsesScrollButtons(true);
    m_addTabButton->setAutoRaise(true);
    setDrawBase(true);
}

MacToolButton* TabBar::addTabButton()
{
    return m_addTabButton;
}

QSize TabBar::tabSizeHint(int index) const
{
    if (!isVisible()) {
        return QSize(-1, -1);
    }

    const int maxWidth = 250;
    const int minWidth = 100;

    TabBar* tabBar = const_cast<TabBar*>(this);

    static int height = -1;

    if (height == -1) {
        height = QTabBar::tabSizeHint(index).height();
#ifndef Q_OS_MAC
        height = qMax(height, 33);
#endif
    }

    int thisTabWidth = minWidth;

    int activeTabWidthBiggerThanOther = 75;
    int availableWidth = width() - (m_addTabButton->isVisible() ? m_addTabButton->width() : 0) - activeTabWidthBiggerThanOther;
    int tabCount = count();
    int boundedWidthForTab = maxWidth;
    int activeTabWidth = boundedWidthForTab;

    if (tabCount > 0) {
        boundedWidthForTab = qBound(minWidth, availableWidth / tabCount, maxWidth);
        if (index == currentIndex()) {
            activeTabWidth = activeTabWidthBiggerThanOther + qBound(boundedWidthForTab, availableWidth - (tabCount - 1) * boundedWidthForTab, maxWidth);
            thisTabWidth = activeTabWidth;
        }
        else {
            thisTabWidth = boundedWidthForTab;
        }
    }

    if (index == currentIndex()) {
        if ((tabCount * minWidth + activeTabWidthBiggerThanOther) > availableWidth) {
            m_addTabButton->hide();
        }
        else {
            int addTabButtonX = (tabCount - 1) * boundedWidthForTab + activeTabWidth;

            if (isRightToLeft()) {
                addTabButtonX = width() - addTabButtonX;
            }

            tabBar->moveAddTabButton(addTabButtonX);
            m_addTabButton->show();
        }
    }
    return QSize(thisTabWidth, height);
}

void TabBar::moveAddTabButton(int posX)
{
    int posY = (height() - m_addTabButton->height()) / 2;

    if (isRightToLeft()) {
        posX = qMax(posX - m_addTabButton->width(), 0);
    }
    else {
        posX = qMin(posX, width() - m_addTabButton->width());
    }
    m_addTabButton->move(posX, posY);
}
