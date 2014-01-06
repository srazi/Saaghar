/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2013 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
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

//#include <QMessageBox>
//#include <QHeaderView>
#include <QIODevice>
#include <QDebug>

#include "lyricsreader.h"
//#include "qganjoordbbrowser.h"

const int ID_DATA = Qt::UserRole + 1;

LyricsReader::LyricsReader(QObject* parent)
    : QObject(parent)
{
}

bool LyricsReader::read(QIODevice* device, const QString &format)
{
    if (format != "GANJOOR_XML") {
        return false;
    }

    QString errorStr;
    int errorLine;
    int errorColumn;

////    m_domDocument.clear();
////    m_syncMap.clear();
//qDebug() << "open=" <<    device->open(QIODevice::ReadOnly);
    if (!m_domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn)) {
        qDebug() << QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
        return false;
    }

    QDomElement root = m_domDocument.documentElement();
    if (root.tagName() != "DesktopGanjoorPoemAudioList") {
        qDebug() << "The file is not an DesktopGanjoorPoemAudioList file.";
        return false;
    }

    QDomNodeList syncInfoList = root.elementsByTagName("SyncInfo");

    for (int i = 0; i < syncInfoList.count(); ++i) {
        QDomElement e = syncInfoList.at(i).toElement();
        if(!e.isNull()) {
            bool ok;
            qint64 time = e.firstChildElement("AudioMiliseconds").text().toLongLong(&ok);
            int vorder = e.firstChildElement("VerseOrder").text().toInt(&ok);
            if (vorder >= 0) {
                ++vorder;
            }
            if (ok) {
                m_syncMap.insert(time, vorder);
            }
        }
    }
    qDebug() << m_syncMap.keys();
    qDebug() << m_syncMap.values();

    return !m_syncMap.isEmpty();
}

int LyricsReader::vorderByTime(qint64 time)
{
    if (m_syncMap.isEmpty() || time < 0) {
        return -1;
    }

    QMap<qint64, int>::const_iterator it = m_syncMap.constBegin();
    qint64 key = it.key();
    while (it != m_syncMap.constEnd()) {
        if (it.key() > time) {
            break;
        }
        key = it.key();
        ++it;
    }

    return m_syncMap.value(key, -1);
}

//bool Bookmarks::write(QIODevice* device)
//{
//    const int IndentSize = 4;

//    QTextStream out(device);
//    out.setCodec("utf-8");
//    QString domString = m_domDocument.toString(IndentSize);
//    domString = domString.replace("&#xd;", "");
//    out << domString;
//    return true;
//}

//void Bookmarks::updateDomElement(QTreeWidgetItem* item, int column)
//{
//    QDomElement element = m_domElementForItem.value(item);
//    if (!element.isNull()) {
//        if (column == 0) {
//            QDomElement oldTitleElement = element.firstChildElement("title");
//            QDomElement newTitleElement = m_domDocument.createElement("title");

//            QDomText newTitleText = m_domDocument.createTextNode(item->text(0));
//            newTitleElement.appendChild(newTitleText);

//            element.replaceChild(newTitleElement, oldTitleElement);
//        }
//        else {
//            if (element.tagName() == "bookmark") {
//                QDomElement oldDescElement = element.firstChildElement("desc");
//                QDomElement newDescElement = m_domDocument.createElement("desc");
//                QDomText newDesc = m_domDocument.createTextNode(item->text(1));//setAttribute("href", item->text(1));
//                newDescElement.appendChild(newDesc);
//                element.replaceChild(newDescElement, oldDescElement);
//                element.setAttribute("href", item->data(1, Qt::UserRole).toString());
//            }
//        }
//    }
//}

//void Bookmarks::parseFolderElement(const QDomElement &element,
//                                   QTreeWidgetItem* parentItem, const QString &/*elementID*/)
//{
//    QDomElement parentElement(element);
//    QString id = "";
//    if (parentElement.tagName() == "folder") {
//        //old files that their 'folder' tag don't use 'id' attribute
//        // just contain 'folder' tag of type 'Verses'!
//        id = parentElement.attribute("id", "Verses");
//        parentElement.setAttribute("id", id);
//    }

//    QTreeWidgetItem* item = createItem(parentElement, parentItem, id);

//    QString title;
//    if (id == "Verses") {
//        title = tr("Verses");
//    }
//    else {
//        title = element.firstChildElement("title").text();
//    }
//    if (title.isEmpty()) {
//        title = QObject::tr("Folder");
//    }

