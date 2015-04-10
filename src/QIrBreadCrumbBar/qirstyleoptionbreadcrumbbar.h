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
**/
#ifndef QIRSTYLEOPTIONBREADCRUMBBAR_H
#define QIRSTYLEOPTIONBREADCRUMBBAR_H

#include <QStyleOption>
#include "QIronCommon/qiron_export.h"

class QComboBox;
class QPainter;

QIR_BEGIN_NAMESPACE

class QIRONSHARED_EXPORT QIrStyleOptionBreadCrumbIndicator : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_CustomBase + 1 };
    enum StyleOptionVersion { Version = 1 };

    QIrStyleOptionBreadCrumbIndicator();
    QIrStyleOptionBreadCrumbIndicator(const QIrStyleOptionBreadCrumbIndicator &other) : QStyleOption(Version, Type) { *this = other; }

    bool isTruncated;
    bool hasLabel;
    bool usePseudoState;
    bool isValid;
    bool isFlat;
};

class QIRONSHARED_EXPORT QIrStyleOptionBreadCrumbLabel : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_CustomBase + 2 };
    enum StyleOptionVersion { Version = 1 };

    QIrStyleOptionBreadCrumbLabel();
    QIrStyleOptionBreadCrumbLabel(const QIrStyleOptionBreadCrumbLabel &other) : QStyleOption(Version, Type) { *this = other; }

    QString text;
    bool hasIndicator;
    bool usePseudoState;
    bool isValid;
    bool isFlat;
};

QIR_END_NAMESPACE

#endif // QIRSTYLEOPTIONBREADCRUMBBAR_H
