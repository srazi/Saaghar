/***************************************************************************
 *  This file is part of QMultiSelectWidget, a custom Qt widget            *
 *                                                                         *
 *  Copyright (C) 2012-2015 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 *  There is some code from qtcenter forum & QxtCheckComboBox.from libqxt..*
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

#include "qmultiselectwidget.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QKeyEvent>

const int LAST_CHECK_STATE = Qt::UserRole + 10;

CustomComboBox::CustomComboBox(QWidget* parent, QListWidget* viewWidget, QObject* filterObject) : QComboBox(parent)
{
    containerMousePress = false;

    if (viewWidget && viewWidget->model()) {
        setModel(viewWidget->model());
        setView(viewWidget);
    }

    QLineEdit* lineEdit = new QLineEdit(this);

    lineEdit->setReadOnly(true);
    setLineEdit(lineEdit);
    lineEdit->disconnect(this);

    setSizeAdjustPolicy(QComboBox::AdjustToContents);

    setInsertPolicy(QComboBox::NoInsert);

    installEventFilter(filterObject);
    view()->installEventFilter(filterObject);
    view()->window()->installEventFilter(filterObject);
    view()->viewport()->installEventFilter(filterObject);
}

CustomComboBox::~CustomComboBox()
{
}

void CustomComboBox::wheelEvent(QWheelEvent* e)
{
    e->ignore();
}

void CustomComboBox::hidePopup()
{
    if (containerMousePress) {
        QComboBox::hidePopup();
    }
}

QMultiSelectWidget::QMultiSelectWidget(QWidget* parent, QMultiSelectWidget::WidgetType type)
    : QListWidget(parent)
    , comboWidget(0)
{
    separator = " - ";

    overCheckBox = false;
    if (type == QMultiSelectWidget::ComboBoxView) {
        comboWidget = new CustomComboBox(parent, this, this);
        comboWidget->view()->setTextElideMode(Qt::ElideMiddle);
        comboWidget->view()->window()->setMaximumHeight(300);
    }
    else {
        comboWidget = 0;
    }

    connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemDataChanged(QListWidgetItem*)));
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(clickedOnItem(QListWidgetItem*)));

    updateSelectedLists();
}

QMultiSelectWidget::~QMultiSelectWidget()
{
}

// returns the selected item whose checkbox lies under 'pos'
// returns 0 if not selected, no item at pos, or does not fit inside checkbox
bool QMultiSelectWidget::isCheckBoxUnderPosition(const QPoint &pos)
{
    QListWidgetItem* item = itemAt(pos);
    if (item) {
        // with the help of styles, check if checkbox rect contains 'pos'
        QStyleOptionButton opt;
        opt.QStyleOption::operator=(viewOptions());
        opt.rect = visualItemRect(item);
        QRect r = style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt);
        // assure that the item is also selected
        if (r.contains(pos)) {
            return true;
        }
    }
    return false;
}

void QMultiSelectWidget::disableOtherItems(QListWidgetItem* excludedItem)
{
    for (int i = 0; i < count(); ++i) {
        if (item(i) != excludedItem) {
            item(i)->setFlags(item(i)->flags() ^ Qt::ItemIsEnabled);
        }
    }
}

void QMultiSelectWidget::enableOtherItems(QListWidgetItem* excludedItem)
{
    for (int i = 0; i < count(); ++i) {
        if (item(i) != excludedItem) {
            item(i)->setFlags(item(i)->flags() | Qt::ItemIsEnabled);
        }
    }
}

void QMultiSelectWidget::itemDataChanged(QListWidgetItem* item)
{
    disconnect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemDataChanged(QListWidgetItem*)));
    if (item->checkState() != item->data(LAST_CHECK_STATE)) {
        item->setData(LAST_CHECK_STATE, item->checkState());
        checkStateChanged(item);
        emit itemCheckStateChanged(item);
    }
    connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemDataChanged(QListWidgetItem*)));
}

void QMultiSelectWidget::checkStateChanged(QListWidgetItem* item)
{
    if (noMultiSelectItems.contains(item)) {
        if (item->checkState() == Qt::Checked) {
            disableOtherItems(item);
        }
        else {
            enableOtherItems(item);
            if (comboWidget) {
                comboWidget->setCurrentIndex(-1);
                comboWidget->lineEdit()->setText("");
            }
        }
    }
    else {
        QListWidgetItem* parentItem = parentChildHash.value(item);

        /***
        //compute check state of parent and children
        ***/
        //if item is a child
        if (parentItem) {
            QList<QListWidgetItem*> childrenList = parentChildHash.keys(parentItem);
            int numOfChecked = 0;
            for (int i = 0; i < childrenList.size(); ++i) {
                if (childrenList.at(i)->checkState() == Qt::Checked) {
                    ++numOfChecked;
                }
            }
            if (numOfChecked == 0) {
                parentItem->setCheckState(Qt::Unchecked);
                parentItem->setData(LAST_CHECK_STATE, Qt::Unchecked);
            }
            else if (numOfChecked < childrenList.size()) {
                parentItem->setCheckState(Qt::PartiallyChecked);
                parentItem->setData(LAST_CHECK_STATE, Qt::PartiallyChecked);
            }
            else if (numOfChecked == childrenList.size()) {
                parentItem->setCheckState(Qt::Checked);
                parentItem->setData(LAST_CHECK_STATE, Qt::Checked);
            }
        }

        //if item is a parent
        QList<QListWidgetItem*> childrenList = parentChildHash.keys(item);
        for (int i = 0; i < childrenList.size(); ++i) {
            childrenList.at(i)->setCheckState(item->checkState());
            childrenList.at(i)->setData(LAST_CHECK_STATE, item->checkState());
        }
    }

    updateSelectedLists();
}


