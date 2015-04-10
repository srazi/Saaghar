/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2015 by S. Razi Alavizadeh                          *
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
/**********************************************************************
**
** This file is part of QIron Toolkit.
**
** Copyright (C) 2009-2020 Dzimi Mve Alexandre <dzimiwine@gmail.com>
**
** Contact: dzimiwine@gmail.com
**
** QIron is a free toolkit developed in Qt by Dzimi Mve A.; you can redistribute
** sources and libraries of this library and/or modify them under the terms of
** the GNU Library General Public License version 3.0 as published by the
** Free Software Foundation and appearing in the file LICENSE.txt included in
** the packaging of this file.
** Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** This SDK is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
**********************************************************************/
#include "private/qirbreadcrumbbarstyle_p.h"
#include "qirbreadcrumbbar.h"
#include <QPainter>
#include <QComboBox>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QImage>

#define MIN_EMPTY_AREA_WIDTH 80
#define ARROW_WIDTH 15
#define ICONLABEL_WIDTH 20

static const char* const arrow_right_img[] = {
    "4 7 2 1",
    "   c None",
    "*  c #BFBFBF",
    "*   ",
    "**  ",
    "*** ",
    "****",
    "*** ",
    "**  ",
    "*   "
};

static const char* const arrow_down_img[] = {
    "7 4 2 1",
    "   c None",
    "*  c #BFBFBF",
    "*******",
    " ***** ",
    "  ***  ",
    "   *   "
};

static const char* const truncated_img[] = {
    "7 7 2 1",
    "   c None",
    "*  c #BFBFBF",
    "       ",
    "  ** **",
    " ** ** ",
    "** **  ",
    " ** ** ",
    "  ** **",
    "       "
};

///////////////////////////////
//QIrBreadCrumbBarStylePrivate
///////////////////////////////
QIrBreadCrumbBarStylePrivate::QIrBreadCrumbBarStylePrivate()
{
}
QIrBreadCrumbBarStylePrivate::~QIrBreadCrumbBarStylePrivate()
{
}

//////////////////////////////////////
//QIrDefaultBreadCrumbBarStylePrivate
//////////////////////////////////////
QIrDefaultBreadCrumbBarStylePrivate::QIrDefaultBreadCrumbBarStylePrivate()
{
}
QIrDefaultBreadCrumbBarStylePrivate::~QIrDefaultBreadCrumbBarStylePrivate()
{
}

//////////////////////////////////////
//QIrStyledBreadCrumbBarStylePrivate
/////////////////////////////////////
QIrStyledBreadCrumbBarStylePrivate::QIrStyledBreadCrumbBarStylePrivate()
{
}
QIrStyledBreadCrumbBarStylePrivate::~QIrStyledBreadCrumbBarStylePrivate()
{
}


////////////////////////
//QIrBreadCrumbBarStyle
////////////////////////
QIrBreadCrumbBarStyle::QIrBreadCrumbBarStyle() : QIrObject(* new QIrBreadCrumbBarStylePrivate)
{
}
QIrBreadCrumbBarStyle::~QIrBreadCrumbBarStyle()
{
}
QIrBreadCrumbBarStyle::QIrBreadCrumbBarStyle(QIrBreadCrumbBarStylePrivate &p) : QIrObject(p)
{
}


///////////////////////////////
//QIrDefaultBreadCrumbBarStyle
///////////////////////////////
QIrDefaultBreadCrumbBarStyle::QIrDefaultBreadCrumbBarStyle() : QIrBreadCrumbBarStyle(* new QIrDefaultBreadCrumbBarStylePrivate)
{
}
QIrDefaultBreadCrumbBarStyle::~QIrDefaultBreadCrumbBarStyle()
{
}
QIrDefaultBreadCrumbBarStyle::QIrDefaultBreadCrumbBarStyle(QIrDefaultBreadCrumbBarStylePrivate &p) : QIrBreadCrumbBarStyle(p)
{
}

QRect QIrDefaultBreadCrumbBarStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex* option, QStyle::SubControl subControl, const QWidget* widget) const
{
    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox* bar = qstyleoption_cast< const QStyleOptionComboBox* >(option)) {
            if (const QIrBreadCrumbBar* barW = qobject_cast< const QIrBreadCrumbBar* >(widget)) {
                QRect editRect  = lineEditRect(barW);

                switch (subControl) {
                case SC_BreadCrumbIcon:
                    return QRect(editRect.topLeft(), QSize(ICONLABEL_WIDTH, editRect.height()));
                case SC_BreadCrumbEditField:
                    return editRect.adjusted(subControlRect(control, bar, (QStyle::SubControl)SC_BreadCrumbIcon, barW).width(), 0, 0, 0);
                case SC_BreadCrumbContainer:
                    return subControlRect(control, bar, (QStyle::SubControl)SC_BreadCrumbEditField, barW);
                default:
                    break;
                }
            }
        }
        break;
    default:
        break;
    }
    return QIrBreadCrumbBarStyle::subControlRect(control, option, subControl, widget);
}
QRect QIrDefaultBreadCrumbBarStyle::subElementRect(QStyle::SubElement subElement, const QStyleOption* option, const QWidget* widget) const
{
    switch (subElement) {
    case SE_BreadCrumbIndicator :
        return QRect(0, 0, ARROW_WIDTH, option->rect.height());
    case SE_BreadCrumbEmptyArea :
        return QRect(0, 0, MIN_EMPTY_AREA_WIDTH, option->rect.height());
    case SE_BreadCrumbLabel :
        if (const QIrStyleOptionBreadCrumbLabel* lb = qstyleoption_cast< const QIrStyleOptionBreadCrumbLabel* >(option)) {
            int spacing = 6;

            return QRect(0, 0, lb->fontMetrics.boundingRect(lb->text).width() + 2 * spacing, lb->rect.height());
        }
        break;
    default:
        break;
    }
    return QIrBreadCrumbBarStyle::subElementRect(subElement, option, widget);
}
void QIrDefaultBreadCrumbBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_BreadCrumbContainerBase:
        painter->fillRect(option->rect, option->palette.base());
        return;
    default:
        break;
    }
    QIrBreadCrumbBarStyle::drawPrimitive(element, option, painter, widget);
}

void QIrDefaultBreadCrumbBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    bool hover = option->state & QStyle::State_MouseOver;
    bool down = option->state & QStyle::State_Sunken;
    QRect rect = option->rect;

    switch (element) {
    case CE_BreadCrumbIndicator:
        if (const QIrStyleOptionBreadCrumbIndicator* indicator = qstyleoption_cast< const QIrStyleOptionBreadCrumbIndicator* >(option)) {
            bool isLeftToRight = widget ? !widget->isRightToLeft() : true;
            bool valid = indicator->isValid;

            if (indicator->usePseudoState) {
                hover = false;
                down = false;
            }
            QImage downArrow(indicator->isTruncated ? truncated_img : down ? arrow_down_img : arrow_right_img);
            QColor color = option->palette.color(QPalette::Normal, QPalette::Highlight);

            downArrow.setColor(1, down ? !valid ? QColor(Qt::red).rgba() : color.dark(120).rgba() : hover ? !valid ? QColor(Qt::red).rgba() : color.rgba() : option->palette.foreground().color().rgba());
            painter->drawImage(rect.center().x() - downArrow.width() / 2,
                               rect.center().y() - downArrow.height() / 2 + 1,
                               isLeftToRight ? downArrow : downArrow.mirrored(false, true));

            return;
        }
        break;
    case CE_BreadCrumbLabel:
        if (const QIrStyleOptionBreadCrumbLabel* label = qstyleoption_cast< const QIrStyleOptionBreadCrumbLabel* >(option)) {
            bool valid = label->isValid;
            if (label->usePseudoState) {
                hover = false;
                down = false;
            }
            QColor color = option->palette.color(QPalette::Normal, QPalette::Highlight);
            painter->setPen(down ? !valid ? QColor(Qt::red).rgba() : color.dark(120) : hover ? !valid ? QColor(Qt::red).rgba() : color : option->palette.foreground().color());
            painter->drawText(option->rect, Qt::AlignCenter, label->text);

            return;
        }
        break;
    default:
        break;
    }
    QIrBreadCrumbBarStyle::drawControl(element, option, painter, widget);
}
QRect QIrDefaultBreadCrumbBarStyle::lineEditRect(const QIrBreadCrumbBar* bar) const
{
    QStyleOptionComboBox option;
    QComboBox* comboBox = bar->comboBox();
    QRect editRect;

    if (!bar->isEditable() || !comboBox) {
        editRect = bar->rect();
    }
    else {
        option.initFrom(comboBox);
        option.editable = true;
        option.frame = true;
        option.subControls = QStyle::SC_All;
        option.currentText = comboBox->currentText();
        option.currentIcon = QIcon();
        option.iconSize = comboBox->iconSize();
        option.state |= QStyle::State_On;
        // subControlRect() returned from Qt internal styles is reversed in RTL layout
        // but we compute editRect for LTR layout because of this we reverse it again.
        editRect = QStyle::visualRect(bar->layoutDirection(), bar->rect(), comboBox->style()->subControlRect(QStyle::CC_ComboBox, &option,
                                        QStyle::SC_ComboBoxEditField, comboBox));
    }

    return editRect;
}

