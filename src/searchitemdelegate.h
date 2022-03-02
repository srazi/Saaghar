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

#ifndef SEARCHITEMDELEGATE_H
#define SEARCHITEMDELEGATE_H

#include <QItemDelegate>
#include <QSyntaxHighlighter>
#include <QTextLayout>
#include <QTextOption>

class SaagharItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SaagharItemDelegate(QWidget* parent = 0, QStyle* style = 0, const QString phrase = QString());

private:
#if 0 // old highlight algorithm
    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
#endif
    void drawDisplay(QPainter* painter, const QStyleOptionViewItem &option,
                     const QRect &rect, const QString &text) const;

    QRect textRectangle(QPainter* painter, const QRect &rect,
                        const QFont &font, const QString &text) const;

    QSizeF doTextLayout(int lineWidth) const;

    void updateAdditionalFormats() const;

    QWidget* parentWidget;
    qreal opacity;
    QStyle* tableStyle;
    QStringList keywordList;

    mutable QTextLayout m_textLayout;
    mutable QTextOption m_textOption;
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    mutable QVector<QTextLayout::FormatRange> m_additionalFormats;
#else
    mutable QList<QTextLayout::FormatRange> m_additionalFormats;
#endif

private slots:
    void keywordChanged(const QString &text);
};

class ParagraphHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ParagraphHighlighter(QTextDocument* parent = 0, const QString &phrase = "");

protected:
    void highlightBlock(const QString &text);

private:
    QStringList keywordList;

private slots:
    void keywordChanged(const QString &text);
};
#endif // SEARCHITEMDELEGATE_H
