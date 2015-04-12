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

#ifndef BREADCRUMBSAAGHARMODEL_H
#define BREADCRUMBSAAGHARMODEL_H

#include "qirabstractbreadcrumbmodel.h"

class QMenu;
class QStringList;

QIR_BEGIN_NAMESPACE

class QIRONSHARED_EXPORT BreadCrumbSaagharModel : public QIrAbstractBreadCrumbModel
{
public:
    BreadCrumbSaagharModel();

    QString defaultPath() const;
    QString cleanPath(const QString &path) const;

    bool isValid(const QString &path) const;
    QIrBreadCrumbModelNodeList splitPath(const QString &path) const;

    QIcon icon(const QIrBreadCrumbModelNode &) const;
    QString label(const QIrBreadCrumbModelNode &) const;

    QMimeData* mimeData(const QIrBreadCrumbModelNode &) const;

    QMenu* buildMenu(const QIrBreadCrumbModelNode &);

    inline bool supportsMenuNavigation() const { return true; }

private:
    QStringList pathSections(const QString &path) const;
    QIrBreadCrumbModelNode::Type pathNodeType(const QStringList &sections) const;
    QIcon icon(const QIrBreadCrumbModelNode::Type &) const;
};

QIR_END_NAMESPACE

#endif // BREADCRUMBSAAGHARMODEL_H
