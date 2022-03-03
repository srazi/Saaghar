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
/*!
  \brief This file contains the definition of QIrBreadCrumbBar class.
  \file qirbreadcrumbbar.cpp
  \author Dzimi Mve Alexandre
  \date 07/25/2009
*/
#include <QAction>
#include <QStyle>
#include <QLabel>
#include <QMenu>
#include <QLineEdit>
#include <QPainter>
#include <QStyleOption>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QResizeEvent>
#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include <QCompleter>
#include <QDrag>
#include <QMimeData>
#include <QUrl>
#include <QTimer>

#include "qirbreadcrumbbar.h"
#include "qirbreadcrumbbarstyle.h"
#include "breadcrumbsaagharmodel.h"
#include "ui/qirbreadcrumbbar_ui.h"

#define ICON_SIZE 16

QIR_BEGIN_NAMESPACE

/////////////////////
//QIrBreadCrumbItem
/////////////////////
QIrBreadCrumbItem::QIrBreadCrumbItem(QIrBreadCrumbComboBoxContainer* container, QIrBreadCrumbItem::Type type)
    : m_visible(false), m_type(type), m_container(container)
{
}

/////////////////////////
//QIrBreadCrumbEmptyArea
/////////////////////////
QIrBreadCrumbEmptyArea::QIrBreadCrumbEmptyArea(QIrBreadCrumbComboBoxContainer* container)
    : QIrBreadCrumbItem(container, EmptyArea)
{
    setVisible(true);
}
void QIrBreadCrumbEmptyArea::clicked(const QPoint &)
{
    container()->comboBox()->edit();
}


/////////////////////////
//QIrBreadCrumbIndicator
/////////////////////////
QIrBreadCrumbIndicator::QIrBreadCrumbIndicator(QIrBreadCrumbLabel* label, QIrBreadCrumbComboBoxContainer* container)
    : QIrBreadCrumbItem(container, Indicator), m_trunc(false), m_label(label)
{
    if (!label) {
        setVisible(true);
    }
    else {
        label->setIndicator(this);
    }
}
QIrBreadCrumbIndicator::~QIrBreadCrumbIndicator()
{
}

void QIrBreadCrumbIndicator::clicked(const QPoint &)
{
    QIrBreadCrumbComboBoxContainer* cont = container();
    QIrBreadCrumbComboBox* comboBox = cont->comboBox();
    QIrAbstractBreadCrumbModel* model = comboBox->bar()->model();
    QIrBreadCrumbModelNode node(cont->hiddenPath(), QIrBreadCrumbModelNode::Global);

    QMenu* menu;

    menu = model->buildMenu(m_label ? m_label->node() : node);
    if (!menu || (menu && menu->actions().isEmpty())) {
        return;
    }
    menu->setStyleSheet(QLatin1String("QMenu { menu-scrollable: 1; }"));
    menu->connect(menu, SIGNAL(triggered(QAction*)), container()->comboBox(), SLOT(slotSetLocation(QAction*)));

    QPoint pos;

    if (cont->isRightToLeft()) {
        pos = cont->mapToGlobal(rect().bottomRight());
        pos.setX(pos.x() - menu->sizeHint().width());
    }
    else {
        pos = cont->mapToGlobal(rect().bottomLeft());
    }

    menu->exec(pos);
}

/////////////////////////
//QIrBreadCrumbLabel
/////////////////////////
QIrBreadCrumbLabel::QIrBreadCrumbLabel(QIrBreadCrumbIndicator* indicator, QIrBreadCrumbComboBoxContainer* container, const QIrBreadCrumbModelNode &node)
    : QIrBreadCrumbItem(container, Label),  m_node(node), m_indicator(indicator)
{
}
void QIrBreadCrumbLabel::clicked(const QPoint &)
{
    QIrBreadCrumbComboBoxContainer* cont = container();

    //To change
    cont->comboBox()->setLocation(m_node.path());
}