//    item->setIcon(0, m_folderIcon);
//    item->setText(0, title);
//    item->setToolTip(0, title);

//    bool folded = (parentElement.attribute("folded") != "no");
//    setItemExpanded(item, !folded);

//    QDomElement child = parentElement.firstChildElement();
//    while (!child.isNull()) {
//        if (child.tagName() == "folder") {
////TODO: we can save labguage within a 'metadata' tag of this 'folder'
////          //update node title by new loaded translation
//            QString id = child.attribute("id");
//            QString title = child.firstChildElement("title").text();

//            if (id.isEmpty()) {
//                //old files that their 'folder' tags don't use 'id' attribute just contain 'folder' tags of type 'Verses'!
//                id = "Verses";
//                title = tr(title.toLocal8Bit().data());
//                QDomElement oldTitleElement = child.firstChildElement("title");
//                QDomElement newTitleElement = m_domDocument.createElement("title");
//                QDomText newTitleText = m_domDocument.createTextNode(tr("Verses"));
//                newTitleElement.appendChild(newTitleText);

//                child.replaceChild(newTitleElement, oldTitleElement);
//                child.setAttribute("id", "Verses");
//            }
//            parseFolderElement(child, item, id);
//        }
//        else if (child.tagName() == "bookmark") {
//            QTreeWidgetItem* childItem = createItem(child, item);

//            QString title = child.firstChildElement("title").text();
//            if (title.isEmpty()) {
//                title = QObject::tr("Folder");
//            }

//            QDomElement infoChild = child.firstChildElement("info");
//            QDomElement metaData = infoChild.firstChildElement("metadata");
//            while (!metaData.isNull()) {
//                QString owner = metaData.attribute("owner");
//                if (owner == "http://saaghar.pozh.org") {
//                    break;
//                }
//                metaData = metaData.nextSiblingElement("metadata");
//            }
//            if (!metaData.isNull() && metaData.attribute("owner") == "http://saaghar.pozh.org") {
//                childItem->setData(0, Qt::UserRole, metaData.text());
//            }
//            else {
//                qDebug() << "This DOM-NODE SHOULD deleted--->" << title;
//            }
//            //href data URL data
//            childItem->setData(1, Qt::UserRole, child.attribute("href", "http://saaghar.pozh.org"));
//            childItem->setIcon(0, m_bookmarkIcon);
//            childItem->setText(0, title);
//            childItem->setToolTip(0, title);
//            childItem->setText(1, child.firstChildElement("desc").text());
//            childItem->setToolTip(1, child.firstChildElement("desc").text());
//        }
//        else if (child.tagName() == "separator") {
//            QTreeWidgetItem* childItem = createItem(child, item);
//            childItem->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEditable));
//            childItem->setText(0, QString(30, 0xB7));
//        }
//        child = child.nextSiblingElement();
//    }
//}

//QTreeWidgetItem* Bookmarks::createItem(const QDomElement &element,
//                                       QTreeWidgetItem* parentItem, const QString &elementID)
//{
//    QTreeWidgetItem* item;
//    if (parentItem) {
//        item = new QTreeWidgetItem(parentItem);
//    }
//    else {
//        item = new QTreeWidgetItem(this);
//    }

//    if (!elementID.isEmpty()) {
//        item->setData(0, ID_DATA, elementID);
//    }
//    m_domElementForItem.insert(item, element);
//    return item;
//}

//QDomElement Bookmarks::findChildNode(const QString &tagName, const QString &type)
//{
//    QDomElement n = m_domDocument.documentElement().firstChildElement(tagName);
//    while (!n.isNull()) {
//        //old files that their 'folder' tags don't use 'id' attribute just contain 'folder' tags of type 'Verses'!
//        QString id = n.attribute("id", "Verses");
//        if (id == type) {
//            break;
//        }

//        QDomElement e = n.toElement();
//        n = n.nextSiblingElement(tagName);
//    }
//    return n;
//}

//QStringList Bookmarks::bookmarkList(const QString &type)
//{
//    QStringList bookmarkedItemList;
//    if (type == "Verses" || type == tr("Verses")) {
//        QDomElement verseNode = findChildNode("folder", "Verses");
//        if (!verseNode.isNull()) {
//            QTreeWidgetItem* versesParentItem = m_domElementForItem.key(verseNode);
//            if (versesParentItem) {
//                int countOfChildren = versesParentItem->childCount();
//                for (int i = 0; i < countOfChildren; ++i) {
//                    QTreeWidgetItem* child = versesParentItem->child(i);
//                    if (child) {
//                        bookmarkedItemList << versesParentItem->child(i)->data(0, Qt::UserRole).toString();
//                    }
//                }
//            }
//        }
//    }

