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
#ifndef QIRBREADCRUMBBARSTYLE_H
#define QIRBREADCRUMBBARSTYLE_H

#include "QIronCommon/qirsubstyle.h"
#include "QIronCommon/qirobject.h"
#include "qirstyleoptionbreadcrumbbar.h"

class QComboBox;

QIR_BEGIN_NAMESPACE

class QIrBreadCrumbBar;
class QIrBreadCrumbBarStylePrivate;
class QIrDefaultBreadCrumbBarStylePrivate;
class QIrStyledBreadCrumbBarStylePrivate;

class QIRONSHARED_EXPORT QIrBreadCrumbBarStyle : public QIrSubStyle, public QIrObject
{
    Q_OBJECT
    QIR_DECLARE_PRIVATE(QIrBreadCrumbBarStyle)

public:
    enum SubControl {SC_BreadCrumbContainer, SC_BreadCrumbEditField, SC_BreadCrumbIcon};
    enum SubElement {SE_BreadCrumbIndicator, SE_BreadCrumbLabel, SE_BreadCrumbEmptyArea};
    enum PrimitiveElement {PE_FrameBreadCrumbBar, PE_BreadCrumbContainerBase};
    enum ControlElement {CE_BreadCrumbIndicator, CE_BreadCrumbLabel, CE_BreadCrumbEmptyArea};

    QIrBreadCrumbBarStyle();
    ~QIrBreadCrumbBarStyle();

protected:
    QIrBreadCrumbBarStyle(QIrBreadCrumbBarStylePrivate &p);
};

class QIRONSHARED_EXPORT QIrDefaultBreadCrumbBarStyle : public QIrBreadCrumbBarStyle
{
    Q_OBJECT
    QIR_DECLARE_PRIVATE(QIrDefaultBreadCrumbBarStyle)

public:
    QIrDefaultBreadCrumbBarStyle();
    ~QIrDefaultBreadCrumbBarStyle();

    virtual QRect subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex* option, QStyle::SubControl subControl, const QWidget* widget = 0) const;
    virtual QRect subElementRect(QStyle::SubElement subElement, const QStyleOption* option, const QWidget* widget = 0) const;
    virtual void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const;
    virtual void drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const;

protected:
    QIrDefaultBreadCrumbBarStyle(QIrDefaultBreadCrumbBarStylePrivate &p);
    QRect lineEditRect(const QIrBreadCrumbBar* bar) const;
};

class QIRONSHARED_EXPORT QIrStyledBreadCrumbBarStyle : public QIrDefaultBreadCrumbBarStyle
{
public:
    QIrStyledBreadCrumbBarStyle() {}
    virtual ~QIrStyledBreadCrumbBarStyle() {}

    virtual QRect subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex* option, QStyle::SubControl subControl, const QWidget* widget = 0) const;
    virtual void drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const;

protected:
    QIrStyledBreadCrumbBarStyle(QIrStyledBreadCrumbBarStylePrivate &p);
};

QIR_END_NAMESPACE

#endif // QIRBREADCRUMBBARSTYLE_H