/////////////////////////////////
//QIrBreadCrumbComboBoxContainer
/////////////////////////////////
QIrBreadCrumbComboBoxContainer::QIrBreadCrumbComboBoxContainer(QIrBreadCrumbComboBox* comboBox)
    : QWidget(comboBox), m_hoverItem(-1), m_downItem(-1),  m_comboBox(comboBox), m_clicked(false)
{
    setMouseTracking(true);
    m_rootIndicator = new QIrBreadCrumbIndicator(0, this);
    m_emptyArea = new QIrBreadCrumbEmptyArea(this);

}
QIrBreadCrumbComboBoxContainer::~QIrBreadCrumbComboBoxContainer()
{
    clearAll();
}
void QIrBreadCrumbComboBoxContainer::splitPath(const QString &location)
{
    QIrAbstractBreadCrumbModel* model = m_comboBox->bar()->model();

    m_nodeList = model->splitPath(location);
    refresh();
}
void QIrBreadCrumbComboBoxContainer::refresh()
{
    QIrAbstractBreadCrumbModel* model = m_comboBox->bar()->model();
    QIrBreadCrumbModelNode node(QString(), QIrBreadCrumbModelNode::Global);
    QIrBreadCrumbLabel* item;
    QIrBreadCrumbIndicator* indic;

    clear();

    if (m_nodeList.isEmpty()) {
        return;
    }
    if (model->supportsMenuNavigation()) {
        m_items << m_rootIndicator;
    }

    for (int i = 0; i < m_nodeList.count(); i++) {
        node = m_nodeList[i];
        indic = 0;
        item = new QIrBreadCrumbLabel(0, this, node);
        m_items.append(item);
        if (node.type() != QIrBreadCrumbModelNode::Leaf && model->supportsMenuNavigation()) {
            m_items.append(indic = new QIrBreadCrumbIndicator(item, this));
        }
        if (i == m_nodeList.count() - 1) {
            item->setVisible(true);
            if (indic) {
                indic->setVisible(true);
            }
        }
    }
    m_items << m_emptyArea;
    updateGeometries();
    update();
}

