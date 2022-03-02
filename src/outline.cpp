/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2016 by S. Razi Alavizadeh                          *
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

#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QHeaderView>
#include <QSearchLineEdit>

#include "outline.h"
#include "searchitemdelegate.h"
#include "tools.h"
#include "outlinemodel.h"
#include "saagharapplication.h"

#ifdef DEV_TOOLS
#include <QFileDialog>
#include <QTextEdit>

#include "databasebrowser.h"
#endif

OutlineTree::OutlineTree(QWidget* parent)
    : QWidget(parent)
{
    pressedMouseButton = Qt::LeftButton;

    m_outlineView = new QTreeView(parent);
    m_outlineView->setModel(sApp->outlineModel());
    m_outlineView->setObjectName("outlineTreeWidget");
    m_outlineView->setLayoutDirection(Qt::RightToLeft);
    m_outlineView->setTextElideMode(Qt::ElideMiddle);
#ifdef Q_OS_WIN
    m_outlineView->setIndentation(10);
#endif
    m_outlineView->header()->hide();
    m_outlineView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_outlineView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(createCustomContextMenu(QPoint)));

    QStringList labels;
    labels << tr("Title") << tr("Comments");

    setObjectName("OutLineWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* toolsLayout = new QHBoxLayout;

    QLabel* filterLabel = new QLabel;
    filterLabel->setObjectName(QString::fromUtf8("OutLineTreeFilterLabel"));
    filterLabel->setText(tr("Filter:"));
    QSearchLineEdit* outLineFilter = new QSearchLineEdit(this);
    outLineFilter->setObjectName("outLineFilter");
#if QT_VERSION >= 0x040700
    outLineFilter->setPlaceholderText(tr("Filter"));
#else
    outLineFilter->setToolTip(tr("Filter"));
#endif
    connect(outLineFilter, SIGNAL(textChanged(QString)), this, SLOT(filterItems(QString)));
    connect(outLineFilter, SIGNAL(clearButtonPressed()), this, SLOT(filterItems()));

    QSpacerItem* filterHorizSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    toolsLayout->addWidget(filterLabel, 0, Qt::AlignRight | Qt::AlignCenter);
    toolsLayout->addWidget(outLineFilter);
    toolsLayout->addItem(filterHorizSpacer);

    mainLayout->addWidget(m_outlineView);
    mainLayout->addLayout(toolsLayout);
    setLayout(mainLayout);

    connect(m_outlineView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
    connect(m_outlineView, SIGNAL(clicked(QModelIndex)), this, SLOT(clicked(QModelIndex)));
    connect(m_outlineView, SIGNAL(pressed(QModelIndex)), this, SLOT(pressed()));

    SaagharItemDelegate* filterDelegate = new SaagharItemDelegate(m_outlineView, m_outlineView->style());
    m_outlineView->setItemDelegate(filterDelegate);
    connect(outLineFilter, SIGNAL(textChanged(QString)), filterDelegate, SLOT(keywordChanged(QString)));
}

void OutlineTree::refreshTree()
{
    sApp->outlineModel(m_connectionID)->clear();

    m_outlineView->setModel(sApp->outlineModel(m_connectionID));
}

bool OutlineTree::filterItems(const QString &str, const QModelIndex &parent)
{
    int childrenSize = m_outlineView->model()->rowCount(parent);

    bool result = true;

    for (int i = 0; i < childrenSize; ++i) {
        QModelIndex ithChild = m_outlineView->model()->index(i, 0, parent);

        QString text = ithChild.data().toString();
        text = Tools::cleanString(text);
        QString cleanStr = Tools::cleanString(str);
        if (!cleanStr.isEmpty() && !text.contains(cleanStr)) {
            // if all its children are hidden it should be hidden, too.
            if (m_outlineView->model()->rowCount(ithChild) > 0) {
                bool tmp = filterItems(cleanStr, ithChild);
                if (!tmp) {
                    m_outlineView->setExpanded(ithChild, true);
                }

                m_outlineView->setRowHidden(i, parent, tmp);
                result = result && tmp;
            }
            else {
                m_outlineView->setRowHidden(i, parent, true);
                //return true when all children are hidden
                result = result && true;
            }
        }
        else {
            recursivelyUnHide(ithChild);
            m_outlineView->setExpanded(ithChild, false);
            //return false when at least one child is visible
            result = result && false;
        }
    }

    return result;
}

void OutlineTree::recursivelyUnHide(const QModelIndex &parent)
{
    int childrenSize = m_outlineView->model()->rowCount(parent);
    if (parent.isValid()) {
        m_outlineView->setRowHidden(parent.row(), parent.parent(), false);
    }

    for (int i = 0; i < childrenSize; ++i) {
        QModelIndex ithChild = m_outlineView->model()->index(i, 0, parent);

        recursivelyUnHide(ithChild);
    }
}

void OutlineTree::doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        emit openParentRequested(index.data(OutlineModel::IDRole).toInt());
    }
}

void OutlineTree::clicked(const QModelIndex &index)
{
    if (pressedMouseButton == Qt::RightButton) {
        return;
    }

    if (index.isValid()) {
        if (m_outlineView->model()->rowCount(index) > 0) {
            m_outlineView->expand(index);
        }
        else {
            emit openParentRequested(index.data(OutlineModel::IDRole).toInt());
        }
    }
}