void QMultiSelectWidget::setSeparator(const QString &sep)
{
    separator = sep;
    updateToolTips();
}

QList<QListWidgetItem*> QMultiSelectWidget::getSelectedItemList()
{
    return selectedItemsList;
}

QStringList QMultiSelectWidget::getSelectedStringList()
{
    return selectedList;
}

QString QMultiSelectWidget::getSelectedString()
{
    return selectedStr;
}

void QMultiSelectWidget::updateSelectedLists()
{
    selectedList.clear();
    selectedItemsList.clear();

    for (int i = 0; i < count(); ++i) {
        QListWidgetItem* iItem = item(i);

        if ((iItem->flags() & Qt::ItemIsEnabled) && iItem->checkState() == Qt::Checked) {
            QListWidgetItem* parentItem = parentChildHash.value(iItem);
            if (!(parentItem && parentItem->checkState() == Qt::Checked)) {
                selectedList << iItem->text();
                selectedItemsList << iItem;
            }
        }
    }

    updateToolTips();
}

void QMultiSelectWidget::updateToolTips()
{
    if (selectedList.isEmpty()) {
        selectedStr = tr("Selected Items: no item!");
    }
    else {
        selectedStr = tr("Selected Items:< /br>") + selectedList.join(separator);
    }

    QString dirStr = "ltr";
    if (layoutDirection() == Qt::RightToLeft) {
        dirStr = "rtl";
    }

    setToolTip(QString("<p dir=\'%1\'>").arg(dirStr) + selectedStr + "</p>");
    if (comboWidget) {
        comboWidget->setToolTip(QString("<p dir=\'%1\'>").arg(dirStr) + selectedStr + "</p>");
        comboWidget->lineEdit()->setText(selectedList.join("-"));
    }
}

void QMultiSelectWidget::clickedOnItem(QListWidgetItem* item)
{

    if ((item->flags() & Qt::ItemIsUserCheckable) && (item->flags() & Qt::ItemIsEnabled) && !overCheckBox) {
        item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
    }
    else if (!(item->flags() & Qt::ItemIsUserCheckable)  && (item->flags() & Qt::ItemIsEnabled)) {
        setCurrentItem(item);
        if (comboWidget) {
            comboWidget->QComboBox::hidePopup();
        }
    }
}

QListWidgetItem* QMultiSelectWidget::insertRow(int row, const QString &label, bool checkable,
        const QString &data, Qt::ItemDataRole role, bool monoSelection, QListWidgetItem* parent)
{
    QListWidgetItem* newItem = new QListWidgetItem(label);

    if (checkable) {
        newItem->setCheckState(Qt::Unchecked);
        newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        newItem->setData(LAST_CHECK_STATE, newItem->checkState());
    }
    else {
        newItem->setFlags(Qt::ItemIsEnabled);
    }

    if (monoSelection) {
        insertNoMultiSelectItem(newItem);
    }

    if (parent && parent != newItem) {
        parentChildHash.insert(newItem, parent);
    }

    if (!data.isEmpty()) {
        newItem->setData(role, data);
    }

    insertItem(row, newItem);

    return newItem;
}

void QMultiSelectWidget::insertNoMultiSelectItem(QListWidgetItem* item)
{
    if (item) {
        noMultiSelectItems << item;
    }
}

QComboBox* QMultiSelectWidget::getComboWidgetInstance()
{
    //maybe 0
    return comboWidget;
}

bool QMultiSelectWidget::eventFilter(QObject* receiver, QEvent* event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (receiver == comboWidget && (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)) {
            comboWidget->showPopup();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Enter ||
                 keyEvent->key() == Qt::Key_Return ||
                 keyEvent->key() == Qt::Key_Escape) {
            // it is important to call QComboBox implementation
            comboWidget->QComboBox::hidePopup();
            if (keyEvent->key() != Qt::Key_Escape) {
                return true;
            }
        }
    }
    case QEvent::MouseButtonPress: {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        overCheckBox = isCheckBoxUnderPosition(mouseEvent->pos());
        comboWidget->containerMousePress = (receiver == window());
        break;
    }
    case QEvent::MouseButtonRelease:
        comboWidget->containerMousePress = false;
        return false;
        break;
    default:
        break;
    }
    return false;
}

void QMultiSelectWidget::clear()
{
    parentChildHash.clear();
    selectedList.clear();
    selectedItemsList.clear();
    noMultiSelectItems.clear();
    QListWidget::clear();
    if (comboWidget) {
        comboWidget->clear();
    }
    updateToolTips();
}
