/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
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

#include <QtGui>

#include "bookmarks.h"

Bookmarks::Bookmarks(QWidget *parent)
	: QTreeWidget(parent)
{
	QStringList labels;
	labels << tr("Title") << tr("Comments");

	header()->setResizeMode(QHeaderView::Interactive/*QHeaderView::Stretch*/);
	setHeaderLabels(labels);

	folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
						 QIcon::Normal, QIcon::Off);
	folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
						 QIcon::Normal, QIcon::On);
	//bookmarkIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
	bookmarkIcon.addPixmap(QPixmap(":/resources/images/bookmark-on.png"));

	setAlternatingRowColors(true);

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(doubleClicked(QTreeWidgetItem*,int)));
}

bool Bookmarks::read(QIODevice *device)
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!domDocument.setContent(device, true, &errorStr, &errorLine,
								&errorColumn)) {
		//QMessageBox::information(window(), tr("DOM Bookmarks"), tr("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr));
		return false;
	}

	QDomElement root = domDocument.documentElement();
	if (root.tagName() != "xbel") {
		//QMessageBox::information(window(), tr("DOM Bookmarks"), tr("The file is not an XBEL file."));
		return false;
	} else if (root.hasAttribute("version")
			   && root.attribute("version") != "1.0") {
		//QMessageBox::information(window(), tr("DOM Bookmarks"), tr("The file is not an XBEL version 1.0 file."));
		return false;
	}

	clear();

	disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			this, SLOT(updateDomElement(QTreeWidgetItem*,int)));

	QDomElement child = root.firstChildElement("folder");
	while (!child.isNull()) {
		parseFolderElement(child);
		child = child.nextSiblingElement("folder");
	}

	resizeColumnToContents(0);
	resizeColumnToContents(1);

	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			this, SLOT(updateDomElement(QTreeWidgetItem*,int)));

	return true;
}

bool Bookmarks::write(QIODevice *device)
{
	const int IndentSize = 4;

	QTextStream out(device);
	out.setCodec("utf-8");
	//domDocument.save(out, IndentSize);
	QString domString = domDocument.toString(IndentSize);
	//domString = domString.replace("\n", "//");
	domString = domString.replace("&#xd;", "");
	out << domString;
	return true;
}

void Bookmarks::updateDomElement(QTreeWidgetItem *item, int column)
{
	QDomElement element = domElementForItem.value(item);
	if (!element.isNull())
	{
		if (column == 0)
		{
			QDomElement oldTitleElement = element.firstChildElement("title");
			QDomElement newTitleElement = domDocument.createElement("title");

			QDomText newTitleText = domDocument.createTextNode(item->text(0));
			newTitleElement.appendChild(newTitleText);

			element.replaceChild(newTitleElement, oldTitleElement);
		}
		else
		{
			if (element.tagName() == "bookmark")
			{
				QDomElement oldDescElement = element.firstChildElement("desc");
				QDomElement newDescElement = domDocument.createElement("desc");
				QDomText newDesc = domDocument.createTextNode(item->text(1));//setAttribute("href", item->text(1));
				newDescElement.appendChild(newDesc);
				element.replaceChild(newDescElement, oldDescElement);
				element.setAttribute("href", item->data(1, Qt::UserRole).toString());
			}
		}
		sortItems(0, Qt::AscendingOrder);
	}
}

void Bookmarks::parseFolderElement(const QDomElement &element,
								QTreeWidgetItem *parentItem)
{
	QTreeWidgetItem *item = createItem(element, parentItem);

	QString title = element.firstChildElement("title").text();
	if (title.isEmpty())
		title = QObject::tr("Folder");

	//item->setFlags(item->flags() | Qt::ItemIsEditable);
	item->setIcon(0, folderIcon);
	item->setText(0, title);

	bool folded = (element.attribute("folded") != "no");
	setItemExpanded(item, !folded);

	QDomElement child = element.firstChildElement();
	while (!child.isNull()) {
		if (child.tagName() == "folder") {
			parseFolderElement(child, item);
		} else if (child.tagName() == "bookmark") {
			QTreeWidgetItem *childItem = createItem(child, item);

			QString title = child.firstChildElement("title").text();
			if (title.isEmpty())
				title = QObject::tr("Folder");
////////////////////////////////////////////////////////////////////////////////
			QDomElement infoChild = child.firstChildElement("info");
			QDomElement metaData = infoChild.firstChildElement("metadata");
			while (!metaData.isNull()) {
				QString owner = metaData.attribute("owner");
				if (owner == "http://pojh.iblogger.org/saaghar") break;
				metaData = metaData.nextSiblingElement("metadata");
			}
			if (!metaData.isNull() && metaData.attribute("owner") == "http://pojh.iblogger.org/saaghar")
				childItem->setData(0, Qt::UserRole, metaData.text());
			else
				qDebug() << "This DOM-NODE SHOULD deleted--->"<<title;
			//href data URL data
			childItem->setData(1, Qt::UserRole, child.attribute("href", "http://pojh.iblogger.org/saaghar"));
////////////////////////////////////////////////////////////////////////////////
			//childItem->setFlags(item->flags() | Qt::ItemIsEditable);
			childItem->setIcon(0, bookmarkIcon);
			childItem->setText(0, title);
			childItem->setText(1, child.firstChildElement("desc").text()/* attribute("href")*/);
		} else if (child.tagName() == "separator") {
			QTreeWidgetItem *childItem = createItem(child, item);
			childItem->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEditable));
			childItem->setText(0, QString(30, 0xB7));
		}
		child = child.nextSiblingElement();
	}
}

