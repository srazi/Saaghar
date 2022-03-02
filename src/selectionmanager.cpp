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

#include "selectionmanager.h"
#include "ui_selectionmanager.h"
#include "saagharapplication.h"
#include "outlinemodel.h"
#include "settingsmanager.h"
#include "tools.h"

#include <QItemSelectionModel>

SelectionManager::SelectionManager(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SelectionManager),
    m_parentsSelectChildren(false)
{
    ui->setupUi(this);

    setWindowTitle(tr("Selection Manager"));
    setObjectName(QLatin1String("SelectionManager"));

    setFocusProxy(ui->selectionView);
    ui->selectionPreView->setExpandsOnDoubleClick(false);
    ui->selectionPreView->setFocusPolicy(Qt::NoFocus);
    setFocus();

    ui->selectionView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->selectionPreView->setSelectionMode(QAbstractItemView::NoSelection);

    ui->selectionView->setModel(sApp->outlineModel());

    QItemSelectionModel* selectionModel = new QItemSelectionModel(sApp->outlineModel());
    ui->selectionView->setSelectionModel(selectionModel);

    m_selectionProxyModel = new SelectionProxyModel(selectionModel, this);
    m_selectionProxyModel->setSourceModel(sApp->outlineModel());
    ui->selectionPreView->setModel(m_selectionProxyModel);

    ui->selectionPreView->setItemDelegate(new SelectionPreviewDelegate(m_selectionProxyModel, this));

    connect(m_selectionProxyModel, SIGNAL(filterInvalidated()), ui->selectionPreView, SLOT(expandAll()));
    connect(ui->selectionPreView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onPreviewIndexDoubleClicked(QModelIndex)));

    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(acceptAndClose()));
}

SelectionManager::~SelectionManager()
{
    delete ui;
}

QStringList SelectionManager::selectionPaths() const
{
    return m_selectionProxyModel->paths();
}

QStringList SelectionManager::selectedCategoriesIDs() const
{
    return m_selectionProxyModel->selectedCategoriesIDs();
}

void SelectionManager::setSelection(const QStringList &paths)
{
    static const QString SEPARATOR = QLatin1String("/");

    foreach (const QString &path, paths) {
        bool ok;
        QModelIndex ind = sApp->outlineModel()->index(path.split(SEPARATOR, SKIP_EMPTY_PARTS), &ok);

        if (ok) {
            ui->selectionView->selectionModel()->select(ind, QItemSelectionModel::Select);
        }
    }
}

void SelectionManager::setButtonBoxHidden(bool hide)
{
    ui->buttonBox->setHidden(hide);
}

void SelectionManager::setSettingsPath(const QString &settingsPath)
{
    Q_ASSERT(m_settingsPath.isEmpty());

    m_settingsPath = settingsPath;

    setSelection(VAR(m_settingsPath.toLatin1().constData()).toStringList());
}

void SelectionManager::parentsSelectChildren(bool yes)
{
    if (m_parentsSelectChildren != yes) {
        m_parentsSelectChildren = yes;
        m_selectionProxyModel->parentsSelectChildren(yes);
    }
}

void SelectionManager::onPreviewIndexDoubleClicked(const QModelIndex &index)
{
    ui->selectionView->scrollTo(m_selectionProxyModel->mapToSource(index));
    setFocus();
}

void SelectionManager::accept()
{
    if (!m_settingsPath.isEmpty()) {
        VAR_DECL(m_settingsPath.toLatin1().constData(), selectionPaths());
        VAR_DECL(QString("%1/CategoriesIDs").arg(m_settingsPath).toLatin1().constData(), selectedCategoriesIDs());
    }

    emit accepted();
}

void SelectionManager::acceptAndClose()
{
    accept();
    close();
}

void SelectionManager::clearSelection()
{
    ui->selectionView->selectionModel()->clearSelection();
}


