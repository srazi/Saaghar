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

#include "searchpatternmanager.h"
#include "searchitemdelegate.h"
#include "databaseelements.h"
#include "saagharwidget.h"
#include "tools.h"

#include <QApplication>
#include <qmath.h>
#include <QPainter>
#include <QIcon>

SaagharItemDelegate::SaagharItemDelegate(QWidget* parent, QStyle* style, QString phrase) : QItemDelegate(parent)
{
    keywordList.clear();
    keywordList = SearchPatternManager::instance()->phraseToList(phrase, false);
    keywordList.removeDuplicates();

    parentWidget = parent;

    tableStyle = style;
    opacity = 0.65;
    if (parent->objectName() == "searchTable") {
        opacity = 1;
    }
}

#if 0 // old highlight algorithm
void SaagharItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    bool flagRightToLeft = true;
    if (parentWidget) {
        flagRightToLeft = parentWidget->layoutDirection() == Qt::RightToLeft;
    }

    int textHMargin = 0;
    if (tableStyle) {
        textHMargin = tableStyle->pixelMetric(QStyle::PM_FocusFrameHMargin, &option) + 1 ;
    }

    QBrush itemBrush = painter->brush();

    itemBrush.setColor(SaagharWidget::matchedTextColor);
    itemBrush.setStyle(Qt::SolidPattern);

    QString text = "";
    QString cleanedText = "";
    int lastX, x;
    QFontMetrics fontMetric(index.data(Qt::FontRole).value<QFont>());

    const QString tatweel = QString(0x0640);

    int iconWidth = 0;

    if (index.data().isValid()) {
        text = index.data().toString();

        if (index.data(Qt::DecorationRole).isValid()) {
            QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
            iconWidth = icon.pixmap(100, 100).width() + 5;
        }

        cleanedText = Tools::cleanString(text);
        text = Tools::cleanString(text, QStringList() << " " << Tools::someSymbols);
    }

    Qt::Alignment itemAlignment = 0;
    QVariant alignValue = index.data(Qt::TextAlignmentRole);

    if (alignValue.isValid() && !alignValue.isNull()) {
        itemAlignment = Qt::Alignment(alignValue.toInt());
    }

    int keywordsCount = keywordList.size();
    for (int i = 0; i < keywordsCount; ++i) {
        lastX = x = option.rect.x() + textHMargin;
        QString keyword = keywordList.at(i);

        keyword.replace(QChar(0x200C), "", Qt::CaseInsensitive);//replace ZWNJ by ""
        keyword = keyword.split("", SKIP_EMPTY_PARTS).join(tatweel + "*");
        keyword.replace("@" + tatweel + "*", "\\S*", Qt::CaseInsensitive); //replace wildcard by word chars
        QRegExp maybeTatweel(keyword, Qt::CaseInsensitive);
        maybeTatweel.indexIn(text);
        keyword = maybeTatweel.cap(0);
        if (!(keyword.isEmpty() || text.indexOf(keyword) == -1)) {
            QString txt = text;
            while (txt.size() > 0) {
                int index = txt.indexOf(keyword);
                QString thisPart;
                if (index == -1) {
                    thisPart = txt;
                    txt = QString();
                }
                else {
                    if (index == 0) {
                        thisPart = txt.mid(0, keyword.size());
                        if (txt == keyword) {
                            txt = QString();
                        }
                        else {
                            txt = txt.mid(keyword.size(), txt.size() - keyword.size());
                        }
                    }
                    else {
                        thisPart = txt.mid(0, index);
                        txt = txt.mid(index);
                    }
                }

                QSize sz = fontMetric.boundingRect(thisPart).size();
                if (index == 0) {
                    if (flagRightToLeft) {
                        switch (itemAlignment ^ Qt::AlignVCenter) {
                        case Qt::AlignRight:
                            lastX = option.rect.left() + textHMargin + fontMetric.boundingRect(text).width() - (lastX - option.rect.x() + sz.width() - textHMargin);
                            break;

                        case Qt::AlignHCenter:
                            lastX = option.rect.left() + textHMargin + fontMetric.boundingRect(text).width() - (lastX - option.rect.x() + sz.width() - textHMargin) + ((option.rect.width() - fontMetric.boundingRect(text).width() - textHMargin) / 2);
                            break;

                        case Qt::AlignLeft:
                        default:
                            lastX = option.rect.right() + textHMargin - 1 - (lastX - option.rect.x() + sz.width());
                            break;
                        }
                    }
                    else {
                        if ((itemAlignment ^ Qt::AlignVCenter) == Qt::AlignHCenter) {
                            lastX = option.rect.left() + textHMargin + fontMetric.boundingRect(text).width() - (lastX - option.rect.x() + sz.width() - textHMargin) + ((option.rect.width() - fontMetric.boundingRect(text).width() - textHMargin) / 2);
                        }
                    }

                    QRect rectf(lastX , option.rect.y() + ((option.rect.height() - qMin(option.rect.height(), fontMetric.height())) / 2), sz.width(), fontMetric.height());
                    if (!flagRightToLeft && ((itemAlignment ^ Qt::AlignVCenter) == Qt::AlignHCenter)) {
                        rectf = QStyle::visualRect(Qt::RightToLeft, option.rect, rectf);
                    }

                    qreal oldOpacity = painter->opacity();
                    painter->setOpacity(opacity);
                    rectf.adjust(-iconWidth, 0, -iconWidth, 0);
                    QPainterPath roundedRect;
                    roundedRect.addRoundRect(rectf, 50, 50);
                    QPen defaultPen(painter->pen());
                    painter->setPen(SaagharWidget::matchedTextColor.darker(150));
                    painter->drawPath(roundedRect);
                    painter->fillPath(roundedRect, itemBrush);
                    painter->setOpacity(oldOpacity);
                    painter->setPen(defaultPen);
                    //painter->fillRect( rectf, itemBrush );
                }
                x += fontMetric.width(thisPart);
                lastX = x;
            }
        }
        else if (!(keyword.isEmpty() || cleanedText.indexOf(keyword) == -1)) {
            qreal oldOpacity = painter->opacity();
            painter->setOpacity(0.35);
            painter->fillRect(option.rect, itemBrush);
            painter->setOpacity(oldOpacity);
        }
    }
    QItemDelegate::paint(painter, option, index);
}
#endif // old highlight algorithm