QString QIrBreadCrumbComboBoxContainer::hiddenPath() const
{
    QString path;

    if (!m_rootIndicator->isTruncated()) {
        return path;
    }

    for (int i = m_items.count() - 1; i > 0; --i) {
        QIrBreadCrumbItem* item = m_items.at(i);

        if (item->type() == QIrBreadCrumbItem::Label && !item->isVisible()) {
            QIrBreadCrumbLabel* label = static_cast<QIrBreadCrumbLabel*>(item);
            path = label->node().path();
            break;
        }
    }

    return path;
}
void QIrBreadCrumbComboBoxContainer::mousePressEvent(QMouseEvent* evt)
{
    if (evt->button() == Qt::LeftButton) {
        m_downItem = itemAt(evt->pos());
        if (m_downItem != -1 && m_items[m_downItem]->type() == QIrBreadCrumbItem::Indicator) {
            m_clicked = true;
            update();
            m_items[m_downItem]->clicked(evt->pos());
            m_downItem = -1;
            m_clicked = false;
            update();
        }
        else {
            update();
        }
    }
}
void QIrBreadCrumbComboBoxContainer::mouseMoveEvent(QMouseEvent* evt)
{
    int hover = m_hoverItem;

    m_hoverItem = itemAt(evt->pos());
    if (hover != m_hoverItem) {
        update();
    }
}
void QIrBreadCrumbComboBoxContainer::mouseReleaseEvent(QMouseEvent* evt)
{
    if (evt->button() == Qt::LeftButton) {
        if (m_downItem != -1 && m_downItem == m_hoverItem &&  m_items[m_downItem]->type() != QIrBreadCrumbItem::Indicator) {
            m_items[m_downItem]->clicked(evt->pos());
        }
        m_downItem = -1;
        update();
    }
}
void QIrBreadCrumbComboBoxContainer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QIrBreadCrumbBar* bar =  m_comboBox->bar();
    QIrBreadCrumbIndicator* indicator;
    QIrBreadCrumbLabel* label;
    QIrAbstractBreadCrumbModel* model = bar->model();
    QIrSubStyle* style = bar->subStyle();
    QIrBreadCrumbItem* item;
    QStyleOptionComboBox option;

    QIrStyleOptionBreadCrumbIndicator incOption;
    QIrStyleOptionBreadCrumbLabel labelOption;
    QStyle::State state;
    int nextIndex, previousIndex;

    option.initFrom(m_comboBox);
    incOption.initFrom(m_comboBox);
    labelOption.initFrom(m_comboBox);

    if (bar->isEditable()) {
        option.rect = style->subControlRect(QStyle::CC_ComboBox, &option, (QStyle::SubControl)QIrBreadCrumbBarStyle::SC_BreadCrumbEditField, bar);
        option.rect.moveTopLeft(mapFromParent(option.rect.topLeft()));
        style->drawPrimitive((QStyle::PrimitiveElement)QIrBreadCrumbBarStyle::PE_BreadCrumbContainerBase, &option, &painter, bar);
    }
    for (int i = 0; i < m_items.count(); i++) {
        state = isEnabled() ? QStyle::State_Enabled : QStyle::State_None;
        item = m_items[i];
        if (item->isVisible()) {
            if (item->type() == QIrBreadCrumbItem::Indicator) {
                previousIndex = i - 1;
                indicator = static_cast< QIrBreadCrumbIndicator* >(item);
                incOption.rect = indicator->rect();
                incOption.hasLabel = (indicator->label() != 0);
                incOption.isTruncated = indicator->isTruncated();
                incOption.usePseudoState = false;
                if (incOption.hasLabel) {
                    incOption.isValid = model->isValid(indicator->label()->node().path());
                }
                if ((m_downItem == i && m_hoverItem == i) || (m_downItem == i && m_clicked)) {
                    state |= QStyle::State_Sunken;
                }
                else if (m_downItem == -1 && i == m_hoverItem) {
                    state |= QStyle::State_MouseOver;
                }
                else if (incOption.hasLabel) {
                    if (m_downItem == previousIndex && m_hoverItem == previousIndex) {
                        incOption.usePseudoState = true;
                        state |= QStyle::State_Sunken;
                    }
                    else if (m_downItem == -1 && m_hoverItem == previousIndex) {
                        incOption.usePseudoState = true;
                        state |= QStyle::State_MouseOver;
                    }
                }
                incOption.state = state;
                style->drawControl((QStyle::ControlElement)QIrBreadCrumbBarStyle::CE_BreadCrumbIndicator, &incOption, &painter, bar);
            }
            else if (item->type() == QIrBreadCrumbItem::Label) {
                nextIndex = i + 1;
                label = static_cast< QIrBreadCrumbLabel* >(item);
                labelOption.rect = label->rect();
                labelOption.hasIndicator = (label->indicator() != 0);
                labelOption.text = label->node().label();
                labelOption.usePseudoState = false;
                labelOption.isValid = model->isValid(label->node().path());
                if (m_downItem == i && m_hoverItem == i) {
                    state |= QStyle::State_Sunken;
                }
                else if (m_downItem == -1 && m_hoverItem == i) {
                    state |= QStyle::State_MouseOver;
                }
                else if (labelOption.hasIndicator) {
                    if ((m_downItem == nextIndex && m_hoverItem == nextIndex) || (m_downItem == nextIndex && m_clicked)) {
                        labelOption.usePseudoState = true;
                        state |= QStyle::State_Sunken;
                    }
                    else if (m_downItem == -1 && m_hoverItem == nextIndex) {
                        labelOption.usePseudoState = true;
                        state |= QStyle::State_MouseOver;
                    }
                }
                labelOption.state = state;
                style->drawControl((QStyle::ControlElement)QIrBreadCrumbBarStyle::CE_BreadCrumbLabel, &labelOption, &painter, bar);
            }
            else {
                option.state = state;
                option.rect = item->rect();
                style->drawControl((QStyle::ControlElement)QIrBreadCrumbBarStyle::CE_BreadCrumbEmptyArea, &option, &painter, bar);
            }
        }
    }
}
void QIrBreadCrumbComboBoxContainer::resizeEvent(QResizeEvent*)
{
    updateGeometries();
}
void QIrBreadCrumbComboBoxContainer::leaveEvent(QEvent*)
{
    m_hoverItem = -1;
    update();
}
void QIrBreadCrumbComboBoxContainer::clear()
{
    if (m_items.isEmpty()) {
        return;
    }
    m_items.removeFirst();
    m_items.removeLast();
    while (!m_items.isEmpty()) {
        delete m_items.takeFirst();
    }
}
void QIrBreadCrumbComboBoxContainer::clearAll()
{
    while (!m_items.isEmpty()) {
        delete m_items.takeFirst();
    }
}