QTreeWidgetItem *Bookmarks::createItem(const QDomElement &element,
									  QTreeWidgetItem *parentItem)
{
	QTreeWidgetItem *item;
	if (parentItem) {
		item = new QTreeWidgetItem(parentItem);
	} else {
		item = new QTreeWidgetItem(this);
	}
	domElementForItem.insert(item, element);
	return item;
}

QDomElement Bookmarks::findChildNode(const QString &tagName, const QString &type)
{
	//QDomNodeList nodeList = domDocument.elementsByTagName("folder");
	QDomElement n = domDocument.documentElement().firstChildElement(tagName);
	while (!n.isNull()) {
		if (n.firstChildElement("title").text() == tr(type.toUtf8().data()))
		{
			qDebug() << type.toUtf8().data()<<"IIIIFFF BREAK" << "localName=" << n.localName() << "nodName=" << n.nodeName() << "title="<< n.firstChildElement("title").text();
			break;
		}
		QDomElement e = n.toElement();
		qDebug() << "Element name: " << e.tagName();
		qDebug() << "localName=" << n.localName() << "nodName=" << n.nodeName() << "title="<< n.firstChildElement("title").text();
		n = n.nextSiblingElement(tagName);
	}
	return n;
}

QStringList Bookmarks::bookmarkList(const QString &type)
{
	QStringList bookmarkedItemList;
	if (type == "Verses")
	{
		QDomElement verseNode = findChildNode("folder", "Verses");
		qDebug() << "bookmarkList VERSE-NODE" << "localName=" << verseNode.localName() << "nodName=" << verseNode.nodeName() << "title="<< verseNode.firstChildElement("title").text();
		if (!verseNode.isNull())
		{
			QTreeWidgetItem *versesParentItem = domElementForItem.key(verseNode);
			if (versesParentItem)
			{
				int countOfChildren = versesParentItem->childCount();
				for (int i=0; i<countOfChildren;++i)
				{
					QTreeWidgetItem *child = versesParentItem->child(i);
					if (child)
					{
						qDebug()<<"bookmarkList"<< countOfChildren << versesParentItem->child(i)->text(0);
						bookmarkedItemList << versesParentItem->child(i)->data(0, Qt::UserRole).toString();
					}
				//QList<QTreeWidgetItem *> verseBookmarks = versesParentItem->takeChildren();
				}
			}
		}
	}
	//return bookmarkHash.values(type);
	return bookmarkedItemList;
}