//    return bookmarkedItemList;
//}

//bool Bookmarks::updateBookmarkState(const QString &type, const QVariant &data, bool state)
//{
//    if (type == "Verses" || type == tr("Verses")) {
//        QDomElement verseNode = findChildNode("folder", "Verses");
//        if (verseNode.isNull()) {
//            QDomElement root = m_domDocument.createElement("folder");
//            QDomElement child = m_domDocument.createElement("title");
//            QDomText newTitleText = m_domDocument.createTextNode(tr("Verses"));
//            root.setAttribute("folded", "no");
//            child.appendChild(newTitleText);
//            root.appendChild(child);
//            m_domDocument.documentElement().appendChild(root);
//            verseNode = root;
//            parseFolderElement(root);
//        }

//        QTreeWidgetItem* parentItem = m_domElementForItem.key(verseNode);
//        //REMOVE OPERATION
//        if (!state) {
//            int countOfChildren = parentItem->childCount();
//            int numOfDel = 0;
//            bool allMatchedRemoved = true;
//            for (int i = 0; i < countOfChildren; ++i) {
//                QTreeWidgetItem* childItem = parentItem->child(i);
//                if (!childItem) {
//                    continue;
//                }

//                if (childItem->data(0, Qt::UserRole).toString() == data.toStringList().at(0) + "|" + data.toStringList().at(1)) {
//                    ++numOfDel;

//                    if (unBookmarkItem(childItem)) {
//                        --i; //because one of children was deleted
//                        --countOfChildren;
//                    }
//                    else {
//                        allMatchedRemoved = false;
//                    }
//                }
//            }

//            //allMatchedRemoved is false when at least one of
//            // matched items are not deleted!!
//            return allMatchedRemoved;
//        }

//        QDomElement bookmark = m_domDocument.createElement("bookmark");
//        QDomElement bookmarkTitle = m_domDocument.createElement("title");
//        QDomElement bookmarkDescription = m_domDocument.createElement("desc");
//        QDomElement bookmarkInfo = m_domDocument.createElement("info");
//        QDomElement infoMetaData = m_domDocument.createElement("metadata");

//        infoMetaData.setAttribute("owner", "http://saaghar.pozh.org");
//        QDomText bookmarkSaagharMetadata = m_domDocument.createTextNode(data.toStringList().at(0) + "|" + data.toStringList().at(1));
//        infoMetaData.appendChild(bookmarkSaagharMetadata);
//        bookmarkInfo.appendChild(infoMetaData);
//        bookmark.appendChild(bookmarkTitle);
//        bookmark.appendChild(bookmarkDescription);
//        bookmark.appendChild(bookmarkInfo);
//        verseNode.appendChild(bookmark);

//        qDebug() << data << state;
//        QDomElement firstChild = m_domDocument.documentElement().firstChildElement("folder");
//        firstChild.text();
//        QTreeWidgetItem* item = createItem(bookmark, parentItem);
//        item->setIcon(0, m_bookmarkIcon);

//        QString title = data.toStringList().at(2);
//        item->setText(0, title);
//        item->setToolTip(0, title);
//        item->setData(0, Qt::UserRole, data.toStringList().at(0) + "|" + data.toStringList().at(1));
//        item->setData(1, Qt::UserRole, data.toStringList().at(3));
//        if (data.toStringList().size() == 5) {
//            item->setText(1, data.toStringList().at(4));
//            item->setToolTip(1, data.toStringList().at(4));
//        }

//        if (parentItem->childCount() == 1) {
//            resizeColumnToContents(0);
//            resizeColumnToContents(1);
//        }

//        return true;
//    }

//    //an unknown type!!
//    return false;
//}

//void Bookmarks::doubleClicked(QTreeWidgetItem* item, int column)
//{
//    //setFlag() emits itemChanged() SIGNAL!
//    disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
//               this, SLOT(updateDomElement(QTreeWidgetItem*,int)));
//    //a tricky hack for enabling one column editing!!
//    if (column == 0) {
//        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
//    }
//    else if (column == 1 && item->parent()) {
//        item->setFlags(item->flags() | Qt::ItemIsEditable);
//    }
//    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(updateDomElement(QTreeWidgetItem*,int)));
//    if (item->parent()) {
//        if (column == 0) {
//            QString text = item->text(0);
//            int newLineIndex = text.indexOf("\n") + 1;
//            int secondNewLineIndex = text.indexOf("\n", newLineIndex);
//            int length = secondNewLineIndex > 0 ? secondNewLineIndex - newLineIndex : text.size() - newLineIndex;
//            text = text.mid(newLineIndex, length);