void QIrBreadCrumbComboBoxContainer::updateGeometries()
{
    QIrBreadCrumbBar* bar = m_comboBox->bar();
    QIrSubStyle* style = bar->subStyle();
    QIrStyleOptionBreadCrumbIndicator arrowOption;
    QIrStyleOptionBreadCrumbLabel labelOption;
    QStyleOption emptyAreaOption;
    QIrAbstractBreadCrumbModel* model = bar->model();
    QRect arrowRect, labelRect, tempRect, emptyAreaRect;
    int lastLabelWidth,
        incWidth = 0,
        accWidth = 0,
        tempWidth,
        remainingWidth = 0,
        lastLabelIndex = m_items.count() - 2,
        i;
    bool trunc = false;
    QPoint startPoint;
    QIrBreadCrumbItem* item;
    QIrBreadCrumbIndicator* indic;
    QIrBreadCrumbLabel* label;

    const QRect containerRect = rect();
    QRect remainingRect = containerRect;

    if (m_items.isEmpty()) {
        return;
    }

    arrowOption.rect = containerRect;

    if (model->supportsMenuNavigation()) {
        arrowRect = style->subElementRect((QStyle::SubElement)QIrBreadCrumbBarStyle::SE_BreadCrumbIndicator, &arrowOption, bar);
    }
    if (m_items[lastLabelIndex]->type() == QIrBreadCrumbItem::Indicator) {
        incWidth = arrowRect.width();
        lastLabelIndex = m_items.count() - 3;
        m_items[lastLabelIndex - 1]->setVisible(true);
    }
    labelOption.text = static_cast< QIrBreadCrumbLabel* >(m_items[lastLabelIndex])->node().label();
    labelOption.rect = containerRect;
    labelRect = style->subElementRect((QStyle::SubElement)QIrBreadCrumbBarStyle::SE_BreadCrumbLabel, &labelOption, bar);
    lastLabelWidth = labelRect.width();
    emptyAreaOption.initFrom(this);
    emptyAreaRect = style->subElementRect((QStyle::SubElement)QIrBreadCrumbBarStyle::SE_BreadCrumbEmptyArea, &emptyAreaOption, bar);
    remainingRect.setWidth(remainingRect.width() - emptyAreaRect.width() - (arrowRect.width() + incWidth) - lastLabelWidth);
    m_items[lastLabelIndex]->setVisible(true);
    if (model->supportsMenuNavigation()) {
        for (i = lastLabelIndex - 1; i > 1; i -= 2) {
            item = m_items[i];
            indic = static_cast< QIrBreadCrumbIndicator* >(item);
            label = indic->label();
            indic->setVisible(false);
            label->setVisible(false);
            labelOption.text = label->node().label();
            labelRect = style->subElementRect((QStyle::SubElement)QIrBreadCrumbBarStyle::SE_BreadCrumbLabel, &labelOption, bar);
            if (!trunc) {
                tempWidth = arrowRect.width() + labelRect.width();
                if (remainingRect.width() < (accWidth + tempWidth)) {
                    trunc = true;
                }
                else {
                    indic->setVisible(true);
                    label->setVisible(true);
                    accWidth += tempWidth;
                }
            }
        }
    }
    else {
        for (i = lastLabelIndex - 1; i >= 0; i--) {
            item = m_items[i];
            label = static_cast< QIrBreadCrumbLabel* >(item);
            label->setVisible(false);
            labelOption.text = label->node().label();
            labelRect = style->subElementRect((QStyle::SubElement)QIrBreadCrumbBarStyle::SE_BreadCrumbLabel, &labelOption, bar);
            if (!trunc) {
                tempWidth = labelRect.width();
                if (remainingRect.width() < (accWidth + tempWidth)) {
                    trunc = true;
                }
                else {
                    label->setVisible(true);
                    accWidth += tempWidth;
                }
            }
        }
    }

    remainingWidth = remainingRect.width() - accWidth + emptyAreaRect.width();

    m_rootIndicator->setTruncated(trunc);
    foreach (QIrBreadCrumbItem* item, m_items) {
        if (item->isVisible()) {
            if (item == m_emptyArea)  {
                tempRect = emptyAreaRect;
                tempRect.setWidth(remainingWidth);
            }
            else if (item->type() == QIrBreadCrumbItem::Label) {
                labelOption.text = static_cast< QIrBreadCrumbLabel* >(item)->node().label();
                tempRect = style->subElementRect((QStyle::SubElement)QIrBreadCrumbBarStyle::SE_BreadCrumbLabel, &labelOption, bar);
            }
            else {
                tempRect = arrowRect;
            }

            tempRect.moveTo(startPoint);

            item->setRect(QStyle::visualRect(layoutDirection(), containerRect, tempRect));
            startPoint.setX(startPoint.x() + tempRect.width());
        }
    }
}
int QIrBreadCrumbComboBoxContainer::itemAt(const QPoint &pos)
{
    for (int i = 0; i < m_items.count(); i++)
        if (m_items[i]->isVisible())
            if (m_items[i]->rect().contains(pos)) {
                return i;
            }
    return -1;
}

