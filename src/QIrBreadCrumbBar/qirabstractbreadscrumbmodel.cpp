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
#include "qirabstractbreadcrumbmodel.h"


/////////////////////////
//QIrBreadCrumbModelNode
/////////////////////////
QIrBreadCrumbModelNode::QIrBreadCrumbModelNode(const QString &path, Type type,  const QIrAbstractBreadCrumbModel* model) :  m_path(path), m_type(type)
{
    if (type != Unknown && model) {
        m_label = model->label(*this);
    }
}
QIrBreadCrumbModelNode &QIrBreadCrumbModelNode::operator =(const QIrBreadCrumbModelNode &other)
{
    m_path = other.m_path;
    m_type = other.m_type;
    m_label = other.m_label;

    return *this;
}


/////////////////////////////
//QIrAbstractBreadCrumbModel
/////////////////////////////
QIrAbstractBreadCrumbModel::QIrAbstractBreadCrumbModel() : m_model(0)
{
}
QIrAbstractBreadCrumbModel::~QIrAbstractBreadCrumbModel()
{
}
void QIrAbstractBreadCrumbModel::setFilter(Filters filter)
{
    m_filter = filter == 0 ? Containers : filter;
}
void QIrAbstractBreadCrumbModel::setItemModel(QAbstractItemModel* model)
{
    if (!model || m_model == model) {
        return;
    }
    m_model = model;
}