//            QString type = item->data(0, ID_DATA).toString();
//            if (type.isEmpty()) {
//                //old files that their 'folder' tags don't use 'id' attribute
//                // just contain 'folder' tags of type 'Verses'!
//                type = "Verses";
//            }
//            emit showBookmarkedItem(type, text, item->data(0, Qt::UserRole).toString(), true, true);
//        }
//    }
//}

//void Bookmarks::filterItems(const QString &str)
//{
//    for (int i = 0; i < topLevelItemCount(); ++i) {
//        QTreeWidgetItem* ithRootChild = topLevelItem(i);
//        int childCount = ithRootChild->childCount();
//        for (int j = 0; j < childCount; ++j) {
//            QTreeWidgetItem* child = ithRootChild->child(j);
//            QString text = child->text(0) + child->text(1);
//            text = QGanjoorDbBrowser::cleanString(text);
//            QString cleanedStr = QGanjoorDbBrowser::cleanString(str);
//            if (!cleanedStr.isEmpty() && !text.contains(cleanedStr)) {
//                child->setHidden(true);
//            }
//            else {
//                child->setHidden(false);
//            }
//        }
//    }
//}

//bool Bookmarks::unBookmarkItem(QTreeWidgetItem* item)
//{
//    if (!item) {
//        item = currentItem();
//    }
//    if (!item || !item->parent()) {
//        return false;
//    }

//    QDomElement elementForRemoving = m_domElementForItem.value(item);
//    bool deleteItem = true;
//    QString itemComment = elementForRemoving.firstChildElement("desc").text();
//    if (!itemComment.isEmpty()) {
//        QMessageBox bookmarkCommentWarning(QMessageBox::Warning, tr("Bookmark"),
//                                           tr("This bookmark has comment if you remove it, the comment will be deleted, too."
//                                              "\nThis operation can not be undoed!"
//                                              "\nBookmark's' Title:\n%1\n\nBookmark's Comment:\n%2")
//                                           .arg(item->text(0)).arg(itemComment),
//                                           QMessageBox::Ok | QMessageBox::Cancel, parentWidget());

//        if (bookmarkCommentWarning.exec() == QMessageBox::Cancel) {
//            deleteItem = false;
//        }
//    }
//    if (deleteItem) {
//        if (!elementForRemoving.isNull()) {
//            elementForRemoving.parentNode().removeChild(elementForRemoving);
//        }

//        QString text = item->text(0);

//        int newLineIndex = text.indexOf("\n") + 1;
//        int secondNewLineIndex = text.indexOf("\n", newLineIndex);
//        int length = secondNewLineIndex > 0 ? secondNewLineIndex - newLineIndex : text.size() - newLineIndex;
//        text = text.mid(newLineIndex, length);

//        QString type = item->data(0, ID_DATA).toString();
//        if (type.isEmpty()) {
//            //old files that their 'folder' tags don't use 'id' attribute
//            // just contain 'folder' tags of type 'Verses'!
//            type = "Verses";
//        }
//        emit showBookmarkedItem(type, text, item->data(0, Qt::UserRole).toString(), false, true);
//        delete item;
//        item = 0;
//    }
//    return deleteItem;
//}

//void Bookmarks::insertBookmarkList(const QVariantList &list)
//{
//    QStringList bookmarkedData = bookmarkList("Verses");
//    for (int i = 0; i < list.size(); ++i) {
//        QStringList data = list.at(i).toStringList();

//        if (!bookmarkedData.contains(data.at(0) + "|" + data.at(1))) {
//            updateBookmarkState("Verses", list.at(i), true);
//        }

//        QString text = data.at(2);
//        int newLineIndex = text.indexOf("\n") + 1;
//        int secondNewLineIndex = text.indexOf("\n", newLineIndex);
//        int length = secondNewLineIndex > 0 ? secondNewLineIndex - newLineIndex : text.size() - newLineIndex;
//        text = text.mid(newLineIndex, length);
//        emit showBookmarkedItem("Verses", text, data.at(0) + "|" + data.at(1), false, false);
//    }
//}
