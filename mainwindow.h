/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
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
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();
		SaagharWidget *saagharWidget;
		void emitReSizeEvent();
		static bool autoCheckForUpdatesState;

	private:
		QString resourcesPath;//not-writable
		int previousTabIndex;
		void importDataBase(const QString fileName);
		QStringList mainToolBarItems;
		QAction *actionInstance(const QString actionObjectName = "", QString iconPath = "", QString displayName = "");
		void searchRegionsInitialize();
		void showSearchResults(QString phrase, int PageStart, int count, int PoetID, QWidget *dockWidget = 0);
		bool isPortable;
		QString convertToTeX(SaagharWidget *saagharObject);
		QPrinter *defaultPrinter;
		void printPreview(QPrinter *printer);
		QString tableToString(QTableWidget *table, QString columnSeparator, QString rowSeparator, int startRow, int startColumn, int endRow, int endColumn);
		void writeToFile(QString fileName, QString textToWrite);
		QString convertToHtml(SaagharWidget *saagharObject);
		QSettings *getSettingsObject();
		QString openedTabs;
		void saveSaagharState();
		void scrollToFirstFoundedItem(QString phrase, int PoemID, int vorder);
		QTabWidget *mainTabWidget;
		void loadTabWidgetSettings();
		bool settingsIconThemeState;
		QString settingsIconThemePath;
		void loadGlobalSettings();
		void saveGlobalSettings();
		bool processTextChanged;
		QToolBar *parentCatsToolBar;
		SaagharWidget *getSaagharWidget(int tabIndex);
		Qt::MouseButtons pressedMouseButton;
		void insertNewTab();
		Ui::MainWindow *ui;
		void createConnections();
		void setupUi();
		QLineEdit *lineEditSearchText;
		QComboBox *comboBoxSearchRegion;
		QSpinBox *spinBoxMaxSearchResult;
		QPushButton *pushButtonSearch;
		QMenu *menuNavigation;
		QMenu *menuTools;
		QMenu *menuFile;
		QMenu *menuHelp;
		QMenu *menuView;
		QMap<QString, QAction *> allActionMap;
		QAction *labelMaxResultSeparator;
		QAction *labelMaxResultAction;
		QAction *spinBoxMaxSearchResultAction;

	private slots:
		void checkForUpdates();
		void actFullScreenClicked(bool checked);
		void actionRemovePoet();
		void newSearchNonAlphabetChanged(bool checked);
		void newSearchFlagChanged(bool checked);
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
		void searchPageNavigationClicked(QAction *action);
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

	protected:
		void resizeEvent( QResizeEvent * event );
		void closeEvent( QCloseEvent * event );
};
#endif // MAINWINDOW_H
