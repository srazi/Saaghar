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
#include <QApplication>
#include <QHash>
#include <QList>
#include <QMutexLocker>
#include <QWidget>
#include "qirsubstyle.h"
#include "QIronCommon/ui/qirwidget_ui.h"

QIR_BEGIN_NAMESPACE

QHash< QString, QPointer< QIrSubStyle > > QIrWidgetUi::subStyles;
QIrWidgetList QIrWidgetUi::allWidgets;

class StyleMutex : public QHash< QString, QMutex*>
{
public:
    StyleMutex() { }
    ~StyleMutex() {
        foreach (QString key, keys()) {
            delete(*this).take(key);
        }
    }
};

static QMutex* styleMutex(const QString &key)
{
    static StyleMutex mutexes;

    if (mutexes.contains(key.trimmed())) {
        mutexes[key] = new QMutex;
    }
    return mutexes[key];
}

///////////////
//QIrWidgetUi
///////////////
QIrWidgetUi::QIrWidgetUi() : w_ptr(0), extraSubStyle(0)
{
}
QIrWidgetUi::~QIrWidgetUi()
{
}
void QIrWidgetUi::setupUi()
{
    QString key = w_ptr->widgetGroupKey();

    if (!QIrWidget::subStyle(key)) {
        QMutexLocker locker(styleMutex(key));

        if (!QIrWidget::subStyle(key)) {
            QIrWidget::setSubStyle(key, w_ptr->defaultSubStyle());
        }
    }
    createActions();
    createToolBars();
    createDocks();
    createCentralWidget();
    createMenus();
    init();
    setupConnections();
    if (QIrSubStyle* style = w_ptr->subStyle()) {
        QWidget* widget = w_ptr->toWidget();

        w_ptr->subStyleChanged();
        style->polish(widget);
        widget->update();
    }
}

////////////
//QIrWidget
////////////
QIrWidget::QIrWidget() : ui_ptr(new QIrWidgetUi)
{
    QIrWidgetUi::allWidgets.append(this);
    ui_ptr->w_ptr = this;
}
QIrWidget::~QIrWidget()
{
    QIrWidgetUi::allWidgets.removeAll(this);
    if (ui_ptr) {
        delete ui_ptr;
    }
}
QIrWidget::QIrWidget(QIrWidgetUi &ui) : ui_ptr(&ui)
{
    QIrWidgetUi::allWidgets.append(this);
    ui_ptr->w_ptr = this;
}
QIrWidgetList QIrWidget::allWidgets(const QString &widgetGroupKey)
{
    QString key = widgetGroupKey.trimmed();
    QIrWidgetList list;

    if (key.isEmpty()) {
        return QIrWidgetUi::allWidgets;
    }
    foreach (QIrWidget* w, QIrWidgetUi::allWidgets) {
        if (w->widgetGroupKey().trimmed() == key) {
            list.append(list);
        }
    }
    return list;
}
QIrSubStyle* QIrWidget::subStyle() const
{
    if (ui_ptr->extraSubStyle) {
        return ui_ptr->extraSubStyle;
    }
    return subStyle(this);
}
void QIrWidget::setSubStyle(QIrSubStyle* style)
{
    QIrSubStyle* oldStyle = subStyle(), * newStyle;

    if (oldStyle == style) {
        return;
    }
    style->unpolish(toWidget());
    if (ui_ptr->extraSubStyle) {
        delete ui_ptr->extraSubStyle;
    }
    ui_ptr->extraSubStyle = style;
    if (style) {
        QWidget* thisWidget = toWidget();

        if (style->parent() != thisWidget) {
            style->setParent(thisWidget);
        }
    }
    newStyle = subStyle();
    if (newStyle) {
        QWidget* widget = toWidget();
        subStyleChanged();
        newStyle->polish(widget);
        widget->update();
    }
}
QIrSubStyle* QIrWidget::subStyle(const QString &widgetGroupKey)
{
    QString key = widgetGroupKey.trimmed();

    if (key.isEmpty()) {
        return 0;
    }
    if (!QIrWidgetUi::subStyles.contains(key)) {
        QIrWidgetUi::subStyles[key] = 0;
    }
    return QIrWidgetUi::subStyles[key];
}
QIrSubStyle* QIrWidget::subStyle(const QIrWidget* w)
{
    if (!w) {
        return 0;
    }
    return subStyle(w->widgetGroupKey());
}
void QIrWidget::setSubStyle(const QString &widgetGroupKey, QIrSubStyle* style)
{
    QString key;
    QIrWidgetList list;
    QIrSubStyle* oldStyle;

    if (!style) {
        return;
    }
    key = widgetGroupKey.trimmed();
    if (key.isEmpty()) {
        return;
    }
    oldStyle = subStyle(key);
    if (oldStyle == style) {
        return;
    }
    list = allWidgets(key);
    if (oldStyle) {
        foreach (QIrWidget* wid, list) {
            if (wid->subStyle() == oldStyle) {
                oldStyle->unpolish(wid->toWidget());
            }
        }
        delete oldStyle;
    }
    QIrWidgetUi::subStyles[key] = style;
    if (style) {
        if (style->parent() != qApp) {
            style->setParent(qApp);
        }
        foreach (QIrWidget* wid, list) {
            if (wid->subStyle() == style) {
                QWidget* widget = wid->toWidget();

                wid->subStyleChanged();
                style->polish(widget);
                widget->update();
            }
        }
    }
}
void QIrWidget::setSubStyle(QIrWidget* w, QIrSubStyle* style)
{
    QString key;
    QIrWidgetList list;
    QIrSubStyle* oldStyle;

    if (!w) {
        return;
    }
    key = w->widgetGroupKey().trimmed();
    if (key.isEmpty()) {
        return;
    }
    oldStyle = subStyle(key);
    if (oldStyle == style) {
        return;
    }
    list = allWidgets(key);
    if (oldStyle) {
        foreach (QIrWidget* wid, list) {
            if (wid->subStyle() == oldStyle) {
                oldStyle->unpolish(wid->toWidget());
            }
        }
        delete oldStyle;
    }
    QIrWidgetUi::subStyles[key] = style;
    if (!style) {
        style = w->defaultSubStyle();
    }
    if (style) {
        if (style->parent() != qApp) {
            style->setParent(qApp);
        }
        foreach (QIrWidget* wid, list) {
            if (wid->subStyle() == style) {
                QWidget* widget = wid->toWidget();

                wid->subStyleChanged();
                style->polish(widget);
                widget->update();
            }
        }
    }
}
void QIrWidget::subStyleChanged()
{
}

QIR_END_NAMESPACE