SelectionProxyModel::SelectionProxyModel(QItemSelectionModel* selectionModel, QObject* parent)
    : QSortFilterProxyModel(parent),
      m_selectionModel(selectionModel),
      m_filterIndices(),
      m_parentsSelectChildren(false)
{
    connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged()));
}

QStringList SelectionProxyModel::paths() const
{
    return m_paths;
}

QStringList SelectionProxyModel::selectedCategoriesIDs() const
{
    return m_selectedCategoriesIDs;
}

void SelectionProxyModel::parentsSelectChildren(bool yes)
{
    m_parentsSelectChildren = yes;

    onSelectionChanged();
}

bool SelectionProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_column);
    Q_UNUSED(source_parent);

    return true;
}

bool SelectionProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    for (int i = 0; i < m_filterIndices.size(); ++i) {
        const QModelIndex index = m_filterIndices.at(i);

        if (index.row() == source_row && index.parent() == source_parent) {
            return true;
        }
    }

    return false;
}

void SelectionProxyModel::onSelectionChanged()
{
    m_selectedIndices = m_selectionModel->selectedIndexes();

    computeFilterIndices();

    invalidateFilter();

    emit filterInvalidated();
}

static void addAllChildren(QStringList &categoriesIDs, const QAbstractItemModel* model, const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    for (int j = 0; j < model->rowCount(index); ++j) {
        const QModelIndex child = model->index(j, 0, index);
        if (child.isValid()) {
            categoriesIDs << QString::number(child.data(OutlineModel::IDRole).toInt());
            addAllChildren(categoriesIDs, model, child);
        }
    }
}

void SelectionProxyModel::computeFilterIndices()
{
    static const QString SEPARATOR = QLatin1String("/");
    m_paths.clear();
    m_selectedCategoriesIDs.clear();
    m_filterIndices = m_selectedIndices;

    if (m_parentsSelectChildren) {
        for (int i = 0; i < m_selectedIndices.size(); ++i) {
            QModelIndex index = m_selectedIndices.at(i);
            QModelIndex parent = index.parent();
            while (parent.isValid()) {
                if (m_selectedIndices.contains(parent)) {
                    m_filterIndices.removeAll(index);
                    break;
                }

                parent = parent.parent();
            }
        }

        m_selectedIndices = m_filterIndices;
    }

    QStringList path;
    QStringList categoriesIDs;
    const QAbstractItemModel* model = m_selectionModel->model();

    for (int i = 0; i < m_selectedIndices.size(); ++i) {
        path.clear();
        categoriesIDs.clear();

        QModelIndex index = m_selectedIndices.at(i);
        if (!index.isValid()) {
            continue;
        }

        path.prepend(index.data(OutlineModel::TitleRole).toString());
        categoriesIDs << QString::number(index.data(OutlineModel::IDRole).toInt());

        if (m_parentsSelectChildren && model) {
            addAllChildren(categoriesIDs, model, index);
        }

        while (index.isValid()) {
            index = index.parent();
            path.prepend(index.data(OutlineModel::TitleRole).toString());

            if (!m_filterIndices.contains(index)) {
                m_filterIndices << index;
            }
        }

        m_paths << path.join(SEPARATOR);
        m_selectedCategoriesIDs << categoriesIDs.join(QLatin1String(","));
    }

    m_paths.removeDuplicates();
    m_selectedCategoriesIDs.removeDuplicates();
}

bool SelectionProxyModel::selectionContains(const QModelIndex &index)
{
    return m_selectedIndices.contains(mapToSource(index));
}


SelectionPreviewDelegate::SelectionPreviewDelegate(SelectionProxyModel* selectionProxyModel, QObject* parent)
    : QStyledItemDelegate(parent),
      m_selectionProxyModel(selectionProxyModel)
{
}

void SelectionPreviewDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    if (m_selectionProxyModel->selectionContains(index)) {
        option->font.setWeight(QFont::DemiBold);
    }
    else {
        option->font.setWeight(QFont::Light);
    }

    option->state = option->state & ~QStyle::State_HasFocus;
}
