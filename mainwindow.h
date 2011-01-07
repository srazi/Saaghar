/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://www.pojh.co.cc>       *
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

	private:
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
		QMenu *menuNavigation;
		QMenu *menuTools;
		QMenu *menuFile;
		QMenu *menuHelp;
		QAction *actionHome;
		QAction *actionPreviousPoem;
		QAction *actionNextPoem;
		QAction *actionViewInGanjoorSite;
		QAction *actionExit;
		QAction *actionNewTab;
		QAction *actionNewWindow;
		QAction *actionAboutSaaghar;
		QAction *searchToolbarAction;
		QAction *actionCopy;
		QAction *actionSettings;
		QAction *actionFaal;
		QAction *actionAboutQt;
		QAction *actionPrint;
		QAction *actionPrintPreview;
		QAction *actionExport;
		QAction *actionExportAsPDF;
		QAction *actionHelpContents;

	private slots:
		void helpContents();
		void actionPrintPreviewClicked();
		void print(QPrinter *printer);
		void actionExportAsPDFClicked();
		void actionExportClicked();
		void actionPrintClicked();
		void actionFaalClicked();
		void aboutSaaghar();
		void tableSelectChanged();
		void searchPageNavigationClicked(QAction *action);
		void tableItemPress(QTableWidgetItem *item);
		void tableItemClick(QTableWidgetItem *item);
		void actionNewTabClicked();
		void actionNewWindowClicked();
		void currentTabChanged(int tabIndex);
		void tabCloser(int tabIndex);
		void globalSettings();
		void newTabForItem(QString type, int id, bool noError);
		void updateCaption();
		void searchWidgetClose();
		void searchStart();
		void searchWidgetSetVisible(bool visible);
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
