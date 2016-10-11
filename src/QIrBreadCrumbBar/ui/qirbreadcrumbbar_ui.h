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
#ifndef QIRBREADCRUMBBAR_UI_H
#define QIRBREADCRUMBBAR_UI_H

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

#include <QComboBox>
#include "../qirabstractbreadcrumbmodel.h"
#include "../qirbreadcrumbbar.h"
#include "../QIronCommon/ui/qirwidget_ui.h"

class QAction;
class QLabel;

QIR_BEGIN_NAMESPACE

class QIrBreadCrumbBarStyle;
class QIrBreadCrumbComboBox;
class QIrBreadCrumbComboBoxContainer;
class QIrBreadCrumbLabel;

class QIrBreadCrumbItem
{
public:
    enum Type {EmptyArea, Indicator, Label};

    QIrBreadCrumbItem(QIrBreadCrumbComboBoxContainer*, QIrBreadCrumbItem::Type);
    virtual ~QIrBreadCrumbItem() {}

    inline QRect rect() const { return m_rect; }
    inline void setRect(const QRect &rect) { m_rect = rect; }

    inline void setVisible(bool visible) { m_visible = visible; }
    inline bool isVisible() { return m_visible; }

    inline QIrBreadCrumbComboBoxContainer* container() const { return m_container; }
    inline Type type() const { return m_type; }

    virtual void clicked(const QPoint &pos) = 0;

private:
    bool m_visible;
    QIrBreadCrumbItem::Type m_type;
    QIrBreadCrumbComboBoxContainer* m_container;
    QRect m_rect;
};

class QIrBreadCrumbEmptyArea : public QIrBreadCrumbItem
{
public:
    QIrBreadCrumbEmptyArea(QIrBreadCrumbComboBoxContainer*);
    ~QIrBreadCrumbEmptyArea() {}

    void clicked(const QPoint &pos);
};

class QIrBreadCrumbIndicator : public QIrBreadCrumbItem
{
public:
    QIrBreadCrumbIndicator(QIrBreadCrumbLabel*, QIrBreadCrumbComboBoxContainer*);
    virtual ~QIrBreadCrumbIndicator();

    inline bool isTruncated() const { return m_trunc; }
    inline void setTruncated(bool trunc) { m_trunc = trunc; }

    inline QIrBreadCrumbLabel* label() const { return m_label; }

    void clicked(const QPoint &pos);

private:
    bool m_trunc;
    QIrBreadCrumbLabel* m_label;
};

class QIrBreadCrumbLabel : public QIrBreadCrumbItem
{
public:
    QIrBreadCrumbLabel(QIrBreadCrumbIndicator*, QIrBreadCrumbComboBoxContainer*, const QIrBreadCrumbModelNode &);
    virtual ~QIrBreadCrumbLabel() { }

    inline QIrBreadCrumbModelNode node() const { return m_node; }

    inline QIrBreadCrumbIndicator* indicator() { return m_indicator; }
    inline void setIndicator(QIrBreadCrumbIndicator* indicator)  { m_indicator = indicator; }

    void clicked(const QPoint &pos);

private:
    QIrBreadCrumbModelNode m_node;
    QIrBreadCrumbIndicator* m_indicator;
};

class QIrBreadCrumbComboBoxContainer : public QWidget
{
    Q_OBJECT

public:
    QIrBreadCrumbComboBoxContainer(QIrBreadCrumbComboBox* comboBox);
    ~QIrBreadCrumbComboBoxContainer();

    inline QIrBreadCrumbComboBox* comboBox() const { return m_comboBox; }

    void splitPath(const QString &);

    inline QIrBreadCrumbModelNodeList nodeList() const { return m_nodeList; }
    void refresh();

    QString hiddenPath() const;

protected:
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void leaveEvent(QEvent*);

private:
    friend class QIrBreadCrumbComboBox;

    void clear();
    void clearAll();
    void updateGeometries();
    int itemAt(const QPoint &pos);

    QList< QIrBreadCrumbItem* > m_items;
    QIrBreadCrumbModelNodeList m_nodeList;
    QIrBreadCrumbIndicator* m_rootIndicator;
    QIrBreadCrumbEmptyArea* m_emptyArea;

    int m_hoverItem;
    int m_downItem;
    QIrBreadCrumbComboBox* m_comboBox;
    bool m_clicked;
};

class QIrBreadCrumbIconWidget : public QWidget
{
    Q_OBJECT

public:
    QIrBreadCrumbIconWidget(QIrBreadCrumbComboBox* comboBox);

    inline QPixmap pixmap() const { return m_pixmap; }
    inline void setPixmap(const QPixmap &p) { m_pixmap = p; update(); }

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);

private slots:
    void slotCopyAddress();
    void slotEditAddress();

private:
    QIrBreadCrumbComboBox* m_comboBox;
    QPixmap m_pixmap;
    bool m_clicked;
};

class QIrBreadCrumbComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit QIrBreadCrumbComboBox(QIrBreadCrumbBar* parent);
    ~QIrBreadCrumbComboBox();

    inline bool isFlat() const { return m_flat; }
    void setFlat(bool f);
    inline QIrBreadCrumbBar* bar() const { return m_bar; }
    inline QString location() const { return m_location; }
    void setLocation(const QString &);

    void showPopup();

    void showBreadCrumbs(bool popupError = true, bool changeLocation = true, bool forceShowContainer = false);
    void edit();

    inline QIrBreadCrumbComboBoxContainer* container() const { return m_container; }
    void updateGeometries();
private slots:
    void setDelayedLocation();
    void slotSetLocation(QAction*);
    void slotHandleEditTextChanged();
    void slotActivated();

protected:
    bool event(QEvent*);
    void resizeEvent(QResizeEvent*);
    void focusOutEvent(QFocusEvent*);

private:

    bool m_flat;
    QString m_location;
    QString m_delayedLocation;
    QIrBreadCrumbIconWidget* m_iconLabel;
    QIrBreadCrumbComboBoxContainer* m_container;
    QIrBreadCrumbBar* m_bar;
    bool m_clicked;

signals:
    void locationChanged(const QString &);
};

class QIrBreadCrumbBarUi : public QIrWidgetUi
{
    QIR_DECLARE_WIDGET(QIrBreadCrumbBar);

public:
    QIrBreadCrumbBarUi();
    ~QIrBreadCrumbBarUi();

    bool flat;
    bool editable;
    QIrBreadCrumbComboBox* comboBox;
    QIrAbstractBreadCrumbModel* model;

protected:
    void init();
};

QIR_END_NAMESPACE

#endif // QIRBREADCRUMBBAR_UI_H
