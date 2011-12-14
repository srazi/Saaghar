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

		previousText = m_saagharWidget->currentLocationList.join("-");
		if (!m_saagharWidget->currentPoemTitle.isEmpty())
			previousText+="-"+m_saagharWidget->currentPoemTitle;
		if (previousText.isEmpty())
			previousText = QObject::tr("Home");

		if ( (type != "PoemID" && type != "CatID") || pageId <0)
			m_saagharWidget->navigateToPage(type, pageId, false);
		else
			m_saagharWidget->navigateToPage(type, pageId, true);
	
		newType = m_saagharWidget->identifier().at(0);
		newId = m_saagharWidget->identifier().at(1).toInt();

		newText = "";
		if (!m_saagharWidget->currentLocationList.isEmpty())
			newText = m_saagharWidget->currentLocationList.at(m_saagharWidget->currentLocationList.size()-1); //join("-");
		if (!m_saagharWidget->currentPoemTitle.isEmpty())
			newText = m_saagharWidget->currentPoemTitle;
			//newText+="-"+m_saagharWidget->currentPoemTitle;
		if (newText.isEmpty())
			newText = QObject::tr("Home");
		setText(QObject::tr("Navigate To %1").arg(newText));
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

	if ( (previousType != "PoemID" && previousType != "CatID") || previousId <0)
		m_saagharWidget->navigateToPage(previousType, previousId, false);
	else
		m_saagharWidget->navigateToPage(previousType, previousId, true);

//	QString text = m_saagharWidget->currentLocationList.join("-");
//	if (!m_saagharWidget->currentPoemTitle.isEmpty())
//		text+="-"+m_saagharWidget->currentPoemTitle;
//	if (text.isEmpty())
//		text = QObject::tr("Home");

	QString text = "";
	if (m_saagharWidget->undoStack->index()>1)
	{
		const NavigateToPage *tmp = static_cast<const NavigateToPage *>(m_saagharWidget->undoStack->command(m_saagharWidget->undoStack->index()-1) );//qobject_cast<const NavigateToPage *>(m_saagharWidget->undoStack->command(m_saagharWidget->undoStack->index()-1));
		qDebug() << "NavigateToPage="<<tmp;
		if (tmp)
			text = tmp->previousText;//m_saagharWidget->undoStack->command(m_saagharWidget->undoStack->index()-1);
	}
	//text = m_saagharWidget->undoStack->text(m_saagharWidget->undoStack->index());
	//setText(QObject::tr("Navigate To %1").arg(text));

	//setText(QObject::tr("Navigate To %1").arg(previousText));

	qDebug() <<"undo-previous="<< previousType << previousId;
	qDebug() <<"undo-NEW="<< newType << newId;
	//setText(QObject::tr("Navigate To %1").arg(m_saagharWidget->currentCaption));
}

void NavigateToPage::redo()
{
	if (!m_saagharWidget) return;

	if ( (newType != "PoemID" && newType != "CatID") || newId <0)
		m_saagharWidget->navigateToPage(newType, newId, false);
	else
		m_saagharWidget->navigateToPage(newType, newId, true);

//	QString text = m_saagharWidget->currentLocationList.join("-");
//	if (!m_saagharWidget->currentPoemTitle.isEmpty())
//		text+="-"+m_saagharWidget->currentPoemTitle;
//	if (text.isEmpty())
//		text = QObject::tr("Home");

	//QString text = "";
	//if (m_saagharWidget->undoStack->index()>1)
	//text = m_saagharWidget->undoStack->text(m_saagharWidget->undoStack->index());
	//setText(QObject::tr("Navigate To %1").arg(text));

	qDebug() <<"redo-previous="<< previousType << previousId;
	qDebug() <<"redo-NEW="<< newType << newId;
	//setText(QObject::tr("Navigate To %1").arg(m_saagharWidget->currentCaption));
}
