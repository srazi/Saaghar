/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
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

#include <QtGui>

#include "commands.h"

#include "SaagharWidget.h"

NavigateToPage::NavigateToPage(SaagharWidget *saagharWidget, const QString &type, int pageId, QUndoCommand *parent)
	: QUndoCommand(parent), m_saagharWidget(saagharWidget)
{
	if (m_saagharWidget)
	{
		previousType = m_saagharWidget->identifier().at(0);
		previousId = m_saagharWidget->identifier().at(1).toInt();
	
		if (type != "PoemID" || type != "CatID" || pageId <0)
			m_saagharWidget->processClickedItem(type, pageId, false);
		else
			m_saagharWidget->processClickedItem(type, pageId, true);
	
		newType = m_saagharWidget->identifier().at(0);
		newId = m_saagharWidget->identifier().at(1).toInt();
	
		setText(QObject::tr("Navigate To %1").arg(m_saagharWidget->currentCaption));
	}
	else
	{
		previousType = "CatID";
		previousId = 0;
		newType = "CatID";
		newId = 0;
	}
}

void NavigateToPage::undo()
{
	if (!m_saagharWidget) return;

	if (previousType != "PoemID" || previousType != "CatID" || previousId <0)
		m_saagharWidget->processClickedItem(previousType, previousId, false);
	else
		m_saagharWidget->processClickedItem(previousType, previousId, true);

	setText(QObject::tr("Navigate To %1").arg(m_saagharWidget->currentCaption));
}

void NavigateToPage::redo()
{
	if (!m_saagharWidget) return;

	if (newType != "PoemID" || newType != "CatID" || newId <0)
		m_saagharWidget->processClickedItem(newType, newId, false);
	else
		m_saagharWidget->processClickedItem(newType, newId, true);

	setText(QObject::tr("Navigate To %1").arg(m_saagharWidget->currentCaption));
}
