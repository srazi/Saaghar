/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2016 by S. Razi Alavizadeh                          *
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

#include "commands.h"
#include "saagharwidget.h"
#include "tools.h"

#include <QTimer>

NavigateToPage::NavigateToPage(SaagharWidget* saagharWidget, const QString &type, int pageId, QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_saagharWidget(saagharWidget)
{
    if (m_saagharWidget) {
        m_previousType = m_saagharWidget->identifier().at(0);
        m_previousId = m_saagharWidget->identifier().at(1).toInt();

        m_previousText = m_saagharWidget->currentLocationList.join("-");
        if (!m_saagharWidget->currentPoemTitle.isEmpty()) {
            m_previousText += "-" + Tools::snippedText(m_saagharWidget->currentPoemTitle, QString(), 0, 10, true, Qt::ElideMiddle);
        }
        if (m_previousText.isEmpty()) {
            m_previousText = SaagharWidget::rootTitle();
        }

        m_previousRow = m_saagharWidget->currentVerticalPosition();
        m_newRow = 0;

        m_newType = type;
        m_newId = pageId;
    }
    else {
        m_previousType = "CatID";
        m_previousId = 0;
        m_newType = "CatID";
        m_newId = 0;
        m_previousRow = 0;
        m_newRow = 0;
    }
}

void NavigateToPage::undo()
{
    if (!m_saagharWidget) {
        return;
    }

    m_newRow = m_saagharWidget->currentVerticalPosition();

    if ((m_previousType != "PoemID" && m_previousType != "CatID") || m_previousId < 0) {
        m_saagharWidget->navigateToPage(m_previousType, m_previousId, false);
        m_saagharWidget->setMVPosition(m_previousRow);
        QTimer::singleShot(0, m_saagharWidget, SLOT(setFromMVPosition()));
    }
    else {
        m_saagharWidget->navigateToPage(m_previousType, m_previousId, true);
        m_saagharWidget->setMVPosition(m_previousRow);
        QTimer::singleShot(0, m_saagharWidget, SLOT(setFromMVPosition()));
    }

    QString text = "";
    if (m_saagharWidget->undoStack->index() > 1) {
        const NavigateToPage* tmp = static_cast<const NavigateToPage*>(m_saagharWidget->undoStack->command(m_saagharWidget->undoStack->index() - 1));

        if (tmp) {
            text = tmp->m_previousText;
        }
    }
}

void NavigateToPage::redo()
{
    if (!m_saagharWidget) {
        return;
    }

    m_previousRow = m_saagharWidget->currentVerticalPosition();

    if ((m_newType != "PoemID" && m_newType != "CatID") || m_newId < 0) {
        m_saagharWidget->navigateToPage(m_newType, m_newId, false);
        m_saagharWidget->setMVPosition(m_newRow);
        QTimer::singleShot(0, m_saagharWidget, SLOT(setFromMVPosition()));
    }
    else {
        m_saagharWidget->navigateToPage(m_newType, m_newId, true);
        m_saagharWidget->setMVPosition(m_newRow);
        QTimer::singleShot(0, m_saagharWidget, SLOT(setFromMVPosition()));
    }

    m_newText = "";
    if (!m_saagharWidget->currentLocationList.isEmpty()) {
        m_newText = m_saagharWidget->currentLocationList.at(m_saagharWidget->currentLocationList.size() - 1);    //join("-");
    }
    if (!m_saagharWidget->currentPoemTitle.isEmpty()) {
        m_newText = Tools::snippedText(m_saagharWidget->currentPoemTitle, QString(), 0, 10, true, Qt::ElideMiddle);
    }

    if (m_newText.isEmpty()) {
        m_newText = SaagharWidget::rootTitle();
    }
    setText(QObject::tr("Navigate To %1").arg(m_newText));
}