////////////////////////
//QIrBreadCrumbIconWidget
//////////////////////
QIrBreadCrumbIconWidget::QIrBreadCrumbIconWidget(QIrBreadCrumbComboBox* comboBox) : QWidget(comboBox), m_comboBox(comboBox), m_clicked(false)
{
    QAction* action;

    setContextMenuPolicy(Qt::ActionsContextMenu);
    addAction(action = new QAction(tr("Copy Address"), this));
    connect(action, SIGNAL(triggered()), this, SLOT(slotCopyAddress()));
    addAction(action = new QAction(tr("Edit Address"), this));
    connect(action, SIGNAL(triggered()), this, SLOT(slotEditAddress()));
}
void QIrBreadCrumbIconWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if (m_pixmap.isNull()) {
        return;
    }
    QIcon icon(m_pixmap);

    QRect r(QPoint(), m_pixmap.size());

    if (m_comboBox->bar()->isEditable()) {
        painter.fillRect(rect(), m_comboBox->palette().base().color());
    }

    r.moveCenter(rect().center());

    painter.drawPixmap(r, icon.pixmap(m_pixmap.size(), isEnabled() ? QIcon::Normal : QIcon::Disabled));
}

void QIrBreadCrumbIconWidget::mousePressEvent(QMouseEvent* evt)
{
    if (evt->button() == Qt::LeftButton) {
        m_comboBox->edit();
    }
}
void QIrBreadCrumbIconWidget::slotCopyAddress()
{
    qApp->clipboard()->setText(m_comboBox->currentText());
}
void QIrBreadCrumbIconWidget::slotEditAddress()
{
    m_comboBox->edit();
}

