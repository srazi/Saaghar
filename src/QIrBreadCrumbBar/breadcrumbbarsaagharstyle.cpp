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
#include "breadcrumbbarsaagharstyle.h"

#include <QPainter>

void BreadCrumbBarSaagharStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    bool hover = option->state & QStyle::State_MouseOver;
    bool down = option->state & QStyle::State_Sunken;
    QRect rect = option->rect;
    QColor color;

    switch (element) {
    case CE_BreadCrumbLabel:
        if (const QIrStyleOptionBreadCrumbLabel* label = qstyleoption_cast< const QIrStyleOptionBreadCrumbLabel* >(option)) {
            color = option->palette.color(QPalette::Normal, QPalette::Highlight);
            QRect textRect = rect;

            color = label->isValid ? option->palette.color(QPalette::Normal, QPalette::Highlight) : QColor(250, 170, 0);

            if (down || hover) {
                if (label->usePseudoState && hover) {
                    color = color.light();
                }
                else if (down) {
                    textRect.adjust(1, 1, 1, 1);
                }
                else {
                    color = color.light(120);
                }

                QLinearGradient g(rect.topLeft(), rect.bottomLeft());
                QColor c = color;

                c.setAlpha(120);
                g.setColorAt(0, c.lighter());
                g.setColorAt(0.495, c.lighter(120));
                g.setColorAt(0.5, c);
                g.setColorAt(1, c);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QBrush(g));
                painter->drawRect(rect);
                painter->setPen(color);
                if (label->isFlat) {
                    painter->drawLine(rect.topLeft(), rect.topRight());
                    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
                }
                painter->drawLine(rect.topLeft(), rect.bottomLeft());
                if (!label->hasIndicator || (!label->usePseudoState && hover)) {
                    painter->drawLine(rect.topRight(), rect.bottomRight());
                }
            }
            painter->setPen(option->palette.foreground().color());

            const static QChar LRE(0x202a);
            const static QChar RLE(0x202b);
            const QChar embeddingChar = label->text.isRightToLeft() ? RLE : LRE;

            const QString text = label->fontMetrics.elidedText(label->text, Qt::ElideMiddle, textRect.width());
            painter->drawText(textRect, Qt::AlignCenter, embeddingChar + text);

            return;
        }
        break;
    default:
        break;
    }

    QIrStyledBreadCrumbBarStyle::drawControl(element, option, painter, widget);
}

QIR_END_NAMESPACE
