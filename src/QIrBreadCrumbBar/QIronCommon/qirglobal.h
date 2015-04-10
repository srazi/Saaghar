/**********************************************************************
**
** This file is part of QIron Toolkit.
**
** Copyright (C) 2009-2010 Dzimi Mve Alexandre <dzimiwine@gmail.com>
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
#ifndef QIRGLOBAL_H
#define QIRGLOBAL_H

#include <QtGlobal>
#define QIR_DECLARE_FLAGS Q_DECLARE_FLAGS

#define QIR_POINT_AWAY QPoint(-100000,-100000)
#define QIR_POINTF_AWAY QPointF(-100000,-100000)

//QIrWidget
#define QIR_DECLARE_UI( Class ) \
    inline Class##Ui * ui_func() { return reinterpret_cast< Class##Ui * > (ui_ptr); } \
    inline const Class##Ui * ui_func() const { return reinterpret_cast< const Class##Ui * > (ui_ptr); } \
    friend class Class##Ui;

#define QIR_DECLARE_WIDGET( Class ) \
    inline Class * w_func() { return static_cast< Class * > (w_ptr); } \
    inline const Class * w_func() const { return static_cast< const Class * > (w_ptr); } \
    friend class Class;

#define QIR_UI( Class ) Class##Ui * ui = ui_func()
#define QIR_W( Class ) Class * w = w_func()
#define QIR_WIDGET_CAST inline QWidget * toWidget() { return this; } \
    inline const QWidget * toWidget() const { return this; }

//QIrObject
#define QIR_DECLARE_PRIVATE( Class ) \
    inline Class##Private * p_func() { return reinterpret_cast< Class##Private * > (p_ptr); } \
    inline const Class##Private * p_func() const { return reinterpret_cast< const Class##Private * > (p_ptr); } \
    friend class Class##Private;

#define QIR_DECLARE_OBJECT( Class ) \
    inline Class * o_func() { return static_cast< Class * > (o_ptr); } \
    inline const Class * o_func() const { return static_cast< const Class * > (o_ptr); } \
    friend class Class;

#define QIR_P( Class ) Class##Private * p = p_func()
#define QIR_O( Class ) Class * o = o_func()

#define QIR_NO_RECURSIVE(called, ret)\
    if ( called ) \
        return ret; \
    QIrBoolBlocker blocker(called)

#ifndef QIR_NAMESPACE
#define QIR_NAMESPACE QIron
#endif // QIR_NAMESPACE

#if QT_VERSION <= 0x040502 || QT_VERSION == 0x040600
#define QIR_NO_NAMESPACE_SUPPORT
#endif
#define QIR_NO_NAMESPACE_SUPPORT

#ifndef QIR_NO_NAMESPACE_SUPPORT
#define QIR_BEGIN_NAMESPACE namespace QIR_NAMESPACE {
#define QIR_END_NAMESPACE }
#define QIR_USE_NAMESPACE using namespace ::QIR_NAMESPACE;
#else
#define QIR_BEGIN_NAMESPACE
#define QIR_END_NAMESPACE
#define QIR_USE_NAMESPACE
#endif //QIR_NAMESPACE_SUPPORT

#endif // QIRGLOBAL_H
