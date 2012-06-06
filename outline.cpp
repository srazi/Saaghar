/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2012 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 *                                                                         *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).       *
 * All rights reserved.                                                    *
 * Contact: Nokia Corporation (qt-info@nokia.com)                          *
 *                                                                         *
 * This file is part of the examples of the Qt Toolkit.                    *
 *                                                                         *
 * $QT_BEGIN_LICENSE:BSD$                                                  *
 * You may use this file under the terms of the BSD license as follows:    *
 *                                                                         *
 * "Redistribution and use in source and binary forms, with or without     *
 * modification, are permitted provided that the following conditions are  *
 * met:                                                                    *
 *   * Redistributions of source code must retain the above copyright      *
 *     notice, this list of conditions and the following disclaimer.       *
 *   * Redistributions in binary form must reproduce the above copyright   *
 *     notice, this list of conditions and the following disclaimer in     *
 *     the documentation and/or other materials provided with the          *
 *     distribution.                                                       *
 *   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor  *
 *     the names of its contributors may be used to endorse or promote     *
 *     products derived from this software without specific prior written  *
 *     permission.                                                         *
 *                                                                         *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   *
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."   *
 * $QT_END_LICENSE$                                                        *
 *                                                                         *
 ***************************************************************************/

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QHeaderView>
#include <QApplication>
#include <QSearchLineEdit>

#include "outline.h"
#include "QGanjoorDbBrowser.h"

OutLineTree::OutLineTree(QWidget *parent)
	: QWidget(parent)
{
	pressedMouseButton==Qt::LeftButton;

	outlineWidget = new QTreeWidget(parent);
	outlineWidget->setObjectName("outlineTreeWidget");
	outlineWidget->setLayoutDirection(Qt::RightToLeft);
	outlineWidget->header()->hide();
	outlineWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(outlineWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(createCustomContextMenu(QPoint)));

	QStringList labels;
	labels << tr("Title") << tr("Comments");

	setObjectName("OutLineWidget");
	//hide();

	//QWidget *bookmarkContainer = new QWidget(bookmarkManagerWidget);
	QVBoxLayout *mainLayout = new QVBoxLayout;
	QHBoxLayout *toolsLayout = new QHBoxLayout;

//	QString clearIconPath = currentIconThemePath()+"/clear-left.png";
//	if (layoutDirection() == Qt::RightToLeft)
//		clearIconPath = currentIconThemePath()+"/clear-right.png";

	QLabel *filterLabel = new QLabel;
	filterLabel->setObjectName(QString::fromUtf8("OutLineTreeFilterLabel"));
	filterLabel->setText(tr("Filter:"));
	QSearchLineEdit *outLineFilter = new QSearchLineEdit(this, ""/*clearIconPath*/, ""/*currentIconThemePath()+"/filter.png"*/);
	outLineFilter->setObjectName("outLineFilter");
#if QT_VERSION >= 0x040700
	outLineFilter->setPlaceholderText(tr("Filter"));
#else
	outLineFilter->setToolTip(tr("Filter"));
#endif
	connect(outLineFilter, SIGNAL(textChanged(const QString &)), this, SLOT(filterItems(const QString &)));
	connect(outLineFilter, SIGNAL(clearButtonPressed()), this, SLOT(filterItems()));

	QSpacerItem *filterHorizSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	toolsLayout->addWidget(filterLabel, 0, Qt::AlignRight|Qt::AlignCenter);
	toolsLayout->addWidget(outLineFilter);
	toolsLayout->addItem(filterHorizSpacer);

	mainLayout->addWidget(outlineWidget);
	mainLayout->addLayout(toolsLayout);
	setLayout(mainLayout);

	connect(outlineWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(doubleClicked(QTreeWidgetItem*,int)));
	connect(outlineWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(justClicked(QTreeWidgetItem*,int)));
	connect(outlineWidget, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(itemPressed(QTreeWidgetItem*,int)));
}

void OutLineTree::setItems(const QList<QTreeWidgetItem *> &items)
{
	outlineWidget->clear();
	outlineWidget->addTopLevelItems(items);
}