/////////////////////////////////
//QIrBreadCrumbComboBox
/////////////////////////////////
QIrBreadCrumbComboBox::QIrBreadCrumbComboBox(QIrBreadCrumbBar* parent) : QComboBox(parent),
    m_flat(false), m_bar(parent), m_clicked(false)
{
    QPalette pal = palette();
    QCompleter* c;

    setObjectName("Location Bar");
    setDuplicatesEnabled(false);
    c = new QCompleter(this);
    setEditable(true);
    setCompleter(c);
    m_iconLabel = new QIrBreadCrumbIconWidget(this);
    pal.setBrush(QPalette::Normal, QPalette::Window, pal.brush(QPalette::Normal, QPalette::Base));
    pal.setBrush(QPalette::Disabled, QPalette::Window, pal.brush(QPalette::Disabled, QPalette::Base));
    pal.setBrush(QPalette::Inactive, QPalette::Window, pal.brush(QPalette::Inactive, QPalette::Base));
    m_iconLabel->setPalette(pal);
    connect(lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotHandleEditTextChanged()));
    m_container = new QIrBreadCrumbComboBoxContainer(this);
    m_container->setAutoFillBackground(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotActivated()));
}
QIrBreadCrumbComboBox::~QIrBreadCrumbComboBox()
{
}
bool QIrBreadCrumbComboBox::event(QEvent* e)
{
    if (e->type() == QEvent::Paint) {
        if (!m_bar->isEditable()) {
            return true;
        }
    }
    else if (e->type() == QEvent::ContextMenu)
        if (lineEdit()->isHidden()) {
            return true;
        }
    return QComboBox::event(e);

}

void QIrBreadCrumbComboBox::setLocation(const QString &location)
{
    QIrAbstractBreadCrumbModel* model = m_bar->model();
    QString tempLocation = location;

    if (tempLocation.isEmpty()) {
        tempLocation = model->defaultPath();
    }
    else if (!model->isValid(tempLocation)) {
        return;
    }

    tempLocation = model->cleanPath(tempLocation);

    m_location = tempLocation;

    setEditText(m_location);
    m_container->splitPath(m_location);

    if (!m_container->m_nodeList.isEmpty()) {
        m_iconLabel->setPixmap(model->icon(m_container->m_nodeList.last()).pixmap(ICON_SIZE, ICON_SIZE));
    }

    emit locationChanged(m_location);
}
void QIrBreadCrumbComboBox::showPopup()
{
    if (m_flat) {
        return;
    }
    edit();
    QComboBox::showPopup();
}

void QIrBreadCrumbComboBox::showBreadCrumbs(bool popupError, bool changeLocation, bool forceShowContainer)
{
    QString text = currentText();
    QIrAbstractBreadCrumbModel* model = m_bar->model();

    if (!m_flat) {
        if (text.isEmpty()) {
            return;
        }

        if (!model->isValid(text)) {
            if (popupError) {
                QString caption =  qApp->applicationName();

                if (!caption.isEmpty()) {
                    caption += " - ";
                }
                caption += objectName();
                QMessageBox::critical(this, caption, tr("Location '%1' cannot be found. Check the spelling and try again.").arg(text));
                lineEdit()->selectAll();
                setFocus(Qt::OtherFocusReason);
            }
            else {
                if (forceShowContainer) {
                    lineEdit()->hide();
                    m_container->show();
                }

                return;
            }
        }
        else {
            if (changeLocation) {
                setLocation(model->cleanPath(text));
            }
            lineEdit()->hide();
            m_container->show();
        }
    }
    else {
        if (text.isEmpty() || !model->isValid(text)) {
            if (changeLocation) {
                setLocation(model->defaultPath());
            }
        }
        else {
            if (changeLocation) {
                setLocation(model->cleanPath(text));
            }
        }
        lineEdit()->hide();
        m_container->show();
    }
}
void QIrBreadCrumbComboBox::setFlat(bool f)
{
    if (m_flat == f) {
        return;
    }
    m_flat = f;
    m_iconLabel->actions()[1]->setEnabled(!f);
    showBreadCrumbs(false);
    updateGeometries();
    update();
}
void QIrBreadCrumbComboBox::edit()
{
    QLineEdit* lineEdit = this->lineEdit();

    if (m_flat) {
        return;
    }
    m_container->hide();
    lineEdit->show();
    lineEdit->selectAll();
}