void Bookmarks::updateBookmarkState(const QString &type, const QVariant &data, bool state)
{
	if (type == "Verses")
	{
		QDomElement verseNode = findChildNode("folder", "Verses");
		if (verseNode.isNull())
		{
			qDebug() << "NOOOOO VERSE" << "localName=" << verseNode.localName() << "nodName=" << verseNode.nodeName() << "title="<< verseNode.firstChildElement("title").text();
			QDomElement root = domDocument.createElement("folder");
			QDomElement child = domDocument.createElement("title");
			QDomText newTitleText = domDocument.createTextNode(tr("Verses"));
			root.setAttribute("folded", "no");
			child.appendChild(newTitleText);
			root.appendChild(child);
			domDocument.documentElement().appendChild(root);
			verseNode = root;
			parseFolderElement(root);
		}
//		else
//		{
//			verseNode = n;
//		}

		QTreeWidgetItem *parentItem = domElementForItem.key(verseNode);
//REMOVE OPERATION
		if (!state)
		{
//			QList<QTreeWidgetItem *> items = findItems(data.toStringList().at(2), /*Qt::MatchContains|*/Qt::MatchFixedString|Qt::MatchRecursive/*|Qt::MatchWrap*/, 0);
//			qDebug() << "REMOVE----OPERATION" << "size=" << items.size() << data.toStringList().at(2);
//			for (int i=0; i< items.size(); ++i)
//			{
//				qDebug() << "REMOVE OPERATION" << "size=" << items.size() << items.at(i)->text(0)<<items.at(i)->data(0, Qt::UserRole).toString();
//				if (items.at(i)->data(0, Qt::UserRole).toString() == data.toStringList().at(0)+"|"+data.toStringList().at(1))
//				{
//					QDomElement elementForRemoving = domElementForItem.value(items.at(i));
//					if (!elementForRemoving.isNull())
//						verseNode.removeChild(elementForRemoving);
//					//delete
//					items.at(i)->setBackgroundColor(1, QColor(Qt::red));
//				}
//			}
//////////////////////////////////////////////
			int countOfChildren = parentItem->childCount();
			//QList<QTreeWidgetItem *> items = findItems(data.toStringList().at(2), /*Qt::MatchContains|*/Qt::MatchFixedString|Qt::MatchRecursive/*|Qt::MatchWrap*/, 0);
			//qDebug() << "REMOVE----OPERATION" << "size=" << items.size() << data.toStringList().at(2);
			int numOfDel = 0;
			//QList<int> deleteList;
			for (int i=0; i< countOfChildren; ++i)
			{
				QTreeWidgetItem *childItem = parentItem->child(i);
				if (!childItem) continue;
				qDebug() << "REMOVE OPERATION" << "size=" << countOfChildren << "tex="<<childItem->text(0)<<"\nfor="<<data.toStringList().at(2)<<"\n"<<childItem->data(0, Qt::UserRole).toString()<<data.toStringList().at(0)+"|"+data.toStringList().at(1);
				if (childItem->data(0, Qt::UserRole).toString() == data.toStringList().at(0)+"|"+data.toStringList().at(1))
				{++numOfDel;
					qDebug() << "REMOVED!!";
					QDomElement elementForRemoving = domElementForItem.value(childItem);
					if (!elementForRemoving.isNull())
						verseNode.removeChild(elementForRemoving);
					//deleteList << i;
					delete childItem;
					--i;//because one of children was deleted
					--countOfChildren;
					//childItem->setBackgroundColor(1, QColor(Qt::red));
				}
			}
			qDebug() << "NUM OF DELETE-->REMOVE OPERATION=" << numOfDel;
/////////////////////////////////////////////////
			return;
		}

		QString tmpT = "Parent ZERO";
		if (parentItem)
			tmpT = parentItem->text(0)+"|"+parentItem->text(1);
		qDebug() << "THERE IS VERSE" << "tmpT=" << tmpT << "localName=" << verseNode.localName() << "nodName=" << verseNode.nodeName() << "title="<< verseNode.firstChildElement("title").text();
//		for (int i=0; i<nodeList.size(); ++i)
//		{
			
//			qDebug() << "size=" << nodeList.size() << "i=" << i << "localName=" << nodeList.at(i).localName() << "nodName=" << nodeList.at(i).nodeName() << "title="<< nodeList.at(i).firstChildElement("title").text();
//		}

		QDomElement bookmark = domDocument.createElement("bookmark");
		QDomElement bookmarkTitle = domDocument.createElement("title");
		QDomElement bookmarkDescription = domDocument.createElement("desc");
		QDomElement bookmarkInfo = domDocument.createElement("info");
		QDomElement infoMetaData = domDocument.createElement("metadata");

		infoMetaData.setAttribute("owner", "http://pojh.iblogger.org/saaghar");
		QDomText bookmarkSaagharMetadata = domDocument.createTextNode(data.toStringList().at(0)+"|"+data.toStringList().at(1));
		infoMetaData.appendChild(bookmarkSaagharMetadata);
		bookmarkInfo.appendChild(infoMetaData);
		bookmark.appendChild(bookmarkTitle);
		bookmark.appendChild(bookmarkDescription);
		bookmark.appendChild(bookmarkInfo);
		verseNode.appendChild(bookmark);

		qDebug() << data << state;
		QDomElement firstChild = domDocument.documentElement().firstChildElement("folder");
		firstChild.text();
		QTreeWidgetItem *item = createItem(bookmark, parentItem /*firstChild*/);  //new QTreeWidgetItem(SaagharWidget::bookmarks);
		item->setIcon(0, bookmarkIcon);
		//item->setFlags(item->flags() | Qt::ItemIsEditable);
		QString title = data.toStringList().at(2);
		//title = title.replace("//","\n");
		item->setText(0, title);
		//item->setText(1, data.toStringList().at(3));//"="+data.toStringList().at(0)+"|"+data.toStringList().at(1)
		item->setData(0, Qt::UserRole, data.toStringList().at(0)+"|"+data.toStringList().at(1));
		item->setData(1, Qt::UserRole, data.toStringList().at(3));
		bookmarkHash.insert("Verses", data.toStringList().at(0)+"|"+data.toStringList().at(1));
	}
}

void Bookmarks::doubleClicked(QTreeWidgetItem *item, int column)
{
	if (item->parent())
	{
		if (column == 1)
			openPersistentEditor(item, 1);// return;//editItem(item, column);
		else if (column == 0)
		{
			emit showBookmarkedItem(item->parent()->text(0), item->data(0, Qt::UserRole).toString());
		}
	}
}
