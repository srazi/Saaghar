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

#include "SearchResultWidget.h"
#include "SearchItemDelegate.h"
#include "QGanjoorDbBrowser.h"

#include <QSearchLineEdit>

#include <QMessageBox>
#include <QDockWidget>
#include <QProgressDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QApplication>

int SearchResultWidget::maxItemPerPage = 0;

SearchResultWidget::SearchResultWidget(QWidget *parent, const QString &searchPhrase, int count, const QString &poetName)
	: QWidget(parent), searchResultContents(parent)
{
	actSearchNextPage = 0;
	actSearchPreviousPage = 0;
	phrase = searchPhrase;
	sectionName = poetName;
	maxItemPerPageChange(count);
}

SearchResultWidget::~SearchResultWidget()
{
	//qDebug() << "SearchResultWidget is destroyed!";
}

void SearchResultWidget::setResultList(const QMap<int, QString> &map)
{
	copyResultList = resultList = map;
}

bool SearchResultWidget::init(QMainWindow *qmw, const QString &iconThemePath)
{
	if (resultList.isEmpty()) return false;

	moreThanOnePage = SearchResultWidget::maxItemPerPage>0 && resultList.size() >= SearchResultWidget::maxItemPerPage+1;
	navButtonsNeeded = false;

	setupUi(qmw, iconThemePath);
	showSearchResult(0);
	
	return true;
}

void SearchResultWidget::setupUi(QMainWindow *qmw, const QString &iconThemePath)
{
	QDockWidget *searchResultWidget = 0;
	searchResultWidget = new QDockWidget(sectionName+":"+phrase, qmw);
	searchResultWidget->setObjectName(QString::fromUtf8("searchResultWidget_new"));//object name for created instance, it renames to 'searchResultWidget_old'
	//searchResultWidget->setLayoutDirection(Qt::RightToLeft);
	searchResultWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
	searchResultWidget->setAttribute(Qt::WA_DeleteOnClose, true);

	QGridLayout *searchGridLayout = new QGridLayout(searchResultContents);
	searchGridLayout->setSpacing(6);
	searchGridLayout->setContentsMargins(11, 11, 11, 11);
	searchGridLayout->setObjectName(QString::fromUtf8("searchGridLayout"));
	QHBoxLayout *horizontalLayout = new QHBoxLayout();
	horizontalLayout->setSpacing(6);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	QGridLayout *searchTableGridLayout = new QGridLayout();
	searchTableGridLayout->setSpacing(6);
	searchTableGridLayout->setObjectName(QString::fromUtf8("gridLayout_3"));

	//create filter lable and lineEdit and layout
	QHBoxLayout *filterHorizontalLayout = new QHBoxLayout();
	filterHorizontalLayout->setSpacing(6);
	filterHorizontalLayout->setObjectName(QString::fromUtf8("filterHorizontalLayout"));

	QLabel *filterLabel = new QLabel(searchResultContents);
	filterLabel->setObjectName(QString::fromUtf8("filterLabel"));
	filterLabel->setText(tr("Filter:"));
	filterHorizontalLayout->addWidget(filterLabel, 0, Qt::AlignRight|Qt::AlignCenter);
	
	QString clearIconPath = iconThemePath+"/clear-left.png";
	if (searchResultContents->layoutDirection() == Qt::RightToLeft)
		clearIconPath = iconThemePath+"/clear-right.png";
	filterLineEdit = new QSearchLineEdit(searchResultContents, clearIconPath, iconThemePath+"/filter.png");
	filterLineEdit->setObjectName(QString::fromUtf8("filterLineEdit"));
	connect(filterLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterResults(const QString &)));
	filterHorizontalLayout->addWidget(filterLineEdit);
	QSpacerItem *filterHorizSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	filterHorizontalLayout->addItem(filterHorizSpacer);

	pageLabel = new QLabel(searchResultContents);
	pageLabel->setObjectName(QString::fromUtf8("pageLabel"));
	filterHorizontalLayout->addWidget(pageLabel, 0, Qt::AlignRight|Qt::AlignCenter);

	searchTableGridLayout->addLayout(filterHorizontalLayout, 1, 0, 1, 1);

	//create QTableWidget
	searchTable = new QTableWidget(searchResultContents);
	searchTable->setObjectName(QString::fromUtf8("searchTable"));
	searchTable->setColumnCount(3);
	searchTable->setLayoutDirection(Qt::RightToLeft);
	searchTable->setAlternatingRowColors(true);
	searchTable->setSelectionMode(QAbstractItemView::NoSelection);
	searchTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	searchTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	searchTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	searchTable->verticalHeader()->setVisible(false);
	//searchTable->horizontalHeader()->setVisible(false);
	searchTable->horizontalHeader()->setHighlightSections(false);
	searchTable->horizontalHeader()->setStretchLastSection(true);

	//install delagate on third column
	SaagharItemDelegate *searchDelegate = new SaagharItemDelegate(searchTable, searchTable->style(), phrase);
	searchTable->setItemDelegateForColumn(2, searchDelegate);
	connect(this, SIGNAL(searchFiltered(const QString &)), searchDelegate, SLOT(keywordChanged(const QString &)) );

	//searchTable->setItemDelegateForColumn(2, new SaagharItemDelegate(searchTable, searchTable->style(), phrase));

	searchTableGridLayout->addWidget(searchTable, 0, 0, 1, 1);

	QVBoxLayout *searchNavVerticalLayout = new QVBoxLayout();
	searchNavVerticalLayout->setSpacing(6);
	searchNavVerticalLayout->setObjectName(QString::fromUtf8("searchNavVerticalLayout"));

	searchNextPage = new QToolButton(searchResultContents);
	searchNextPage->setObjectName(QString::fromUtf8("searchNextPage"));

	actSearchNextPage = new QAction(searchResultContents);
	//actSearchNextPage->setIcon(actionInstance("actionNextPoem")->icon());
	searchNextPage->setDefaultAction(actSearchNextPage);

	connect(searchNextPage, SIGNAL(triggered(QAction *)), this, SLOT(searchPageNavigationClicked(QAction *)));

	searchNextPage->setEnabled(false);
	searchNextPage->hide();
	
	searchNavVerticalLayout->addWidget(searchNextPage);

	searchPreviousPage = new QToolButton(searchResultContents);
	searchPreviousPage->setObjectName(QString::fromUtf8("searchPreviousPage"));
	
	actSearchPreviousPage = new QAction(searchResultContents);
	//actSearchPreviousPage->setIcon(actionInstance("actionPreviousPoem")->icon());
	searchPreviousPage->setDefaultAction(actSearchPreviousPage);

	if (qmw->layoutDirection() == Qt::LeftToRight)
	{
		actSearchPreviousPage->setIcon(QIcon(iconThemePath+"/previous.png"));
		actSearchNextPage->setIcon(QIcon(iconThemePath+"/next.png"));
	}
	else
	{
		actSearchPreviousPage->setIcon(QIcon(iconThemePath+"/next.png"));
		actSearchNextPage->setIcon(QIcon(iconThemePath+"/previous.png"));
	}

	connect(searchPreviousPage, SIGNAL(triggered(QAction *)), this, SLOT(searchPageNavigationClicked(QAction *)));

	searchPreviousPage->setEnabled(false);
	searchPreviousPage->hide();

	searchNavVerticalLayout->addWidget(searchPreviousPage);

	QSpacerItem *searchNavVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

	searchNavVerticalLayout->addItem(searchNavVerticalSpacer);

	if (moreThanOnePage)
		horizontalLayout->addLayout(searchNavVerticalLayout);

	horizontalLayout->addLayout(searchTableGridLayout);

	searchGridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