void QIrBreadCrumbComboBox::slotSetLocation(QAction* action)
{
    if (action->data().isValid()) {
        m_delayedLocation = action->data().toString();
        QTimer::singleShot(0, this, SLOT(setDelayedLocation()));
    }
}

void QIrBreadCrumbComboBox::slotHandleEditTextChanged()
{
    updateGeometries();
    showBreadCrumbs();
}
void QIrBreadCrumbComboBox::slotActivated()
{
    showBreadCrumbs(false);
}
void QIrBreadCrumbComboBox::resizeEvent(QResizeEvent* evt)
{
    QComboBox::resizeEvent(evt);
    updateGeometries();
}
void QIrBreadCrumbComboBox::focusOutEvent(QFocusEvent* evt)
{
    QWidget* focus = qApp->focusWidget();

    if (focus && focus != this && evt->reason() != Qt::PopupFocusReason) {
        showBreadCrumbs(false, false, true);
    }
    QComboBox::focusOutEvent(evt);
}
void QIrBreadCrumbComboBox::updateGeometries()
{
    QStyleOptionComboBox option;
    QLineEdit* lineEdit = this->lineEdit();
    QIrSubStyle* style = m_bar->subStyle();

    initStyleOption(&option);
    m_iconLabel->setGeometry(QStyle::visualRect(layoutDirection(), m_bar->rect(), style->subControlRect(QStyle::CC_ComboBox, &option, (QStyle::SubControl)QIrBreadCrumbBarStyle::SC_BreadCrumbIcon, m_bar)));
    if (!m_flat && lineEdit) {
        // a Qt element and it is reversed within RTL layouts
        lineEdit->setGeometry(QStyle::visualRect(layoutDirection(), m_bar->rect(), style->subControlRect(QStyle::CC_ComboBox, &option, (QStyle::SubControl)QIrBreadCrumbBarStyle::SC_BreadCrumbEditField, m_bar)));
    }
    m_container->setGeometry(QStyle::visualRect(layoutDirection(), m_bar->rect(), style->subControlRect(QStyle::CC_ComboBox, &option, (QStyle::SubControl)QIrBreadCrumbBarStyle::SC_BreadCrumbContainer, m_bar)));
}

void QIrBreadCrumbComboBox::setDelayedLocation()
{
    setLocation(m_delayedLocation);
    m_delayedLocation.clear();
}


//////////////////////////
// QIrBreadCrumbBarUi
//////////////////////////
QIrBreadCrumbBarUi::QIrBreadCrumbBarUi() : flat(false), editable(true),  comboBox(0), model(0)
{
}
QIrBreadCrumbBarUi::~QIrBreadCrumbBarUi()
{
}
void QIrBreadCrumbBarUi::init()
{
    QIR_W(QIrBreadCrumbBar);

    comboBox = new QIrBreadCrumbComboBox(w);
    w->setModel(new QIrDefaultBreadCrumbModel);
    w->setLocation(QString());
    comboBox->showBreadCrumbs(false);
    w->setSizePolicy(comboBox->sizePolicy());
}

