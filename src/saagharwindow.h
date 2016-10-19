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

#ifndef SAAGHARWINDOW_H
#define SAAGHARWINDOW_H

#include "saagharwidget.h"

#include <QMainWindow>
#include <QSettings>
#include <QSpinBox>
#include <QToolButton>
#include <QUndoGroup>
#include <QComboBox>
class QPrinter;

class QMultiSelectWidget;
class OutlineTree;
class Settings;
class TabWidget;
class QIrBreadCrumbBar;
class BreadCrumbSaagharModel;
class AudioRepoDownloader;

namespace Ui
{
class SaagharWindow;
}

class SaagharWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SaagharWindow(QWidget* parent = 0);
    ~SaagharWindow();

    enum TabType {
        SaagharViewerTab,
        WidgetTab
    };

    SaagharWidget* saagharWidget;

    void checkRegistration(bool forceShow = false);
    QMenu* cornerMenu();

    void saveSettings();

private:
    void loadAudioForCurrentTab(SaagharWidget* old_saagharWidget = 0);

    QSplitter* splitter;
    OutlineTree* outlineTree;
    bool skipContextMenu;
    void setupBookmarkManagerUi();
    QBoxLayout* searchToolBarBoxLayout;
    bool skipSearchToolBarResize;
    void setupSearchToolBarUi();
    bool eventFilter(QObject* receiver, QEvent* event);
    void toolBarViewActions(QToolBar* toolBar, QMenu* menu, bool subMenu);

    int previousTabIndex;

    QAction* actionInstance(const QString &actionObjectName = "", QString iconPath = "", QString displayName = "");
    void deleteActionInstance(const QString &actionObjectName);
    void multiSelectObjectInitialize(QMultiSelectWidget* multiSelectWidget, const QStringList &selectedData = QStringList(), int insertIndex = 0);
    void multiSelectInsertItems(QMultiSelectWidget* multiSelectWidget);
    QString convertToTeX(SaagharWidget* saagharObject);
    QPrinter* defaultPrinter;
    void printPreview(QPrinter* printer);
    QString tableToString(QTableWidget* table, QString columnSeparator, QString rowSeparator, int startRow, int startColumn, int endRow, int endColumn);
    void writeToFile(QString fileName, QString textToWrite);
    QString convertToHtml(SaagharWidget* saagharObject);
    //QStringList openedTabs;
    void scrollToFirstFoundedItem(QString phrase, int PoemID, int vorder);
    TabWidget* mainTabWidget;
    void loadTabWidgetSettings();

    bool processTextChanged;
    QToolBar* parentCatsToolBar;
    QToolBar* m_breadCrumbToolBar;
    QIrBreadCrumbBar* m_breadCrumbBar;
    BreadCrumbSaagharModel* m_breadCrumbSaagharModel;
    SaagharWidget* getSaagharWidget(int tabIndex);
    Qt::MouseButtons pressedMouseButton;

    // empty connectionID means DatabaseBrowser::defaultConnectionId()
    QWidget* insertNewTab(TabType tabType = SaagharWindow::SaagharViewerTab, const QString &title = QString(),
                          int id = -1, const QString &type = "CatID", bool noError = true,
                          bool pushToStack = true, const QString &connectionID = QString());
    Ui::SaagharWindow* ui;
    void createConnections();
    void setupUi();
    QMultiSelectWidget* selectSearchRange;
    QComboBox* comboBoxSearchRegion;
    QSpinBox* spinBoxMaxSearchResult;
    QPushButton* pushButtonSearch;
    QMenu* menuNavigation;
    QMenu* menuTools;
    QMenu* menuFile;
    QMenu* menuHelp;
    QMenu* menuView;
    QMenu* menuBookmarks;
    QMenu* m_cornerMenu;

    //submenus
    QMenu* menuOpenedTabs, *menuClosedTabs;
    //void updateTabsSubMenus();//move to slot section

    QMap<QString, QAction*> allActionMap;
    QAction* labelMaxResultSeparator;
    //QAction *labelMaxResultAction;
    QAction* spinBoxMaxSearchResultAction;
    QMenu* searchOptionMenu;
    QString mainToolBarSizeAction;
    QString mainToolBarStyleAction;
    //bool dataFromIdentifier(const QString &identifier, QString *type = 0, int *id = 0);

    //Undo FrameWork
    QUndoGroup* undoGroup;
    QAction* globalRedoAction;
    QAction* globalUndoAction;
    Settings* m_settingsDialog;

    QString m_compareSettings;
    QDockWidget* m_outlineDock;
    QDockWidget* m_bookmarkManagerDock;
