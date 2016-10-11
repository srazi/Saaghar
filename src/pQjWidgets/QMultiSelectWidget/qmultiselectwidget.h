/***************************************************************************
 *  This file is part of QMultiSelectWidget, a custom Qt widget            *
 *                                                                         *
 *  Copyright (C) 2012-2016 by S. Razi Alavizadeh                          *
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

#ifndef QMULTISELECTWIDGET_H
#define QMULTISELECTWIDGET_H

#include <QHash>
#include <QComboBox>
#include <QListWidget>

class CustomComboBox : public QComboBox
{
    Q_OBJECT

public:
    CustomComboBox(QWidget* parent = 0, QListWidget* viewWidget = 0, QObject* filterObject = 0);
    ~CustomComboBox();

    void hidePopup();

    bool containerMousePress;

protected:
    virtual void wheelEvent(QWheelEvent* e);
};

class QMultiSelectWidget : public QListWidget
{
    Q_OBJECT

public:
    enum WidgetType {
        ComboBoxView,
        ListView
    };
    explicit QMultiSelectWidget(QWidget* parent = 0, QMultiSelectWidget::WidgetType type = QMultiSelectWidget::ComboBoxView);
    virtual ~QMultiSelectWidget();

    //QWidget *getWidgetInstance();
    QComboBox* getComboWidgetInstance();

    void insertNoMultiSelectItem(QListWidgetItem* item);
    void setSeparator(const QString &sep);
    void updateSelectedLists();

    QList<QListWidgetItem*> getSelectedItemList();
    QStringList getSelectedStringList();
    QString getSelectedString();
    //void insertPoets(const QList<GanjoorPoet *> &poets);
    QListWidgetItem* insertRow(int row, const QString &label = "", bool checkable = true,
                               const QString &data = "", Qt::ItemDataRole role = Qt::UserRole,
                               bool monoSelection = false, QListWidgetItem* parent = 0);

public slots:
    void clear();

private:
    void disableOtherItems(QListWidgetItem* excludedItem);
    void enableOtherItems(QListWidgetItem* excludedItem);
    bool overCheckBox;
    bool isCheckBoxUnderPosition(const QPoint &pos);
    bool eventFilter(QObject* receiver, QEvent* event);
    QListWidget* listWidget;
    CustomComboBox* comboWidget;
    QWidget* containerWidget;
    QHash<QListWidgetItem*, QListWidgetItem*> parentChildHash;
    QList<QListWidgetItem*> noMultiSelectItems;
    QString selectedStr;
    QStringList selectedList;
    QList<QListWidgetItem*> selectedItemsList;
    void updateToolTips();
    QString separator;

private slots:
    void checkStateChanged(QListWidgetItem* changedItem);
    void itemDataChanged(QListWidgetItem* item);
    void clickedOnItem(QListWidgetItem* item);

signals:
    void itemCheckStateChanged(QListWidgetItem*);
};
#endif // QMULTISELECTWIDGET_H
