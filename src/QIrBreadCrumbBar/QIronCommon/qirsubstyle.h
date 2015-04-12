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
#ifndef QIRSUBSTYLE_H
#define QIRSUBSTYLE_H

#include <QCommonStyle>
#include "qiron_export.h"

QIR_BEGIN_NAMESPACE

class QIRONSHARED_EXPORT QIrSubStyle : public QCommonStyle
{
    Q_OBJECT

public:
    QIrSubStyle();
    ~QIrSubStyle();

    virtual void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = 0) const;
    virtual void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const;
    virtual void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const;
    virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption* option) const;
    virtual SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex* option, const QPoint &position, const QWidget* widget = 0) const;
    virtual int pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0) const;
    virtual QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* opt = 0,
                                   const QWidget* widget = 0) const;
    virtual QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize &contentsSize, const QWidget* widget = 0) const;
    virtual int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
    virtual QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget = 0) const;
    virtual QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget = 0) const;

private:
    Q_DISABLE_COPY(QIrSubStyle)
};

QIR_END_NAMESPACE
#endif // QIRSUBSTYLE_H