void OutlineTree::createCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = m_outlineView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    int itemID = index.data(OutlineModel::IDRole).toInt();

    QMenu* contextMenu = new QMenu;
    contextMenu->addAction(tr("Open in New Tab"));
    contextMenu->addAction(tr("Open"));
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Random in New Tab"));
    contextMenu->addAction(tr("Random"));
    contextMenu->addSeparator();
    if (m_outlineView->model()->rowCount(index) > 0) {
        if (m_outlineView->isExpanded(index)) {
            contextMenu->addAction(tr("Collapse"));
        }
        else {
            contextMenu->addAction(tr("Expand"));
        }
    }

#ifdef DEV_TOOLS
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Create Empty Audio List"));
    contextMenu->addAction(tr("Create Audio List From File"));
#endif

    QAction* action = contextMenu->exec(QCursor::pos());
    if (!action) {
        return;
    }

    QString text = action->text();
    text.remove("&");

    if (text == tr("Open in New Tab")) {
        emit newParentRequested(itemID);
    }
    else if (text == tr("Open")) {
        emit openParentRequested(itemID);
    }
    else if (text == tr("Collapse") || text == tr("Expand")) {
        m_outlineView->setExpanded(index, !m_outlineView->isExpanded(index));
    }
    else if (text == tr("Random in New Tab")) {
        emit openRandomRequested(itemID, true);
    }
    else if (text == tr("Random")) {
        emit openRandomRequested(itemID, false);
    }
#ifdef DEV_TOOLS
    else if (text == tr("Create Audio List From File")) {
        createAudioList(itemID, true);
    }
    else if (text == tr("Create Empty Audio List")) {
        createAudioList(itemID, false);
    }
#endif

}

void OutlineTree::pressed()
{
    pressedMouseButton = QApplication::mouseButtons();
}

void OutlineTree::setTreeFont(const QFont &font)
{
    if (m_outlineView) {
        m_outlineView->setFont(font);
    }
}

void OutlineTree::setTreeColor(const QColor &color)
{
    if (m_outlineView && color.isValid()) {
        QPalette p(m_outlineView->palette());
        p.setColor(QPalette::Text, color);
        m_outlineView->setPalette(p);
    }
}

void OutlineTree::setConnectionID(const QString &connectionID)
{
    if (!connectionID.isEmpty() && connectionID != m_connectionID) {
        m_connectionID = connectionID;
        m_outlineView->setModel(sApp->outlineModel(m_connectionID));
    }
}

void OutlineTree::createAudioList(int catID, bool askInputMediaList)
{
#ifdef DEV_TOOLS
    QString theConnectionID = m_connectionID;
    if (theConnectionID.isEmpty()) {
        theConnectionID = DatabaseBrowser::defaultConnectionId();
    }

    QStringList lowMediaLinks;
    QStringList hqMediaLinks;

    if (askInputMediaList) {
        QString fileName = QFileDialog::getOpenFileName(0, tr("Select File Containing media links..."), QDir::homePath(), tr("Text File (*.txt);;All (*.*)"));

        QFile file(fileName);

        if (!file.open(QFile::ReadOnly)) {
            return;
        }

        QStringList lines = QString::fromUtf8(file.readAll()).replace("\r", "\n").split("\n", SKIP_EMPTY_PARTS);

        foreach (QString line, lines) {
            line = line.trimmed();
            if (line.isEmpty()) {
                continue;
            }
            QStringList tokens = line.split("=", SKIP_EMPTY_PARTS);
            if (tokens.size() == 2) {
                if (tokens.at(0) != "mp3") {
                    lowMediaLinks << tokens.at(1);
                }
                else {
                    hqMediaLinks << tokens.at(1);
                }
            }
            else if (tokens.size() == 1) {
                lowMediaLinks << tokens.at(0);
            }
        }
    }

    QStringList audioList;
    const QString poemAudio = QString("<PoemAudio>\n"
                                      "    <audio_post_ID>%1</audio_post_ID>\n"
                                      "    <audio_order>%2</audio_order>\n"
                                      "    <audio_xml>%3</audio_xml>\n"
                                      "    <audio_ogg>%4</audio_ogg>\n"
                                      "    <audio_mp3_hq>%5</audio_mp3_hq>\n"
                                      "    <audio_mp3>%6</audio_mp3>\n"
                                      "    <audio_title>%7</audio_title>\n"
                                      "    <audio_artist>%8</audio_artist>\n"
                                      "    <audio_artist_url>%9</audio_artist_url>\n"
                                      "</PoemAudio>");
    QList<GanjoorPoem*> poems = sApp->databaseBrowser()->getPoems(catID, theConnectionID);

    for(int i = 0; i < poems.size(); ++i) {
        GanjoorPoem* poem = poems.at(i);
        QString hqLink;
        QString lowLink;
        if (hqMediaLinks.size() > i) {
            hqLink = hqMediaLinks.at(i);
        }

        if (lowMediaLinks.size() > i) {
            lowLink = lowMediaLinks.at(i);
        }

        audioList << poemAudio.arg(poem->_ID)
                     .arg(1)
                     .arg(QString())
                     .arg(QString())
                     .arg(hqLink)
                     .arg(lowLink)
                     .arg(poem->_Title)
                     .arg(QString())
                     .arg(QString());
    }

    QTextEdit *doc = new QTextEdit(0);
    doc->setPlainText(audioList.join("\n"));
    doc->show();
#endif
}
