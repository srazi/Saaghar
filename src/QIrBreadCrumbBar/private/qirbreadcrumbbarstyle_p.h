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
#ifndef QIRBREADCRUMBARSTYLE_P_H
#define QIRBREADCRUMBARSTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QIron API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "../qirbreadcrumbbarstyle.h"
#include "../QIronCommon/private/qirobject_p.h"

QIR_BEGIN_NAMESPACE

class QIrBreadCrumbBarStylePrivate : public QIrObjectPrivate
{
    QIR_DECLARE_OBJECT(QIrBreadCrumbBarStyle);

public:
    QIrBreadCrumbBarStylePrivate();
    ~QIrBreadCrumbBarStylePrivate();
};

class QIrDefaultBreadCrumbBarStylePrivate : public QIrBreadCrumbBarStylePrivate
{
    QIR_DECLARE_OBJECT(QIrDefaultBreadCrumbBarStyle);

public:
    QIrDefaultBreadCrumbBarStylePrivate();
    ~QIrDefaultBreadCrumbBarStylePrivate();
};

class QIrStyledBreadCrumbBarStylePrivate : public QIrDefaultBreadCrumbBarStylePrivate
{
    QIR_DECLARE_OBJECT(QIrStyledBreadCrumbBarStyle);

public:
    QIrStyledBreadCrumbBarStylePrivate();
    ~QIrStyledBreadCrumbBarStylePrivate();
};

QIR_END_NAMESPACE

#endif // QIRBREADCRUMBARSTYLE_P_H
