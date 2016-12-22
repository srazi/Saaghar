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

#ifndef SEARCHRESULTWIDGET_H
#define SEARCHRESULTWIDGET_H

#include <QMainWindow>
#include <QHash>
#include <QTableWidget>
#include <QToolButton>
#include <QAction>
#include <QLabel>

class QSearchLineEdit;

const int ITEM_SEARCH_DATA = Qt::UserRole + 10;


class SearchResultWidget : public QWidget
{
    Q_OBJECT

public:
    SearchResultWidget(QMainWindow* qmw, QWidget* parent = 0, const QString &searchPhrase = QString(), const QString &poetName = QString());
    ~SearchResultWidget();

    void setResultList(const QMap<int, QString> &map);

    void addTaskInQuequed();

    static int currentSearchWidgetCount();
    //static void setMaxItemPerPage(int max);
    QTableWidget* searchTable;
    static int maxItemPerPage;
    static bool nonPagedSearch;
    static bool skipVowelSigns;
    static bool skipVowelLetters;

private:
    QDockWidget* searchResultWidget;
    QWidget* searchResultContents;
    void setupUi(QMainWindow* qmw);
    void showSearchResult(int start);
    QString m_phrase;

    bool moreThanOnePage;
    QString m_sectionName;
    QToolButton* searchNextPage, *searchPreviousPage;
    QAction* actSearchNextPage, *actSearchPreviousPage;
    int pageNumber, pageCount;
    QLabel* pageLabel;
    QSearchLineEdit* filterLineEdit;
    QMap<int, QString> resultList;
    QMap<int, QString> copyResultList;
    QStringList viewedItems;

    Qt::DockWidgetArea m_dockWidgetArea;
    QMainWindow* m_mainWindow;
    int m_taskInQuequedCount;

    static int s_searchWidgetCount;

private slots:
    void currentRowColumnChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void searchPageNavigationClicked(QAction* action);
    void maxItemPerPageChange();
    void filterResults(const QString &text);
    void onConcurrentResultReady(const QString &type, const QVariant &results);
    void onDockLocationChanged(Qt::DockWidgetArea area);
    void createCustomContextMenu(const QPoint &pos);

protected:
    bool eventFilter(QObject* watched, QEvent* event);

signals:
    void searchFiltered(const QString &);
    void cancelProgress();
};

#endif // SEARCHRESULTWIDGET_H
