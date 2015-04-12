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

#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QDialog>
#include <QModelIndexList>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

class QItemSelection;
class QItemSelectionModel;

namespace Ui
{
class SelectionManager;
}

class SelectionProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SelectionProxyModel(QItemSelectionModel* selectionModel, QObject* parent = 0);

    QStringList paths() const;

protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private slots:
    void onSelectionChanged();

private:
    void computeFilterIndices();
    bool selectionContains(const QModelIndex &index);

    QItemSelectionModel* m_selectionModel;
    QModelIndexList m_filterIndices;
    QModelIndexList m_selectedIndices;
    QStringList m_paths;

    friend class SelectionPreviewDelegate;

signals:
    void filterInvalidated();
};

class SelectionPreviewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SelectionPreviewDelegate(SelectionProxyModel* selectionProxyModel, QObject* parent = 0);

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex &index) const;

private:
    SelectionProxyModel* m_selectionProxyModel;
};

class SelectionManager : public QDialog
{
    Q_OBJECT

public:
    explicit SelectionManager(QWidget* parent = 0);
    ~SelectionManager();

    QStringList selectionPaths() const;
    void setSelection(const QStringList &paths);

private slots:
    void onPreviewIndexDoubleClicked(const QModelIndex &index);

private:
    Ui::SelectionManager* ui;
    SelectionProxyModel* m_selectionProxyModel;
};

#endif // SELECTIONMANAGER_H