bool OutLineTree::filterItems(const QString &str, QTreeWidgetItem *parentItem)
{
//	QList<QTreeWidgetItem *> list = outlineWidget->findItems("", Qt::MatchContains|Qt::MatchWrap|Qt::MatchRecursive);
	
//		for (int i=0; i<list.size();++i)
//		{
//			if (list.at(i))
//			{
//				QString text = list.at(i)->text(0);
//				text = QGanjoorDbBrowser::cleanString(text);
//				if (!str.isEmpty() && !text.contains(str))
//					list.at(i)->setHidden(true);
//				else
//					list.at(i)->setHidden(false);
//			}
//		}

	int childrenSize;
	if (!parentItem)
		childrenSize = outlineWidget->topLevelItemCount();
	else
		childrenSize = parentItem->childCount();

	bool result = true;

	for (int i=0; i<childrenSize; ++i)
	{
		QTreeWidgetItem *ithChild;
		if (!parentItem)
			ithChild = outlineWidget->topLevelItem(i);
		else
			ithChild = parentItem->child(i);

		QString text = ithChild->text(0);
		text = QGanjoorDbBrowser::cleanString(text);
		QString cleanStr = QGanjoorDbBrowser::cleanString(str);
		if (!cleanStr.isEmpty() && !text.contains(cleanStr))
		{
			if (ithChild->childCount()>0)//if all its children are hidden it should be hidden, too.
			{
				bool tmp = filterItems(cleanStr, ithChild);
				if (!tmp)
				{
					ithChild->setExpanded(true);
					//recursivelyUnHide(ithChild);
				}

				ithChild->setHidden(tmp);
				result = result && tmp;
			}
			else
			{
				ithChild->setHidden(true);
				result = result && true;//return true when all children are hidden
			}
		}
		else
		{
			////if (ithChild->isHidden())
//			{
//				ithChild->setHidden(false);
//				int childCount = ithChild->childCount();
//				for (int j=0; j<childCount;++j)
//				{
//					QTreeWidgetItem *child = ithChild->child(j);
//					child->setHidden(false);
//				}
//			}
			recursivelyUnHide(ithChild);
			ithChild->setExpanded(false);
			result = result && false;//return false when at least one child is visible
		}
//		int childCount = ithChild->childCount();
//		for (int j=0; j<childCount;++j)
//		{
//			QTreeWidgetItem *child = ithRootChild->child(j);
//			QString text = child->text(0)+child->text(1);
//			text = QGanjoorDbBrowser::cleanString(text);
//			if (!str.isEmpty() && !text.contains(str))
//				child->setHidden(true);
//			else
//				child->setHidden(false);
//		}
	}
	return result;
}

void OutLineTree::recursivelyUnHide(QTreeWidgetItem *parentItem)
{
	int childrenSize;
	if (!parentItem)
		childrenSize = outlineWidget->topLevelItemCount();
	else
	{
		childrenSize = parentItem->childCount();
		parentItem->setHidden(false);
	}

	for (int i=0; i<childrenSize; ++i)
	{
		QTreeWidgetItem *ithChild;
		if (!parentItem)
			ithChild = outlineWidget->topLevelItem(i);
		else
			ithChild = parentItem->child(i);
		recursivelyUnHide(ithChild);
	}
}

void OutLineTree::doubleClicked(QTreeWidgetItem *item, int /*column*/)
{
	if (item)
	{
		int id = item->data(0, Qt::UserRole).toInt();
		emit openParentRequested(id);
	}
}

void OutLineTree::justClicked(QTreeWidgetItem *item, int /*column*/)
{
	if (pressedMouseButton==Qt::RightButton) return;

	if (item && item->childCount()>0)
	{
		item->setExpanded(!item->isExpanded());
	}
}

void OutLineTree::createCustomContextMenu(const QPoint &pos)
{
	QTreeWidgetItem *item = outlineWidget->itemAt(pos);
	if (!item)
		return;

	int itemID = item->data(0, Qt::UserRole).toInt();

	QMenu *contextMenu = new QMenu;
	contextMenu->addAction(tr("Open in New Tab"));
	contextMenu->addAction(tr("Open"));
	contextMenu->addSeparator();
//	contextMenu->addAction(tr("Random in New Tab"));
//	contextMenu->addAction(tr("Random"));
	contextMenu->addSeparator();
	if (item->childCount()>0)
	{
		if (item->isExpanded())
			contextMenu->addAction(tr("Collapse"));
		else
			contextMenu->addAction(tr("Expand"));
	}

	QAction *action = contextMenu->exec(QCursor::pos());
	if (!action)
		return;

	QString text = action->text();
	text.remove("&");

	if (text == tr("Open in New Tab"))
	{
		emit newParentRequested(itemID);
	}
	else if (text == tr("Open"))
	{
		emit openParentRequested(itemID);
	}
	else if (text == tr("Collapse") || text == tr("Expand"))
	{
		item->setExpanded(!item->isExpanded());
	}
		
//	else if (text == tr("Random in New Tab"))
//	{
////		QString tableText = tableToString(saagharWidget->tableViewWidget, "          "/*ten blank spaces-Mesra separator*/, "\n"/*Beyt separator*/, 0, 1, saagharWidget->tableViewWidget->rowCount(), saagharWidget->tableViewWidget->columnCount());
////		QApplication::clipboard()->setText(tableText);
//	}
//	else if (text == tr("Random"))
//	{
////		insertNewTab();
	//	}
}

void OutLineTree::itemPressed(QTreeWidgetItem */*item*/, int /*column*/)
{
	pressedMouseButton = QApplication::mouseButtons();
}

void OutLineTree::setTreeFont(const QFont &font)
{
	if (outlineWidget)
		outlineWidget->setFont(font);
}

void OutLineTree::setTreeColor(const QColor &color)
{
	if (outlineWidget && color.isValid())
	{
		QPalette p(outlineWidget->palette());
		p.setColor(QPalette::Text, color);
		outlineWidget->setPalette(p);
	}
}
