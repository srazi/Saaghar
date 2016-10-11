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
#include "qirstyleoptionbreadcrumbbar.h"

QIR_BEGIN_NAMESPACE

////////////////////////////////
//QIrStyleOptionBreadCrumbIndicator
////////////////////////////////
QIrStyleOptionBreadCrumbIndicator::QIrStyleOptionBreadCrumbIndicator() : QStyleOption(Version, Type),
    isTruncated(false), hasLabel(true), usePseudoState(false), isValid(true), isFlat(false)
{
}

/////////////////////////////////
//QIrStyleOptionBreadCrumbLabel
/////////////////////////////////
QIrStyleOptionBreadCrumbLabel::QIrStyleOptionBreadCrumbLabel() : QStyleOption(Version, Type), text(QString()),
    hasIndicator(true), usePseudoState(false), isValid(true), isFlat(false)
{
}

QIR_END_NAMESPACE