// taken from qitemdelegate.cpp
inline static QString replaceNewLine(QString text)
{
    const QChar nl = QLatin1Char('\n');
    for (int i = 0; i < text.count(); ++i) {
        if (text.at(i) == nl) {
            text[i] = QChar::LineSeparator;
        }
    }

    return text;
}

// taken from qitemdelegate.cpp
void SaagharItemDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, option.palette.brush(cg, QPalette::Highlight));
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    }
    else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    if (text.isEmpty()) {
        return;
    }

    if (option.state & QStyle::State_Editing) {
        painter->save();
        painter->setPen(option.palette.color(cg, QPalette::Text));
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
        painter->restore();
    }

#if QT_VERSION >= 0x050000
    const QStyleOptionViewItem opt = option;
    const QStyleOptionViewItem* v3 = &option;
    const bool wrapText = opt.features & QStyleOptionViewItem::WrapText;
#else
    const QStyleOptionViewItemV4 opt = option;
    const QStyleOptionViewItemV3* v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    const bool wrapText = opt.features & QStyleOptionViewItemV2::WrapText;
#endif

    QStyle* style = v3 && v3->widget ? v3->widget->style() : QApplication::style();
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, (v3 && v3->widget ? v3->widget : 0)) + 1;
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    m_textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    m_textOption.setTextDirection(option.direction);
    m_textOption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
    m_textLayout.setTextOption(m_textOption);
    m_textLayout.setFont(option.font);
    m_textLayout.setText(replaceNewLine(text));

    QSizeF textLayoutSize = doTextLayout(textRect.width());

    if (textRect.width() < textLayoutSize.width()
            || textRect.height() < textLayoutSize.height()) {
        QString elided;
        int start = 0;
        int end = text.indexOf(QChar::LineSeparator, start);
        if (end == -1) {
            elided += option.fontMetrics.elidedText(text, option.textElideMode, textRect.width());
        }
        else {
            while (end != -1) {
                elided += option.fontMetrics.elidedText(text.mid(start, end - start),
                                                        option.textElideMode, textRect.width());
                elided += QChar::LineSeparator;
                start = end + 1;
                end = text.indexOf(QChar::LineSeparator, start);
            }
            //let's add the last line (after the last QChar::LineSeparator)
            elided += option.fontMetrics.elidedText(text.mid(start),
                                                    option.textElideMode, textRect.width());
        }
        m_textLayout.setText(elided);
        textLayoutSize = doTextLayout(textRect.width());
    }

    const QSize layoutSize(textRect.width(), int(textLayoutSize.height()));
    const QRect layoutRect = QStyle::alignedRect(option.direction, option.displayAlignment,
                             layoutSize, textRect);
    // if we still overflow even after eliding the text, enable clipping
    if (!hasClipping() && (textRect.width() < textLayoutSize.width()
                           || textRect.height() < textLayoutSize.height())) {
        painter->save();
        painter->setClipRect(layoutRect);
        m_textLayout.draw(painter, layoutRect.topLeft(), QVector<QTextLayout::FormatRange>(), layoutRect);
        painter->restore();
    }
    else {
        m_textLayout.draw(painter, layoutRect.topLeft(), QVector<QTextLayout::FormatRange>(), layoutRect);
    }
}

