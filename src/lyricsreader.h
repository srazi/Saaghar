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

#ifndef LYRICSREADER_H
#define LYRICSREADER_H

#include <QDomDocument>
#include <QHash>
#include <QIcon>
#include <QTreeWidget>

class LyricsReader : public QObject
{
    Q_OBJECT

public:
    LyricsReader(QObject* parent = 0);

    bool read(QIODevice* device, const QString &format);

    int vorderByTime(qint64 time);

//public slots:

//    bool write(QIODevice* device);
//    bool updateBookmarkState(const QString &type, const QVariant &data, bool state);
//    QStringList bookmarkList(const QString &type);
//    void insertBookmarkList(const QVariantList &list);

//private slots:
//    bool unBookmarkItem(QTreeWidgetItem* item = 0);
//    void doubleClicked(QTreeWidgetItem* item, int column);
//    void updateDomElement(QTreeWidgetItem* item, int column);
//    void filterItems(const QString &str);

private:
//    QDomElement findChildNode(const QString &tagName, const QString &type);
//    void parseFolderElement(const QDomElement &element,
//                            QTreeWidgetItem* parentItem = 0, const QString &elementID = QString());
//    QTreeWidgetItem* createItem(const QDomElement &element,
//                                QTreeWidgetItem* parentItem = 0, const QString &elementID = QString());

    QDomDocument m_domDocument;
    QMap<qint64, int> m_syncMap;
//    QHash<QTreeWidgetItem*, QDomElement> m_domElementForItem;
//    QIcon m_folderIcon;
//    QIcon m_bookmarkIcon;

//signals:
//    void showBookmarkedItem(const QString &, const QString &, const QString &, bool, bool);
};

#endif // LYRICSREADER_H
