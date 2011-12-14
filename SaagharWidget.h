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

#ifndef SAAGHARWIDGET_H
#define SAAGHARWIDGET_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QUndoStack>

#include "QGanjoorDbBrowser.h"
#include "settings.h"

#include "bookmarks.h"

class SaagharWidget : public QWidget
{
Q_OBJECT

public:
	SaagharWidget(QWidget *parent, QToolBar *catsToolBar, QTableWidget *tableWidget);
	~SaagharWidget();
	QString currentCaption;
	QPushButton *parentCatButton;
	QTableWidget *tableViewWidget;
	void resizeTable(QTableWidget *table);
	bool nextPoem();
	bool previousPoem();
	void clearSaagharWidget();
	QToolBar *parentCatsToolBar;
	void showHome();
	QString currentPageGanjoorUrl ();
	void loadSettings();
	void showParentCategory(GanjoorCat category);
	void processClickedItem(QString type, int id, bool error);
	void navigateToPage(QString type, int id, bool error);
	int minMesraWidth;

	//STATIC Variables
	static QString poetsImagesDir;
	static QLocale  persianIranLocal;
	static QFont tableFont;
	static bool showBeytNumbers;
	static bool backgroundImageState;
	static bool centeredView;
	//static bool newSearchFlag;
	//static bool newSearchSkipNonAlphabet;
	static QString backgroundImagePath;
	static QColor textColor;
	static QColor matchedTextColor;
	static QColor backgroundColor;
	static QTableWidgetItem *lastOveredItem;
	static int maxPoetsPerGroup;

	//bookmark widget
	static Bookmarks *bookmarks;

	//DataBase
	static QGanjoorDbBrowser *ganjoorDataBase;
	static int computeRowHeight(const QFontMetrics &fontMetric, int textWidth, int width, int height=0);

	int currentPoem;
	int currentCat;
	int currentParentID;
	QStringList currentLocationList;
	QString currentPoemTitle;

	void homeResizeColsRows();
	inline bool isDirty()
		{return dirty;}
	inline void setDirty()
		{dirty = true;}
	QStringList identifier();
	void refresh();

	//Undo FrameWork
	QUndoStack *undoStack;

private:
	bool initializeCustomizedHome();
	QMap<int,int> singleColumnHeightMap;
	void showCategory(GanjoorCat category);
	void showPoem(GanjoorPoem poem);
	QPoint pressedPosition;
	bool dirty;

private slots:
	void parentCatClicked();
	void clickedOnItem(int row,int col);
	void pressedOnItem(int row,int col);

public slots:
	QTableWidgetItem *scrollToFirstItemContains(const QString &phrase, bool pharseIsList = true, bool scroll = true);

signals:
	void captionChanged();
	void navNextActionState(bool);
	void navPreviousActionState(bool);
	void loadingStatusText(const QString &);
};
#endif // SAAGHARWIDGET_H
