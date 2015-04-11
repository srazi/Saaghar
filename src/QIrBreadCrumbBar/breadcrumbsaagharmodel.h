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
