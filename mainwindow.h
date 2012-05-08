/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2012 by S. Razi Alavizadeh                          *
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "SaagharWidget.h"

#include <QMainWindow>
#include <QSettings>
#include <QSpinBox>
#include <QToolButton>
#include <QUndoGroup>
#include <QComboBox>

class QMultiSelectWidget;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0, QObject *splashScreen = 0, bool fresh = false);
		~MainWindow();
		SaagharWidget *saagharWidget;
//		void emitReSizeEvent();
		static bool autoCheckForUpdatesState;

//	public slots:
//		void emitReSizeEvent();

	private:
		void setupBookmarkManagerUi();
		QString currentIconThemePath();
		QBoxLayout *searchToolBarBoxLayout;
		bool skipSearchToolBarResize;
		void setupSearchToolBarUi();
		bool eventFilter(QObject* receiver, QEvent* event);
		void toolBarViewActions(QToolBar *toolBar, QMenu *menu, bool subMenu);
		QStringList selectedRandomRange;
		QStringList selectedSearchRange;
		bool randomOpenInNewTab;
		QString resourcesPath;//not-writable
		QString userHomePath;//writable
		int previousTabIndex;
		void importDataBase(const QString fileName);
		QStringList mainToolBarItems;
		QAction *actionInstance(const QString actionObjectName = "", QString iconPath = "", QString displayName = "");
		void multiSelectObjectInitialize(QMultiSelectWidget *multiSelectWidget, const QStringList &selectedData = QStringList());
		bool isPortable;
		QString convertToTeX(SaagharWidget *saagharObject);
		QPrinter *defaultPrinter;
		void printPreview(QPrinter *printer);
		QString tableToString(QTableWidget *table, QString columnSeparator, QString rowSeparator, int startRow, int startColumn, int endRow, int endColumn);
		void writeToFile(QString fileName, QString textToWrite);
		QString convertToHtml(SaagharWidget *saagharObject);
		QSettings *getSettingsObject();
		//QStringList openedTabs;
		void scrollToFirstFoundedItem(QString phrase, int PoemID, int vorder);
		QTabWidget *mainTabWidget;
		void loadTabWidgetSettings();
		bool settingsIconThemeState;
		QString settingsIconThemePath;
		void loadGlobalSettings();
		void saveSettings();
		bool processTextChanged;
		QToolBar *parentCatsToolBar;
		SaagharWidget *getSaagharWidget(int tabIndex);
		Qt::MouseButtons pressedMouseButton;
		void insertNewTab();
		Ui::MainWindow *ui;
		void createConnections();
		void setupUi();
		QMultiSelectWidget *selectSearchRange;
		QComboBox *comboBoxSearchRegion;
		QSpinBox *spinBoxMaxSearchResult;
		QPushButton *pushButtonSearch;
		QMenu *menuNavigation;
		QMenu *menuTools;
		QMenu *menuFile;
		QMenu *menuHelp;
		QMenu *menuView;
		QMenu *menuBookmarks;

		//submenus
		QMenu *menuOpenedTabs, *menuClosedTabs;
		//void updateTabsSubMenus();//move to slot section

		QMap<QString, QAction *> allActionMap;
		QAction *labelMaxResultSeparator;
		//QAction *labelMaxResultAction;
		QAction *spinBoxMaxSearchResultAction;
		QMenu *searchOptionMenu;
		QString mainToolBarSizeAction;
		QString mainToolBarStyleAction;
		//bool dataFromIdentifier(const QString &identifier, QString *type = 0, int *id = 0);
		
		//Undo FrameWork
		QUndoGroup *undoGroup;
		QAction *globalRedoAction;
		QAction *globalUndoAction;

	public slots:
		void updateTabsSubMenus();

	private slots:
		void mediaInfoChanged(const QString &fileName);
		void ensureVisibleBookmarkedItem(const QString &type, const QString &itemText, const QString &data, bool ensureVisible = true, bool unbookmark = true);
		void setHomeAsDirty();
		void setAllAsDirty();
		void setAsDirty(int catId = -1, int poemId = -1);//-1 for skip it
		void actionClosedTabsClicked();
		void namedActionTriggered(bool checked = false);
		void toolbarViewChanges(QAction *action);
		void customizeRandomDialog();
		void toolBarContextMenu(const QPoint &pos);
		void getMaxResultPerPage();
		void checkForUpdates();
		void actFullScreenClicked(bool checked);
		void actionRemovePoet();
		//void newSearchNonAlphabetChanged(bool checked);
		//void newSearchFlagChanged(bool checked);
		void actionImportNewSet();
		void closeCurrentTab();
		void helpContents();
		void actionPrintPreviewClicked();
		void print(QPrinter *printer);
		void actionExportAsPDFClicked();
		void actionExportClicked();
		void actionPrintClicked();
		void actionFaalRandomClicked();
		void aboutSaaghar();
		void tableSelectChanged();
		void tableItemPress(QTableWidgetItem *item);
		void tableItemClick(QTableWidgetItem *item);
		void tableItemMouseOver(QTableWidgetItem *item);
		void tableCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
		void actionNewTabClicked();
		void actionNewWindowClicked();
		void currentTabChanged(int tabIndex);
		void tabCloser(int tabIndex);
		void globalSettings();
		void newTabForItem(QString type, int id, bool noError);
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
		void showSearchOptionMenu();

	protected:
//		void resizeEvent( QResizeEvent * event );
		void closeEvent( QCloseEvent * event );
	
	signals:
		void maxItemPerPageChanged(int);
		void loadingStatusText(const QString &);
};
#endif // MAINWINDOW_H
