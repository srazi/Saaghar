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
#include <QPixmap>
#include <QRect>
#include "qirsubstyle.h"

QIR_BEGIN_NAMESPACE

QIrSubStyle::QIrSubStyle()
{
}
QIrSubStyle::~QIrSubStyle()
{
}
void QIrSubStyle::drawComplexControl(ComplexControl , const QStyleOptionComplex* , QPainter* , const QWidget*) const
{
}
void QIrSubStyle::drawControl(ControlElement , const QStyleOption* , QPainter* , const QWidget*) const
{
}
void QIrSubStyle::drawPrimitive(PrimitiveElement , const QStyleOption* , QPainter* , const QWidget*) const
{
}
QPixmap QIrSubStyle::generatedIconPixmap(QIcon::Mode , const QPixmap & , const QStyleOption*) const
{
    return QPixmap();
}
QStyle::SubControl QIrSubStyle::hitTestComplexControl(ComplexControl , const QStyleOptionComplex* , const QPoint & , const QWidget*) const
{
    return QStyle::SC_None;
}
int QIrSubStyle::pixelMetric(PixelMetric , const QStyleOption* , const QWidget*) const
{
    return 0;
}
QSize QIrSubStyle::sizeFromContents(ContentsType , const QStyleOption* , const QSize & , const QWidget*) const
{
    return QSize();
}
QPixmap QIrSubStyle::standardPixmap(StandardPixmap, const QStyleOption*, const QWidget*) const
{
    return QPixmap();
}

int QIrSubStyle::styleHint(StyleHint , const QStyleOption* , const QWidget* , QStyleHintReturn*) const
{
    return 0;
}
QRect QIrSubStyle::subControlRect(ComplexControl , const QStyleOptionComplex* , SubControl , const QWidget*) const
{
    return QRect();
}
QRect QIrSubStyle::subElementRect(SubElement , const QStyleOption* , const QWidget*) const
{
    return QRect();
}

QIR_END_NAMESPACE
