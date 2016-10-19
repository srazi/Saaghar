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
#ifndef QIRABSTRACTBREADCRUMBMODEL_H
#define QIRABSTRACTBREADCRUMBMODEL_H

#include "QIronCommon/qiron_export.h"
#include <QString>

class QAbstractItemModel;
class QIcon;
class QIrAbstractBreadCrumbModel;
class QMenu;
class QMimeData;
template< typename T >
class QList;

QIR_BEGIN_NAMESPACE

class QIRONSHARED_EXPORT QIrBreadCrumbModelNode
{
public:
    enum Type {Global, Root, Container, Leaf, Unknown};
    QIrBreadCrumbModelNode() : m_path(QString()), m_label(QString()), m_type(Unknown)
    { }
    explicit QIrBreadCrumbModelNode(const QString &path, Type type, const QIrAbstractBreadCrumbModel* model = 0);

    inline QIrBreadCrumbModelNode(const QIrBreadCrumbModelNode &other) : m_path(other.m_path), m_label(other.m_label),
        m_type(other.m_type)
    { }

    inline Type type() const { return m_type; }
    inline QString path() const { return m_path; }
    inline QString label() const { return m_label; }

    QIrBreadCrumbModelNode &operator = (const QIrBreadCrumbModelNode &other);

private:
    QString m_path;
    QString m_label;
    Type m_type;
};
typedef QList< QIrBreadCrumbModelNode > QIrBreadCrumbModelNodeList;

class QIRONSHARED_EXPORT QIrAbstractBreadCrumbModel
{
public:
    enum Filter {
        Containers = 1,
        AllNodes = 2,
        Hidden = 4
    };
    Q_DECLARE_FLAGS(Filters, Filter)

    QIrAbstractBreadCrumbModel();
    virtual ~QIrAbstractBreadCrumbModel();

    inline QAbstractItemModel* itemModel() const { return m_model; }

    inline Filters filter() const { return m_filter; }
    void setFilter(Filters filter);

    virtual QString defaultPath() const = 0;
    virtual QString cleanPath(const QString &) const = 0;

    virtual bool isValid(const QString &path) const = 0;
    virtual QIrBreadCrumbModelNodeList splitPath(const QString &path) const = 0;

    virtual QIcon icon(const QIrBreadCrumbModelNode &) const = 0;
    virtual QString label(const QIrBreadCrumbModelNode &) const = 0;

    virtual QMimeData* mimeData(const QIrBreadCrumbModelNode &) const = 0;

    virtual QMenu* buildMenu(const QIrBreadCrumbModelNode &) = 0;

    virtual bool supportsMenuNavigation() const = 0;

protected:
    void setItemModel(QAbstractItemModel*) const;

private:
    mutable QAbstractItemModel* m_model;
    Filters m_filter;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QIrAbstractBreadCrumbModel::Filters)

QIR_END_NAMESPACE

#endif // QIRABSTRACTBREADCRUMBMODEL_H