/****************************/
	QDockWidget *tmpDockWidget = 0;
	QObjectList mainWindowChildren = qmw->children();
	for (int i=0; i < mainWindowChildren.size(); ++i)
	{
		tmpDockWidget = qobject_cast<QDockWidget *>(mainWindowChildren.at(i));
		if (tmpDockWidget)
		{
			if (mainWindowChildren.at(i)->objectName() == "searchResultWidget_old")
			break;
		}
	}

/****************************************/
	searchResultWidget->setWidget(searchResultContents);
	
	qmw->addDockWidget(Qt::LeftDockWidgetArea, searchResultWidget);
	
	if (tmpDockWidget && tmpDockWidget->objectName() == "searchResultWidget_old") //there is another search results dock-widget present
		qmw->tabifyDockWidget(tmpDockWidget, searchResultWidget);

	
	searchResultWidget->setObjectName("searchResultWidget_old");

	searchResultWidget->show();
	searchResultWidget->raise();
}

void SearchResultWidget::showSearchResult(int start)
{
	if (resultList.isEmpty()) return;
	if (start < 0) start = 0;
	int end = 0;
	if (SearchResultWidget::maxItemPerPage == 0)
		moreThanOnePage = false;
	else
	{
		end = start + SearchResultWidget::maxItemPerPage-1;
		
		if (end >= resultList.size()) end = resultList.size()-1;
		if (start > end) return;
	}

	if (!moreThanOnePage)
	{
		start = 0;
		end = resultList.size()-1;
		pageNumber = 1;
		pageCount = 1;
	}
	else
	{
		pageCount = resultList.size()/SearchResultWidget::maxItemPerPage;
		if (resultList.size()%SearchResultWidget::maxItemPerPage != 0)
			++pageCount;
		if (end == resultList.size()-1)
			pageNumber = pageCount;
		else if (start == 0)
			pageNumber = 1;
		else
		{
			pageNumber = (start+1)/SearchResultWidget::maxItemPerPage;//'start' is started from 0
			if ((start+1)%SearchResultWidget::maxItemPerPage != 0)
				++pageNumber;
		}
	}
	//updatePageLabel(pageCount, pageNumber;
	pageLabel->setText(tr("page %1 of %2 page(s)").arg(pageNumber).arg(pageCount));

	int count = end-start+1;
	searchTable->clear();
	searchTable->setRowCount(count);
	searchTable->setHorizontalHeaderLabels(QStringList() << tr("#") << tr("Title") << tr("Verse") );

	navButtonsNeeded = moreThanOnePage;
	
	if (navButtonsNeeded)
	{//almost one of navigation button needs to be enabled
		searchNextPage->show();
		searchPreviousPage->show();
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QProgressDialog progress(tr("Initializing Results Table..."), tr("Cancel"), 0, count, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(3000);

	searchTable->setColumnWidth(0, searchTable->fontMetrics().boundingRect(QString::number((end+1)*100)).width());
	int maxPoemWidth = 0, maxVerseWidth = 0;

	QList<int> tmpList = resultList.keys();
	////count = tmpList.size();
	//int progressBarIndex = 50;
	//const int step = count/20;
	
	//bool firstTimeUpdate = false;

	for (int i = start; i < end+1; ++i)
	{
		/*if (i>progressBarIndex)
		{
			progress.setValue(i);
			progressBarIndex += step;
			if (!firstTimeUpdate)
			{
				searchTable->setUpdatesEnabled(false);
				firstTimeUpdate = true;
			}
		}*/
		
		if (count > 300) progress.setValue(i-start);

		if (progress.wasCanceled())
			break;

		int poemId = tmpList.at(i);
		//GanjoorPoem poem = SaagharWidget::ganjoorDataBase->getPoem(poemId/*resultList.at(i)*/);

		//poem._HighlightText = phrase;
		
		//we need a modified verion of showParentCategory
		//showParentCategory(SaagharWidget::ganjoorDataBase->getCategory(poem._CatID));
		//adding Numbers
		QLocale persianLocal = QLocale(QLocale::Persian, QLocale::Iran);
		persianLocal.setNumberOptions(QLocale::OmitGroupSeparator);
		QString localizedNumber = persianLocal.toString(i+1);
		QTableWidgetItem *numItem = new QTableWidgetItem(localizedNumber);
		numItem->setFlags(Qt::NoItemFlags /*Qt::ItemIsEnabled*/);

		searchTable->setItem(i-start, 0, numItem);

		QString firstVerse = "", poemTiltle = "", poetName = "";
		QStringList verseData = resultList.value(poemId, "").split("|", QString::SkipEmptyParts);
		if (verseData.size() == 3)
		{
			firstVerse = verseData.at(0);
			firstVerse.remove("verseText=");
			
			poemTiltle = verseData.at(1);
			poemTiltle.remove("poemTitle=");
			
			poetName = verseData.at(2);
			poetName.remove("poetName=");
		}

		//add Items
		QString snippedPoemTitle = QGanjoorDbBrowser::snippedText(poemTiltle, "", 0, 5, true);
		if (sectionName == tr("All"))
			snippedPoemTitle.prepend(poetName+": ");
		QTableWidgetItem *poemItem = new QTableWidgetItem( snippedPoemTitle );
		poemItem->setFlags(Qt::ItemIsEnabled);
		poemItem->setData(Qt::UserRole, "PoemID="+QString::number(poemId));

		int tmpWidth = searchTable->fontMetrics().boundingRect(snippedPoemTitle).width();
		if ( tmpWidth > maxPoemWidth )
			maxPoemWidth = tmpWidth;

//		QString cleanedFirstVerse = QGanjoorDbBrowser::cleanString(firstVerse/*, false*/);
//		QString snippedFirstVerse = QGanjoorDbBrowser::snippedText(cleanedFirstVerse, phrase, 0, 8, true);
//		if (snippedFirstVerse.isEmpty())
//			snippedFirstVerse = QGanjoorDbBrowser::snippedText(cleanedFirstVerse, "", 0, 8, true);

		//change 'cleanedFirstVerse' to 'firstVerse' maybe this conflicts with 'snippedText()' algorithm!
		QString snippedFirstVerse = QGanjoorDbBrowser::snippedText(firstVerse, phrase, 0, 8, true);
		if (snippedFirstVerse.isEmpty())
			snippedFirstVerse = QGanjoorDbBrowser::snippedText(firstVerse, "", 0, 8, true);

		QTableWidgetItem *verseItem = new QTableWidgetItem(snippedFirstVerse);
		verseItem->setFlags(Qt::ItemIsEnabled/*Qt::NoItemFlags*/);
		verseItem->setData(Qt::UserRole, "PoemID="+QString::number(poemId));
		//set search data
		verseItem->setData(ITEM_SEARCH_DATA, phrase);
		poemItem->setData(ITEM_SEARCH_DATA, phrase);

		//insert items to table
		searchTable->setItem(i-start, 1, poemItem);
		searchTable->setItem(i-start, 2, verseItem);

		tmpWidth = searchTable->fontMetrics().boundingRect(snippedFirstVerse).width();
		if ( tmpWidth > maxVerseWidth )
			maxVerseWidth = tmpWidth;
	}

	searchTable->setColumnWidth(1, maxPoemWidth+searchTable->fontMetrics().boundingRect("00").width() );
	searchTable->setColumnWidth(2, maxVerseWidth+searchTable->fontMetrics().boundingRect("00").width() );

	searchTable->setUpdatesEnabled(true);

	if (start > 0)
	{
		searchPreviousPage->setEnabled(true);
		actSearchPreviousPage->setData("actSearchPreviousPage|"+QString::number(start-SearchResultWidget::maxItemPerPage));
	}
	else
	{
		searchPreviousPage->setEnabled(false);
	}

	if (end < resultList.size()-1)
	{
		searchNextPage->setEnabled(true);
		actSearchNextPage->setData("actSearchNextPage|"+QString::number(end+1));
	}
	else
	{
		searchNextPage->setEnabled(false);
	}

	QApplication::restoreOverrideCursor();
}

void SearchResultWidget::searchPageNavigationClicked(QAction *action)
{
	if (!action) return;

	QVariant actionData = action->data();
	if ( !actionData.isValid() || actionData.isNull() ) return;
	QStringList dataList = actionData.toString().split("|", QString::SkipEmptyParts);
	if (dataList.size() == 2)
		showSearchResult(dataList.at(1).toInt());
}

void SearchResultWidget::maxItemPerPageChange(int value)
{
	//update 'actSearchPreviousPage' data object
	if (actSearchPreviousPage && searchPreviousPage)
	{
		bool enabled = searchPreviousPage->isEnabled();
		QVariant actionData = actSearchPreviousPage->data();
		if ( actionData.isValid() && !actionData.isNull() )
		{
			QStringList dataList = actionData.toString().split("|", QString::SkipEmptyParts);
			if (dataList.size() == 2)
			{
				int start = dataList.at(1).toInt()+SearchResultWidget::maxItemPerPage;
				actSearchPreviousPage->setData("actSearchPreviousPage|"+QString::number(start-value));
			}
		}
		searchPreviousPage->setEnabled(enabled);
	}

	SearchResultWidget::maxItemPerPage = value;
}

void SearchResultWidget::filterResults(const QString &text)
{
	if (text.isEmpty())
	{
		resultList = copyResultList;
		emit searchFiltered(phrase);
		showSearchResult(0);
		return;
	}//searchTable->setItemDelegateForColumn(2, new SaagharItemDelegate());

	//SaagharItemDelegate *itemDelegate = searchTable->itemDelegateForColumn(2);
	//itemDelegate->
	emit searchFiltered(phrase+" "+text);
	//QMap<int, QString> tmpList;

	resultList.clear();
	QMap<int, QString>::const_iterator it = copyResultList.constBegin();
	const QMap<int, QString>::const_iterator endIterator = copyResultList.constEnd();
	while (it != endIterator)
	{
		const QString value = it.value();
		if (value.contains(text, Qt::CaseInsensitive))
			resultList.insert(it.key(), value);
		++it;
	}
	if (resultList.isEmpty())
	{
		pageLabel->setText(tr("Nothing found!"));
		searchPreviousPage->setEnabled(false);
		searchNextPage->setEnabled(false);
		searchTable->clear();
		searchTable->setRowCount(0);
	}
	else
		showSearchResult(0);
}

//void SearchResultWidget::setMaxItemPerPage(int max)
//{
//	SearchResultWidget::maxItemPerPage = max;
//}