// taken from qitemdelegate.cpp
QRect SaagharItemDelegate::textRectangle(QPainter* /*painter*/, const QRect &rect,
        const QFont &font, const QString &text) const
{
    m_textOption.setWrapMode(QTextOption::WordWrap);
    m_textLayout.setTextOption(m_textOption);
    m_textLayout.setFont(font);
    m_textLayout.setText(replaceNewLine(text));
    QSizeF fpSize = doTextLayout(rect.width());
    const QSize size = QSize(qCeil(fpSize.width()), qCeil(fpSize.height()));
    // ###: textRectangle should take style option as argument
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    return QRect(0, 0, size.width() + 2 * textMargin, size.height());
}

// taken from qitemdelegate.cpp
QSizeF SaagharItemDelegate::doTextLayout(int lineWidth) const
{
    updateAdditionalFormats();

    qreal height = 0;
    qreal widthUsed = 0;
    m_textLayout.beginLayout();
    while (true) {
        QTextLine line = m_textLayout.createLine();
        if (!line.isValid()) {
            break;
        }
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    m_textLayout.endLayout();

    return QSizeF(widthUsed, height);
}

void SaagharItemDelegate::updateAdditionalFormats() const
{
    m_additionalFormats.clear();

#if QT_VERSION >= 0x050F00
    m_textLayout.clearFormats();
#else
    m_textLayout.clearAdditionalFormats();
#endif

    const QString &text = m_textLayout.text();
    qreal pointSize = m_textLayout.font().pointSizeF() + 2.0;
    //int fontWeight = m_textLayout.font().weight() + 50;
    QColor highlight = SaagharWidget::matchedTextColor;

    // see: http://gamedev.stackexchange.com/a/38542
    qreal l = 0.2126 * highlight.redF() * highlight.redF() +
              0.7152 * highlight.greenF() * highlight.greenF() +
              0.0722 * highlight.blueF() * highlight.blueF();

    if (l > 0.85) {
        highlight = highlight.darker(150);
        highlight.setAlpha(200);
    }
    else if (l > 0.6) {
        highlight = highlight.lighter(105);
        highlight.setAlpha(40);
    }
    else if (l > 0.4) {
        highlight = highlight.lighter(120);
        highlight.setAlpha(50);
    }
    else {
        highlight = highlight.lighter(140);
        highlight.setAlpha(30);
    }

    int keywordsCount = keywordList.size();
    for (int i = 0; i < keywordsCount; ++i) {
        QString keyword = keywordList.at(i);
        if (keyword.isEmpty()) {
            continue;
        }

        keyword = keyword.split("", SKIP_EMPTY_PARTS).join("[" + Tools::OTHER_GLYPHS + "]*") + "[" + Tools::OTHER_GLYPHS + "]*";
        keyword.replace("@[" + Tools::OTHER_GLYPHS + "]*", "\\S*", Qt::CaseInsensitive); //replace wildcard by word chars
        keyword.replace(Tools::AE_Variant.at(0), "(" + Tools::AE_Variant.join("|") + ")");
        keyword.replace(Tools::Ye_Variant.at(0), "(" + Tools::Ye_Variant.join("|") + ")");
        keyword.replace(Tools::He_Variant.at(0), "(" + Tools::He_Variant.join("|") + ")");
        keyword.replace(Tools::Ve_Variant.at(0), "(" + Tools::Ve_Variant.join("|") + ")");

#if QT_VERSION_MAJOR >= 5
        QRegularExpression maybeOthers(keyword, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = maybeOthers.globalMatch(text);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QTextLayout::FormatRange fr;
            fr.start = match.capturedStart();
            fr.length = match.capturedLength(0);

            if (fr.length == 0) {
                break;
            }

            fr.format.setFontPointSize(pointSize);
            fr.format.setForeground(SaagharWidget::matchedTextColor);
            fr.format.setBackground(highlight);
            fr.format.setFontWeight(99);

            m_additionalFormats << fr;
        }
#else
        QRegExp maybeOthers(keyword, Qt::CaseInsensitive);

        int pos = 0;
        while ((pos = maybeOthers.indexIn(text, pos)) != -1) {
            QTextLayout::FormatRange fr;
            fr.start = pos;
            fr.length = maybeOthers.matchedLength();

            if (fr.length == 0) {
                break;
            }
            pos += fr.length;

            fr.format.setFontPointSize(pointSize);
            fr.format.setForeground(SaagharWidget::matchedTextColor);
            fr.format.setBackground(highlight);
            fr.format.setFontWeight(99);

            m_additionalFormats << fr;
        }
#endif
    }

    if (!m_additionalFormats.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
        m_textLayout.setFormats(m_additionalFormats);
#else
        m_textLayout.setAdditionalFormats(m_additionalFormats);
#endif
    }
}

void SaagharItemDelegate::keywordChanged(const QString &text)
{
    keywordList.clear();
    QString tmp = text;
    keywordList = SearchPatternManager::instance()->phraseToList(tmp, false);
    keywordList.removeDuplicates();

    if (QAbstractScrollArea* scrollArea = qobject_cast<QAbstractScrollArea*>(parent())) {
        scrollArea->viewport()->update();
    }
}

/***********************************************/
/////////ParagraphHighlighter Class//////////////
/***********************************************/
ParagraphHighlighter::ParagraphHighlighter(QTextDocument* parent, const QString &phrase)
    : QSyntaxHighlighter(parent)
{
    keywordList.clear();
    keywordList = SearchPatternManager::instance()->phraseToList(phrase, false);
    keywordList.removeDuplicates();
}

void ParagraphHighlighter::keywordChanged(const QString &text)
{
    keywordList.clear();
    QString tmp = text;
    keywordList = SearchPatternManager::instance()->phraseToList(tmp, false);
    keywordList.removeDuplicates();
    rehighlight();
}

void ParagraphHighlighter::highlightBlock(const QString &text)
{
    int keywordsCount = keywordList.size();

    QColor highlight = SaagharWidget::matchedTextColor;
    // see: http://gamedev.stackexchange.com/a/38542
    qreal l = 0.2126 * highlight.redF() * highlight.redF() +
              0.7152 * highlight.greenF() * highlight.greenF() +
              0.0722 * highlight.blueF() * highlight.blueF();

    if (l > 0.85) {
        highlight = highlight.darker(150);
        highlight.setAlpha(200);
    }
    else if (l > 0.6) {
        highlight = highlight.lighter(105);
        highlight.setAlpha(40);
    }
    else if (l > 0.4) {
        highlight = highlight.lighter(120);
        highlight.setAlpha(50);
    }
    else {
        highlight = highlight.lighter(140);
        highlight.setAlpha(30);
    }

    QTextCharFormat paragraphHighightFormat;
    paragraphHighightFormat.setForeground(SaagharWidget::matchedTextColor);
    paragraphHighightFormat.setBackground(highlight);

    for (int i = 0; i < keywordsCount; ++i) {
        QString keyword = keywordList.at(i);

        keyword = keyword.split("", SKIP_EMPTY_PARTS).join("[" + Tools::OTHER_GLYPHS + "]*") + "[" + Tools::OTHER_GLYPHS + "]*";
        keyword.replace("@[" + Tools::OTHER_GLYPHS + "]*", "\\S*", Qt::CaseInsensitive); //replace wildcard by word chars
        keyword.replace(Tools::AE_Variant.at(0), "(" + Tools::AE_Variant.join("|") + ")");
        keyword.replace(Tools::Ye_Variant.at(0), "(" + Tools::Ye_Variant.join("|") + ")");
        keyword.replace(Tools::He_Variant.at(0), "(" + Tools::He_Variant.join("|") + ")");
        keyword.replace(Tools::Ve_Variant.at(0), "(" + Tools::Ve_Variant.join("|") + ")");
#if QT_VERSION_MAJOR >= 5
        QRegularExpression maybeOthers(keyword, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = maybeOthers.globalMatch(text);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int index = match.capturedStart();
            int length = match.capturedLength(0);
            if (length == 0) {
                break;
            }

            setFormat(index, length, paragraphHighightFormat);
        }
#else
        QRegExp maybeOthers(keyword, Qt::CaseInsensitive);
        int index = maybeOthers.indexIn(text);
        while (index >= 0) {
            int length = maybeOthers.matchedLength();
            if (length == 0) {
                break;
            }

            setFormat(index, length, paragraphHighightFormat);
            index = maybeOthers.indexIn(text, index + length);
        }
#endif
    }
}