////////////////////
// QIrBreadCrumbBar
////////////////////
QIrBreadCrumbBar::QIrBreadCrumbBar(QWidget* parent) : QWidget(parent, Qt::WindowFlags(0)), QIrWidget(* new QIrBreadCrumbBarUi)
{
    QIR_UI(QIrBreadCrumbBar);

    ui->setupUi();

    connect(ui->comboBox, SIGNAL(locationChanged(QString)), this, SLOT(onComboboxLocationChanged(QString)));
}
QIrBreadCrumbBar::~QIrBreadCrumbBar()
{
}
QComboBox* QIrBreadCrumbBar::comboBox() const
{
    QIR_UI(const QIrBreadCrumbBar);

    return ui->comboBox;
}
bool QIrBreadCrumbBar::isEditable() const
{
    return ui_func()->editable;
}
void QIrBreadCrumbBar::setEditable(bool e)
{
    QIR_UI(QIrBreadCrumbBar);

    ui->editable = e;
    ui->comboBox->setFlat(!e);
}
QString QIrBreadCrumbBar::location() const
{
    QIR_UI(const QIrBreadCrumbBar);

    return ui->comboBox->location();
}
QSize QIrBreadCrumbBar::sizeHint() const
{
    QIR_UI(const QIrBreadCrumbBar);
    QSize size = ui->comboBox->sizeHint();
    QStyleOptionComboBox option;
    QIrSubStyle* style = subStyle();

    option.initFrom(ui->comboBox);
    size = QSize(size.width() + style->subElementRect((QStyle::SubElement)QIrBreadCrumbBarStyle::SE_BreadCrumbEmptyArea, &option, this).width(), size.height());
    return size;
}
void QIrBreadCrumbBar::setLocation(const QString &location)
{
    QIR_UI(QIrBreadCrumbBar);

    ui->comboBox->setLocation(location);
}
QIrAbstractBreadCrumbModel* QIrBreadCrumbBar::model() const
{
    QIR_UI(const QIrBreadCrumbBar);

    return ui->model;
}
void QIrBreadCrumbBar::setModel(QIrAbstractBreadCrumbModel* model)
{
    QIR_UI(QIrBreadCrumbBar);
    QAbstractItemModel* itemModel;

    if (!model) {
        return;
    }
    if (ui->model != model) {
        ui->model = model;
        if (ui->comboBox && (itemModel = model->itemModel())) {
            ui->comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
            ui->comboBox->completer()->setCaseSensitivity(Qt::CaseInsensitive);
            ui->comboBox->completer()->setCompletionRole(Qt::DisplayRole);
            ui->comboBox->completer()->setModel(itemModel);
        }
        ui->comboBox->container()->refresh();
    }
}
void QIrBreadCrumbBar::refresh()
{
    QIR_UI(QIrBreadCrumbBar);

    ui->comboBox->updateGeometries();
    ui->comboBox->container()->refresh();
}
void QIrBreadCrumbBar::edit()
{
    QIR_UI(QIrBreadCrumbBar);

    ui->comboBox->edit();
}
void QIrBreadCrumbBar::goToDefault()
{
    QIR_UI(QIrBreadCrumbBar);

    setLocation(ui->model->defaultPath());
}

void QIrBreadCrumbBar::onComboboxLocationChanged(const QString &location)
{
    emit locationChanged(location);
}

QIrSubStyle* QIrBreadCrumbBar::defaultSubStyle() const
{
    return new QIrDefaultBreadCrumbBarStyle;
}
void QIrBreadCrumbBar::resizeEvent(QResizeEvent*)
{
    QIR_UI(QIrBreadCrumbBar);

    ui->comboBox->setGeometry(QRect(QPoint(), size()));
}
void QIrBreadCrumbBar::paintEvent(QPaintEvent*)
{
    QIR_UI(QIrBreadCrumbBar);
    QPainter painter(this);
    QStyleOption option;

    if (!ui->editable) {
        option.initFrom(this);
        subStyle()->drawPrimitive((QStyle::PrimitiveElement)QIrBreadCrumbBarStyle::PE_FrameBreadCrumbBar, &option, &painter, this);
    }
}

QIR_END_NAMESPACE