#ifdef MEDIA_PLAYER
    AudioRepoDownloader* m_audioRepoDownloader;
#endif
    bool m_updateTaskScheduled;

public slots:
    void updateTabsSubMenus();
    void highlightTextOnPoem(int poemId, int vorder);

private slots:
    void openPath(const QString &path);
    void openParentPage(int parentID, bool newPage = false);
    void openChildPage(int childID, bool newPage = false);
    void openPage(int id, SaagharWidget::PageType type, bool newPage = false, const QString &connectionID = QString());
    void createCustomContextMenu(const QPoint &pos);
    void onCurrentLocationChanged(const QStringList &locationList, const QString &connectionID = QString());

#ifdef MEDIA_PLAYER
    void mediaInfoChanged(const QString &fileName, const QString &title = "", int id = -1);
#endif

    void ensureVisibleBookmarkedItem(const QString &type, const QString &itemText, const QString &data, bool ensureVisible = true, bool unbookmark = true);
    void setHomeAsDirty();
    void setAllAsDirty();
    void setAsDirty(int catId, int poemId);//-1 for skip it
    void actionClosedTabsClicked();
    void namedActionTriggered(bool checked = false);
    void toolbarViewChanges(QAction* action);
    void customizeRandomDialog();
    void toolBarContextMenu(const QPoint &pos);
    void checkForUpdates();
    void processUpdateData(const QString &type, const QVariant &results);
    void actFullScreenClicked(bool checked);
    void actionRemovePoet();
    //void newSearchNonAlphabetChanged(bool checked);
    //void newSearchFlagChanged(bool checked);
    void actionImportNewSet();
    void closeCurrentTab();
    void helpContents();
    void actionPrintPreviewClicked();
    void print(QPrinter* printer);
    void actionExportAsPDFClicked();
    void actionExportClicked();
    void actionPrintClicked();
    // empty connectionID means DatabaseBrowser::defaultConnectionId()
    void openRandomPoem(int parentID, bool newPage = false, const QString &connectionID = QString());
    void aboutSaaghar();
    void tableSelectChanged();
    void tableItemPress(QTableWidgetItem* item);
    void tableItemClick(QTableWidgetItem* item);
    void tableItemMouseOver(QTableWidgetItem* item);
    void tableCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
    void actionNewTabClicked();
    void actionNewWindowClicked();
    void currentTabChanged(int tabIndex);
    void tabCloser(int tabIndex);
    void showSettingsDialog();
    void applySettings();
    void newTabForItem(int id, const QString &type = "CatID", bool noError = true, bool pushToStack = true, const QString &connectionID = QString());
    void updateCaption();
    void searchStart();
    //Navigation
    void copySelectedItems();
    void actionHomeClicked();
    void actionPreviousPoemClicked();
    void actionNextPoemClicked();
    //Tools
    void actionGanjoorSiteClicked();

    //search options
    void showSearchOptionsDialog();
    void showSearchOptionMenu();
    void showSearchTips();
    void showStatusText(const QString &message, int newLevelsCount = 0);

    void onDatabaseUpdate(const QString &connectionID);

protected:
//      void resizeEvent( QResizeEvent * event );
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void closeEvent(QCloseEvent* event);
#if QT_VERSION >= 0x050000
    void paintEvent(QPaintEvent* event);
#endif

signals:
    void maxItemPerPageChanged();
    void loadingStatusText(const QString &);
    void highlightedTextChanged(const QString &);
    void verseSelected(int);
};
#endif // SAAGHARWINDOW_H