////////////////////////////////////
//QIrStyledBreadCrumbBarStyle
////////////////////////////////////
QRect QIrStyledBreadCrumbBarStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex* option, QStyle::SubControl subControl, const QWidget* widget) const
{
    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox* comboBox = qstyleoption_cast< const QStyleOptionComboBox* >(option)) {
            switch (subControl) {
            case SC_BreadCrumbContainer: {
                QRect editRect  = QIrDefaultBreadCrumbBarStyle::subControlRect(control, option, (QStyle::SubControl)SC_BreadCrumbEditField, widget);
                QRect r(editRect.left(), 1, editRect.width(), comboBox->rect.height() - 2);
                return r;
            }
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
    return QIrDefaultBreadCrumbBarStyle::subControlRect(control, option, subControl, widget);
}
void QIrStyledBreadCrumbBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    bool hover = option->state & QStyle::State_MouseOver;
    bool down = option->state & QStyle::State_Sunken;
    QRect rect = option->rect;
    QColor color;

    switch (element) {
    case CE_BreadCrumbIndicator:
        if (const QIrStyleOptionBreadCrumbIndicator* indicator = qstyleoption_cast< const QIrStyleOptionBreadCrumbIndicator* >(option)) {
            QImage downArrow(indicator->isTruncated ? truncated_img : down && !indicator->usePseudoState ? arrow_down_img : arrow_right_img);
            bool isLeftToRight = widget ? !widget->isRightToLeft() : true;

            if (down || hover) {
                QLinearGradient g(rect.topLeft(), rect.bottomLeft());

                color = indicator->isValid ? option->palette.color(QPalette::Normal, QPalette::Highlight) : QColor(250, 170, 0);

                if (indicator->usePseudoState && hover) {
                    color = color.light();
                }
                else if (hover) {
                    color = color.light(120);
                }
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
                if (indicator->isFlat) {
                    painter->drawLine(rect.topLeft(), rect.topRight());
                    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
                }
                if (!(indicator->usePseudoState && hover)) {
                    painter->drawLine(rect.topLeft(), rect.bottomLeft());
                }
                painter->drawLine(rect.topRight(), rect.bottomRight());
            }
            downArrow.setColor(1, option->palette.foreground().color().rgba());
            painter->drawImage(rect.center().x() - downArrow.width() / 2,
                               rect.center().y() - downArrow.height() / 2 + 1,
                               isLeftToRight ? downArrow : downArrow.mirrored(true, false));

            return;
        }
        break;
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
            painter->drawText(textRect, Qt::AlignCenter, label->text);

            return;
        }
        break;
    default:
        break;
    }
    QIrDefaultBreadCrumbBarStyle::drawControl(element, option, painter, widget);
}

QIR_END_NAMESPACE
