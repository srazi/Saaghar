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

#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "SearchItemDelegate.h"
#include "version.h"
#include "settings.h"
#include "SearchResultWidget.h"
#include "SearchPatternManager.h"

#include <QTextBrowserDialog>
#include <QSearchLineEdit>
#include <QMultiSelectWidget>

#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QTableWidget>
#include <QFontDatabase>
#include <QClipboard>
#include <QProcess>
#include <QDir>
#include <QCloseEvent>
#include <QTextCodec>
#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QPrintPreviewDialog>
#include <QDockWidget>
#include <QDateTime>
#include <QInputDialog>
#include <QProgressDialog>
#include <QScrollBar>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

//const int ITEM_SEARCH_DATA = Qt::UserRole+10;

bool MainWindow::autoCheckForUpdatesState = true;

MainWindow::MainWindow(QWidget *parent, QObject *splashScreen, bool fresh) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	if (splashScreen)
		connect(this, SIGNAL(loadingStatusText(const QString &)), splashScreen, SLOT(showMessage(const QString &)));
	QCoreApplication::setOrganizationName("Pojh");
	QCoreApplication::setApplicationName("Saaghar");
	QCoreApplication::setOrganizationDomain("Pojh.iBlogger.org");

	setWindowIcon(QIcon(":/resources/images/saaghar.png"));

#ifdef D_MSVC_CC
	QTextCodec::setCodecForLocale( QTextCodec::codecForName( "Windows-1256" ) );
#endif
#ifdef D_MINGW_CC
	QTextCodec::setCodecForLocale( QTextCodec::codecForName( "utf-8" ) );
#endif

	SaagharWidget::persianIranLocal = QLocale(QLocale::Persian, QLocale::Iran);
	SaagharWidget::persianIranLocal.setNumberOptions(QLocale::OmitGroupSeparator);

	defaultPrinter = 0;

#ifdef Q_WS_MAC
	QFileInfo portableSettings(QCoreApplication::applicationDirPath()+"/../Resources/settings.ini");
#else
	QFileInfo portableSettings(QCoreApplication::applicationDirPath()+"/settings.ini");
#endif

	if ( portableSettings.exists() && portableSettings.isWritable() ) 
		isPortable = true;
	else
		isPortable = false;

	//Undo FrameWork
	undoGroup = new QUndoGroup(this);

	saagharWidget = 0;
	pressedMouseButton = Qt::LeftButton;
	
	const QString tempDataBaseName = "/ganjoor.s3db";//temp
	QString dataBaseCompleteName = "/ganjoor.s3db";

	if (isPortable)
	{
#ifdef Q_WS_MAC
		dataBaseCompleteName = QCoreApplication::applicationDirPath()+"/../Resources/"+dataBaseCompleteName;
		resourcesPath = QCoreApplication::applicationDirPath() + "/../Resources/";
#else
		dataBaseCompleteName = QCoreApplication::applicationDirPath()+dataBaseCompleteName;
		resourcesPath = QCoreApplication::applicationDirPath();
#endif
	}
	else
	{
#ifdef Q_WS_WIN
		dataBaseCompleteName = QDir::homePath()+"/Pojh/Saaghar/"+dataBaseCompleteName;
		resourcesPath = QCoreApplication::applicationDirPath();
#endif
#ifdef Q_WS_X11
		dataBaseCompleteName = "/usr/share/saaghar/"+dataBaseCompleteName;
		resourcesPath = "/usr/share/saaghar/";
#endif
#ifdef Q_WS_MAC
		dataBaseCompleteName = QDir::homePath()+"/Library/Saaghar/"+dataBaseCompleteName;
		resourcesPath = QCoreApplication::applicationDirPath() + "/../Resources/";
#endif
	}

	SaagharWidget::poetsImagesDir = resourcesPath + "/poets_images/";
	
	//loading application fonts
	//QString applicationFontsPath = resourcesPath + "/fonts/";
	QDir fontsDir(resourcesPath + "/fonts/");
	QStringList fontsList = fontsDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
	for (int i=0; i<fontsList.size(); ++i)
	{
		QFontDatabase::addApplicationFont(resourcesPath + "/fonts/"+fontsList.at(i));
	}

	ui->setupUi(this);

	//create Parent Categories ToolBar
	parentCatsToolBar = new QToolBar(this);
	parentCatsToolBar->setObjectName(QString::fromUtf8("parentCatsToolBar"));
	parentCatsToolBar->setLayoutDirection(Qt::RightToLeft);
	parentCatsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
	//addToolBar(Qt::BottomToolBarArea, parentCatsToolBar);
	insertToolBar(ui->mainToolBar, parentCatsToolBar);
	parentCatsToolBar->setWindowTitle(QApplication::translate("MainWindow", "Parent Categories", 0, QApplication::UnicodeUTF8));

	//create Tab Widget
	mainTabWidget = new QTabWidget(ui->centralWidget);
	mainTabWidget->setDocumentMode(true);
	mainTabWidget->setUsesScrollButtons(true);
	mainTabWidget->setMovable(true);

	emit loadingStatusText(tr("<i><b>Loading settings...</b></i>"));
	loadGlobalSettings();

	if (autoCheckForUpdatesState && !fresh)
	{
		emit loadingStatusText(tr("<i><b>Checking for update...</b></i>"));
		QCoreApplication::processEvents();
		checkForUpdates();
	}

	setupUi();

	//setup corner widget
	QToolButton *cornerButton = new QToolButton(mainTabWidget);
	cornerButton->setIcon(QIcon(":/resources/images/corner-button.png"));
	cornerButton->setPopupMode(QToolButton::InstantPopup);
	cornerButton->setStyleSheet("QToolButton::menu-indicator{image: none;}");
	QMenu *cornerMenu = new QMenu(0);
	QMenu *mainMenuAsSubMenu = new QMenu(tr("Main Menu"),cornerMenu);
	mainMenuAsSubMenu->addMenu(menuFile);
	mainMenuAsSubMenu->addMenu(menuNavigation);
	mainMenuAsSubMenu->addMenu(menuView);
	mainMenuAsSubMenu->addMenu(menuTools);
	mainMenuAsSubMenu->addMenu(menuHelp);

	cornerMenu->addMenu(mainMenuAsSubMenu);
	cornerMenu->addSeparator();

	cornerMenu->addAction(actionInstance("actionNewTab"));
	cornerMenu->addAction(actionInstance("actionNewWindow"));
	cornerMenu->addSeparator();
	cornerMenu->addMenu(menuOpenedTabs);
	cornerMenu->addMenu(menuClosedTabs);
	cornerButton->setMenu(cornerMenu);
	mainTabWidget->setCornerWidget(cornerButton);

	if (windowState() & Qt::WindowFullScreen)
	{
		actionInstance("actionFullScreen")->setChecked(true);
		actionInstance("actionFullScreen")->setText(tr("Exit &Full Screen"));
		actionInstance("actionFullScreen")->setIcon(QIcon(currentIconThemePath()+"/no-fullscreen.png"));
	}
	else
	{
		actionInstance("actionFullScreen")->setChecked(false);
		actionInstance("actionFullScreen")->setText(tr("&Full Screen"));
		actionInstance("actionFullScreen")->setIcon(QIcon(currentIconThemePath()+"/fullscreen.png"));
	}

	//searchin database-path for database-file
	for (int i=0; i<QGanjoorDbBrowser::dataBasePath.size(); ++i)
	{
		if ( QFile::exists(QGanjoorDbBrowser::dataBasePath.at(i)+tempDataBaseName) )
		{
			dataBaseCompleteName = QGanjoorDbBrowser::dataBasePath.at(i)+tempDataBaseName;
			break;
		}
	}
	//create ganjoor DataBase browser
	SaagharWidget::ganjoorDataBase = new QGanjoorDbBrowser(dataBaseCompleteName);
	SaagharWidget::ganjoorDataBase->setObjectName(QString::fromUtf8("ganjoorDataBaseBrowser"));

	loadTabWidgetSettings();

	ui->gridLayout->setContentsMargins(0,0,0,0);
	
	ui->gridLayout->addWidget(mainTabWidget, 0, 0, 1, 1);

	if (!fresh)
	{
		QStringList openedTabs = Settings::READ("Opened tabs from last session").toStringList();
		for (int i=0; i<openedTabs.size(); ++i)
		{
			QStringList tabViewData = openedTabs.at(i).split("=", QString::SkipEmptyParts);
			if (tabViewData.size() == 2 && (tabViewData.at(0) == "PoemID" || tabViewData.at(0) == "CatID") )
			{
				bool Ok = false;
				int id = tabViewData.at(1).toInt(&Ok);
				if (Ok)
					newTabForItem(tabViewData.at(0), id, true);
			}
		}
	}

	if (mainTabWidget->count() < 1)
		insertNewTab();

	ui->centralWidget->setLayoutDirection(Qt::RightToLeft);

	connect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
	connect(mainTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloser(int)));

	connect(ui->mainToolBar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(toolBarContextMenu(QPoint)));
	//connect(ui->searchToolBar, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(searchToolBarView()));
	ui->searchToolBar->installEventFilter(this);

	createConnections();
	previousTabIndex = mainTabWidget->currentIndex();
	currentTabChanged(mainTabWidget->currentIndex());

	QListWidgetItem *item = selectSearchRange->insertRow(0, tr("All Opened Tab"), true, "ALL_OPENED_TAB", Qt::UserRole, true);
	multiSelectObjectInitialize(selectSearchRange, selectedSearchRange);
	item->setCheckState(selectedSearchRange.contains("ALL_OPENED_TAB") ? Qt::Checked : Qt::Unchecked);

	//seeding random function
	uint numOfSecs = QDateTime::currentDateTime().toTime_t();
	uint seed = QCursor::pos().x()+QCursor::pos().y()+numOfSecs+QDateTime::currentDateTime().time().msec();
	qsrand(seed);

	//install search pattern manager
	SearchPatternManager *searchPatternManager = new SearchPatternManager();
	searchPatternManager->setWildcardCharacter("%");

	emit loadingStatusText(tr("<i><b>Saaghar is starting...</b></i>"));
}

MainWindow::~MainWindow()
{
	delete SaagharWidget::ganjoorDataBase;
	delete ui;
}

void MainWindow::searchStart()
{
	if (!lineEditSearchText->searchWasCanceled())
		return;

	QString phrase = lineEditSearchText->text();
	phrase.remove(" ");
	phrase.remove("\t");
	if (phrase.isEmpty())
	{
		lineEditSearchText->setText("");
		QMessageBox::information(this, tr("Error"), tr("The search phrase can not be empty."));
		return;
	}
	phrase = lineEditSearchText->text();
	phrase.replace(QChar(0x200C), "", Qt::CaseInsensitive);//replace ZWNJ by ""

	//QString currentItemData = comboBoxSearchRegion->itemData(comboBoxSearchRegion->currentIndex(), Qt::UserRole).toString();

	QList<QListWidgetItem *> selectList = selectSearchRange->getSelectedItemList();

	int numOfResults = 0;
	bool searchCanceled = false;
	bool slowSearch = false;//more calls of 'processEvents'
	if (selectList.size()>1)
		slowSearch = true;

	for (int i=0; i<selectList.size(); ++i)
	{
		QVariant currentItemData = selectList.at(i)->data(Qt::UserRole);
		//QString currentItemStrData = currentItemData.toString();

		if (currentItemData.toString() == "ALL_OPENED_TAB")
		{
			for (int j = 0; j < mainTabWidget->count(); ++j)
			{
				SaagharWidget *tmp = getSaagharWidget(j);
				//QAbstractItemDelegate *tmpDelegate = tmp->tableViewWidget->itemDelegate();
	
				//delete tmpDelegate;
				//tmpDelegate = 0;
				if (tmp)
					tmp->scrollToFirstItemContains(phrase);
				//tmp->tableViewWidget->setItemDelegate(new SaagharItemDelegate(tmp->tableViewWidget, saagharWidget->tableViewWidget->style(), lineEditSearchText->text()));
			}
		}
		//else if (currentItemData == "CURRENT_TAB")
		//{
		//	if (saagharWidget)
		//	{
		//		//connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), saagharWidget, SLOT(scrollToFirstItemContains(const QString &)) );
		//		//QAbstractItemDelegate *currentDelegate = saagharWidget->tableViewWidget->itemDelegate();
		//		saagharWidget->scrollToFirstItemContains(lineEditSearchText->text());
		//		//SaagharItemDelegate *tmp = new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), lineEditSearchText->text());
		//		//saagharWidget->tableViewWidget->setItemDelegate(tmp);
		//		//connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), tmp, SLOT(keywordChanged(const QString &)) );
		//		//delete currentDelegate;
		//		//currentDelegate = 0;
		//	}
		//}
		else
		{
			//phrase = QGanjoorDbBrowser::cleanString(lineEditSearchText->text(), SaagharWidget::newSearchSkipNonAlphabet);
			//int poetID = comboBoxSearchRegion->itemData(comboBoxSearchRegion->currentIndex(), Qt::UserRole).toInt();
			SearchPatternManager::setInputPhrase(phrase);
			SearchPatternManager::init();
			QVector<QStringList> phraseVectorList = SearchPatternManager::outputPhrases();
			QVector<QStringList> excludedVectorList = SearchPatternManager::outputExcludedLlist();
			int vectorSize = phraseVectorList.size();
//			qDebug() << "SearchPatternManager::finished!";
//			qDebug() << "phraseVectorList=" <<phraseVectorList;
//			qDebug() << "excludedVectorList="<<excludedVectorList;

			bool ok = false;
			int poetID = currentItemData.toInt(&ok);
			if (!ok)
				continue;//itemData is not int skip to next item

			QString poetName = "";
			if (poetID == 0)
				poetName = tr("All");
			else
			{
				//because of 'ok == true' we know this is  poet name
				poetName = selectList.at(i)->text();
				//SaagharWidget::ganjoorDataBase->getPoet(poetID)._Name;
			}

			QWidget *searchResultContents = new QWidget(this);
			searchResultContents->setObjectName(QString::fromUtf8("searchResultContents"));
			searchResultContents->setAttribute(Qt::WA_DeleteOnClose, true);
			//searchResultContents->setLayoutDirection(Qt::RightToLeft);

			SearchResultWidget *searchResultWidget = new SearchResultWidget(searchResultContents, lineEditSearchText->text(), SearchResultWidget::maxItemPerPage, poetName);

			/////////////////////////////////////
			QMap<int, QString> finalResult;
//			QProgressDialog searchProgress(tr("Searching Data Base..."),  tr("Cancel"), 0, 0, this);
//			connect( &searchProgress, SIGNAL( canceled() ), &searchProgress, SLOT( hide() ) );
//			searchProgress.setWindowModality(Qt::WindowModal);
//			searchProgress.setFixedSize(searchProgress.size());
//			searchProgress.setMinimumDuration(0);
//			//searchProgress.setValue(0);
//			//searchProgress.show();
			
			lineEditSearchText->searchStart(&searchCanceled);
			connect(SaagharWidget::ganjoorDataBase, SIGNAL(searchStatusChanged(const QString &)), lineEditSearchText, SLOT(setSearchProgressText(const QString &)));
			//emit SaagharWidget::ganjoorDataBase->searchStatusChanged(tr("Searching Data Base..."));
			lineEditSearchText->setSearchProgressText(tr("Searching Data Base(subset= %1)...").arg(poetName));
			QApplication::processEvents();
			int resultCount = 0;
			for (int j=0;j<vectorSize;++j)
			{
				QStringList phrases = phraseVectorList.at(j);
//				int phraseCount = phrases.size();
				QStringList excluded = excludedVectorList.at(j);

				QMap<int, QString> mapResult = SaagharWidget::ganjoorDataBase->getPoemIDsByPhrase(poetID, phrases, excluded, &searchCanceled, resultCount, slowSearch);

				QMap<int, QString>::const_iterator it = mapResult.constBegin();

//				qDebug() << "mapResult-size" << mapResult.size();
				while (it != mapResult.constEnd())
				{
					finalResult.insert(it.key(), it.value());//insertMulti for more than one result from a poem
					++it;
				}
				resultCount = finalResult.size();
//				qDebug() << "resultCount=" << resultCount;

				QApplication::processEvents();

				qDebug() << "finalResult-size" << finalResult.size();
				if (searchCanceled)
					break;
			}

			numOfResults+=resultCount;

			if (i == selectList.size()-1)
				lineEditSearchText->searchStop();
			///////////////////////////////////////

			searchResultWidget->setResultList( finalResult );
			if ( !searchResultWidget->init(this, currentIconThemePath()) )
			{
				if ( numOfResults == 0 && (i == selectList.size()-1) )
				{
					//QMessageBox::information(this, tr("Search"), tr("Current Scope: %1\nNo match found.").arg(poetName));
					lineEditSearchText->notFound();
					connect(selectSearchRange, SIGNAL(itemCheckStateChanged(QListWidgetItem*)), lineEditSearchText, SLOT(resetNotFound()));
					lineEditSearchText->setSearchProgressText(tr("Current Scope: %1\nNo match found.").arg(selectSearchRange->getSelectedStringList().join("-")));
				}
				delete searchResultWidget;
				searchResultWidget = 0;

				if (searchCanceled)
					break;//search is canceled
				else
					continue;
			}

			connect(this, SIGNAL(maxItemPerPageChanged(int)), searchResultWidget, SLOT(maxItemPerPageChange(int)));

			//create connections for mouse signals
			connect(searchResultWidget->searchTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClick(QTableWidgetItem *)));
			connect(searchResultWidget->searchTable, SIGNAL(itemPressed(QTableWidgetItem *)), this, SLOT(tableItemPress(QTableWidgetItem *)));

			//resize table on create and destroy
//			emitReSizeEvent();
//			connect(searchResultWidget, SIGNAL(destroyed()), this, SLOT(emitReSizeEvent()));
		}
		//we should create conection first and then break!
		if (searchCanceled)
			break;//search is canceled
	}
}

void MainWindow::multiSelectObjectInitialize(QMultiSelectWidget *multiSelectWidget, const QStringList &selectedData)
{
	QListWidgetItem *rootItem = multiSelectWidget->insertRow(1, tr("All"), true, "0", Qt::UserRole);

	QList<GanjoorPoet *> poets = SaagharWidget::ganjoorDataBase->getPoets();
	
	//int insertIndex = count();
	for(int i=0; i<poets.size(); ++i)
	{
		multiSelectWidget->insertRow(i+2, poets.at(i)->_Name, true,
			QString::number(poets.at(i)->_ID), Qt::UserRole, false, rootItem)->setCheckState(selectedData.contains(QString::number(poets.at(i)->_ID)) ? Qt::Checked : Qt::Unchecked);
	}

	if (selectedData.contains("0"))
		rootItem->setCheckState(Qt::Checked);

	multiSelectWidget->updateSelectedLists();
}

void MainWindow::actionRemovePoet()
{
	bool ok = false;
	QStringList items;
	items << tr("Select a name...");
	QList<GanjoorPoet *> poets = SaagharWidget::ganjoorDataBase->getPoets();
	for (int i=0; i<poets.size(); ++i)
		items << poets.at(i)->_Name+"("+tr("poet's code=")+QString::number(poets.at(i)->_ID)+")";
	QString item = QInputDialog::getItem(this, tr("Remove Poet"), tr("Select a poet name and click on 'OK' button, for remove it from database."), items, 0, false, &ok);
	if (ok && !item.isEmpty() && item != tr("Select a name...") )
	{ 
		qDebug() <<"item="<< item;
		qDebug() <<"int="<< item.toInt();
		qDebug() <<"Int-itemleft="<< item.mid(item.indexOf("("+tr("poet's code="))+tr("poet's code=").size()+1).remove(")").toInt();
		
		//int poetID = SaagharWidget::ganjoorDataBase->getPoet(item.left(item.indexOf("(")))._ID;
		int poetID = item.mid(item.indexOf("("+tr("poet's code="))+tr("poet's code=").size()+1).remove(")").toInt();
		if (poetID>0)
		{
			QMessageBox warnAboutDelete(this);
			warnAboutDelete.setWindowTitle(tr("Please Notice!"));
			warnAboutDelete.setIcon(QMessageBox::Warning);
			warnAboutDelete.setText(tr("Are you sure for removing \"%1\", from database?").arg(SaagharWidget::ganjoorDataBase->getPoet(poetID)._Name/*item*/));
			warnAboutDelete.addButton(tr("Continue"), QMessageBox::AcceptRole);
			warnAboutDelete.setStandardButtons(QMessageBox::Cancel);
			warnAboutDelete.setEscapeButton(QMessageBox::Cancel);
			warnAboutDelete.setDefaultButton(QMessageBox::Cancel);
			int ret = warnAboutDelete.exec();
			if ( ret == QMessageBox::Cancel)
				return;

			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			SaagharWidget::ganjoorDataBase->removePoetFromDataBase(poetID);
			comboBoxSearchRegion->clear();
			QListWidgetItem *item = selectSearchRange->insertRow(0, tr("All Opened Tab"), true, "ALL_OPENED_TAB", Qt::UserRole, true);
			multiSelectObjectInitialize(selectSearchRange, selectedSearchRange);
			item->setCheckState(selectedSearchRange.contains("ALL_OPENED_TAB") ? Qt::Checked : Qt::Unchecked);
			//update visible region of searchToolBar
//			bool tmpFlag = labelMaxResultAction->isVisible();
//			labelMaxResultAction->setVisible(!tmpFlag);
//			ui->searchToolBar->update();
//			QApplication::processEvents();
//			labelMaxResultAction->setVisible(tmpFlag);
			setHomeAsDirty();
			QApplication::restoreOverrideCursor();
		}
	}
}

void MainWindow::currentTabChanged(int tabIndex)
{
	SaagharWidget *tmpSaagharWidget = getSaagharWidget(tabIndex);
	if ( tmpSaagharWidget )
	{
		saagharWidget = tmpSaagharWidget;
		saagharWidget->loadSettings();

		if ( saagharWidget->isDirty() )
		{
			saagharWidget->refresh();
		}
		saagharWidget->showParentCategory(SaagharWidget::ganjoorDataBase->getCategory(saagharWidget->currentCat));//just update parentCatsToolbar
		saagharWidget->resizeTable(saagharWidget->tableViewWidget);

		connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), saagharWidget, SLOT(scrollToFirstItemContains(const QString &)) );

		if (previousTabIndex != tabIndex)
		{
			tmpSaagharWidget = getSaagharWidget(previousTabIndex);
			if (tmpSaagharWidget)
			{
				disconnect(lineEditSearchText, SIGNAL(textChanged(const QString &)), tmpSaagharWidget, SLOT(scrollToFirstItemContains(const QString &)) );
			}
			previousTabIndex = tabIndex;
		}

		if (saagharWidget->tableViewWidget->rowCount() > 0 && saagharWidget->currentCat != 0)
		{
			if (saagharWidget->currentPoem == 0)
			{
				QTableWidgetItem *item = saagharWidget->tableViewWidget->item(0,0);
				if (item && !item->icon().isNull() && saagharWidget->tableViewWidget->columnCount() == 1)
				{
					QString text = item->text();
					int textWidth = saagharWidget->tableViewWidget->fontMetrics().boundingRect(text).width();
					int verticalScrollBarWidth=0;
					if ( saagharWidget->tableViewWidget->verticalScrollBar()->isVisible() )
					{
						verticalScrollBarWidth=saagharWidget->tableViewWidget->verticalScrollBar()->width();
					}
					//int totalWidth = saagharWidget->tableViewWidget->columnWidth(0)-verticalScrollBarWidth-82;
					int totalWidth = saagharWidget->tableViewWidget->viewport()->width()-verticalScrollBarWidth-82;
					totalWidth = qMax(82+verticalScrollBarWidth, totalWidth);
					saagharWidget->tableViewWidget->setRowHeight(0, qMax(100, SaagharWidget::computeRowHeight(saagharWidget->tableViewWidget->fontMetrics(), textWidth, totalWidth)) );
					//saagharWidget->tableViewWidget->setRowHeight(0, 2*saagharWidget->tableViewWidget->rowHeight(0)+(saagharWidget->tableViewWidget->fontMetrics().height()*(numOfRow/*+1*/)));
				}
			}
			else if(saagharWidget->tableViewWidget->columnCount()>1)
			{
				QTableWidgetItem *item = saagharWidget->tableViewWidget->item(0,1);
				if (item)
				{
					QString text = item->text();
					int textWidth = saagharWidget->tableViewWidget->fontMetrics().boundingRect(text).width();
					int verticalScrollBarWidth=0;
					if ( saagharWidget->tableViewWidget->verticalScrollBar()->isVisible() )
					{
						verticalScrollBarWidth=saagharWidget->tableViewWidget->verticalScrollBar()->width();
					}
					int totalWidth = saagharWidget->tableViewWidget->viewport()->width();
					saagharWidget->tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(saagharWidget->tableViewWidget->fontMetrics(), textWidth, totalWidth) );
				}
			}
		}

		undoGroup->setActiveStack(saagharWidget->undoStack);
		updateCaption();
		updateTabsSubMenus();

		actionInstance("actionPreviousPoem")->setEnabled( !SaagharWidget::ganjoorDataBase->getPreviousPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull() );
		actionInstance("actionNextPoem")->setEnabled( !SaagharWidget::ganjoorDataBase->getNextPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull() );
	}
}

SaagharWidget *MainWindow::getSaagharWidget(int tabIndex)
{
	if (tabIndex == -1) return 0;
	QWidget *curTabContent = mainTabWidget->widget(tabIndex);
	if (!curTabContent) return 0;
	QObjectList tabContentList = curTabContent->children();

	for (int i=0; i< tabContentList.size(); ++i)
	{
		if (qobject_cast<SaagharWidget *>(tabContentList.at(i)))
		{
			return qobject_cast<SaagharWidget *>(tabContentList.at(i));
		}
	}
	return 0;
}

void MainWindow::checkForUpdates()
{
	QAction *action = qobject_cast<QAction *>(sender());

	QEventLoop loop;
	
	QNetworkRequest requestVersionInfo(QUrl("http://saaghar.sourceforge.net/saaghar.version"));
	QNetworkAccessManager *netManager = new QNetworkAccessManager();
	QNetworkReply *reply = netManager->get(requestVersionInfo);
	
	QProgressDialog updateProgress(tr("Checking for updates..."),  tr("Cancel"), 0, 0, this);
	if (action)
	{
		updateProgress.setMinimumDuration(0);
	}
	else
	{
		updateProgress.setParent(0);
		updateProgress.setMinimumDuration(5000);
	}

	updateProgress.setWindowModality(Qt::WindowModal);
	updateProgress.setFixedSize(updateProgress.size());
	if (action) updateProgress.show();

	connect( &updateProgress, SIGNAL( canceled() ), &loop, SLOT( quit() ) );
	connect( reply, SIGNAL( finished() ), &updateProgress, SLOT( hide() ) );
	connect( reply, SIGNAL( finished() ), &loop, SLOT( quit() ) );
	loop.exec();

	if (updateProgress.wasCanceled())
	{
		updateProgress.hide();
		loop.quit();
		return;
	}

	if( reply->error() )
	{
		emit loadingStatusText("!QTransparentSplashInternalCommands:HIDE");
		QMessageBox criticalError(QMessageBox::Critical, tr("Error"), tr("There is an error when checking for updates...\nError: %1").arg(reply->errorString()), QMessageBox::Ok, this, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint );
		criticalError.exec();
		//QMessageBox::critical(this, tr("Error"), tr("There is an error when checking for updates...\nError: %1").arg(reply->errorString()));
		emit loadingStatusText("!QTransparentSplashInternalCommands:SHOW");
		return;
	}
	QStringList data = QString::fromUtf8( reply->readAll() ).split("|", QString::SkipEmptyParts);

	QMap<QString, QString> releaseInfo;
	for (int i=0; i<data.size(); ++i)
	{
		QStringList fields = data.at(i).split("*", QString::SkipEmptyParts);
		if (fields.size() == 2)
			releaseInfo.insert(fields.at(0), fields.at(1));
	}
	
	if (releaseInfo.value("VERSION").isEmpty()) return;
	QString runningAppVersionStr = QString(VER_FILEVERSION_STR);
	runningAppVersionStr.remove('.');
	int runningAppVersion = runningAppVersionStr.toInt();
	QString serverAppVerionStr = releaseInfo.value("VERSION");
	serverAppVerionStr.remove('.');
	int serverAppVerion = serverAppVerionStr.toInt();
	
	if (serverAppVerion > runningAppVersion)
	{
		emit loadingStatusText("!QTransparentSplashInternalCommands:CLOSE");
		QMessageBox updateIsAvailable(this);
		updateIsAvailable.setWindowFlags(updateIsAvailable.windowFlags() | Qt::WindowStaysOnTopHint);
		updateIsAvailable.setTextFormat(Qt::RichText);
		updateIsAvailable.setWindowTitle(tr("New Saaghar Version Available"));
		updateIsAvailable.setIcon(QMessageBox::Information);
		QString newVersionInfo = "";
		if ( !releaseInfo.value("infoLinks").isEmpty() )
		{
			QStringList infoLinks = releaseInfo.value("infoLinks").split('\\', QString::SkipEmptyParts);
			if (!infoLinks.isEmpty())
			{
				newVersionInfo = infoLinks.join("<br />");
			}
		}
		updateIsAvailable.setText(tr("The Version <strong>%1</strong> of Saaghar is available for download.<br />%2<br />Do you want to browse download page?").arg(releaseInfo.value("VERSION")).arg(newVersionInfo));
		updateIsAvailable.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
		updateIsAvailable.setEscapeButton(QMessageBox::Cancel);
		updateIsAvailable.setDefaultButton(QMessageBox::Ok);
		QString downloadLinkKey = "OTHERS";

#ifdef Q_WS_WIN
	downloadLinkKey = "WINDOWS";
#endif
#ifdef Q_WS_X11
	downloadLinkKey = "LINUX";
#endif
#ifdef Q_WS_MAC
	downloadLinkKey = "MACOSX";
#endif
		int ret = updateIsAvailable.exec();
		if ( ret == QMessageBox::Ok)
			QDesktopServices::openUrl(QUrl( releaseInfo.value(downloadLinkKey, "http://pojh.iblogger.org/saaghar/downloads/") ));
		else return; 
	}
	else
	{
		if (action)
			QMessageBox::information(this, tr("Saaghar is up to date"), tr("There is no new version available. Please check for updates later!") );
	}
}

void MainWindow::tabCloser(int tabIndex)
{
	if (tabIndex == mainTabWidget->currentIndex())
		SaagharWidget::lastOveredItem = 0;
	//if EditMode app need to ask about save changes!
	disconnect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

	QAction *tabAct = new QAction(mainTabWidget->tabText(tabIndex), menuClosedTabs);
	SaagharWidget *tmp = getSaagharWidget(tabIndex);
	tabAct->setData( tmp->identifier());
	connect(tabAct, SIGNAL(triggered()), this, SLOT(actionClosedTabsClicked()) );
	menuClosedTabs->addAction(tabAct);

//	QAction *tabAction = new QAction(mainTabWidget->tabText(i), menuOpenedTabs);
//	tabAction->setData(mainTabWidget->widget(i) /*QString::number(i)*/);
//	menuOpenedTabs->addAction(tabAction);

	QWidget *closedTabContent = mainTabWidget->widget(tabIndex);
	mainTabWidget->setUpdatesEnabled(false);
	mainTabWidget->removeTab(tabIndex);
	mainTabWidget->setUpdatesEnabled(true);
	
	delete closedTabContent;

	if (mainTabWidget->count() == 1) //there is just one tab present.
		mainTabWidget->setTabsClosable(false);
	currentTabChanged(mainTabWidget->currentIndex());//maybe we don't need this line as well disconnect and connect.
	connect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
}

void MainWindow::insertNewTab()
{
	QWidget *tabContent = new QWidget();
	QGridLayout *tabGridLayout = new QGridLayout(tabContent);
	tabGridLayout->setObjectName(QString::fromUtf8("tabGridLayout"));
	QTableWidget *tabTableWidget = new QTableWidget(tabContent);

	//table defaults
	tabTableWidget->setObjectName(QString::fromUtf8("mainTableWidget"));
	tabTableWidget->setLayoutDirection(Qt::RightToLeft);
	tabTableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	tabTableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	tabTableWidget->setShowGrid(false);
	tabTableWidget->setGridStyle(Qt::NoPen);
	tabTableWidget->setFrameShape(QFrame::NoFrame);
	tabTableWidget->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
	tabTableWidget->horizontalHeader()->setVisible(false);
	tabTableWidget->verticalHeader()->setVisible(false);
	tabTableWidget->setMouseTracking(true);
	tabTableWidget->setAutoScroll(false);
	//install event filter on viewport
	tabTableWidget->viewport()->installEventFilter(this);
	//install 'delegate' on QTableWidget
	QAbstractItemDelegate *tmpDelegate = tabTableWidget->itemDelegate();
	delete tmpDelegate;
	tmpDelegate = 0;

	SaagharItemDelegate *searchDelegate = new SaagharItemDelegate(tabTableWidget, tabTableWidget->style());
	tabTableWidget->setItemDelegate(searchDelegate);
	connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), searchDelegate, SLOT(keywordChanged(const QString &)) );

	//tmp->scrollToFirstItemContains(lineEditSearchText->text());
	//tabTableWidget->setItemDelegate(new SaagharItemDelegate(tabTableWidget, tabTableWidget->style()));
	///////

	saagharWidget = new SaagharWidget( tabContent, parentCatsToolBar, tabTableWidget);
	saagharWidget->setObjectName(QString::fromUtf8("saagharWidget"));

	undoGroup->addStack(saagharWidget->undoStack);
	//currentTabChanged() does this
	//undoGroup->setActiveStack(saagharWidget->undoStack);

	connect(saagharWidget, SIGNAL(loadingStatusText(QString)), this, SIGNAL(loadingStatusText(QString)));

	//connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), saagharWidget, SLOT(scrollToFirstItemContains(const QString &)) );

	connect(saagharWidget, SIGNAL(captionChanged()), this, SLOT(updateCaption()));
	//temp
	connect(saagharWidget, SIGNAL(captionChanged()), this, SLOT(updateTabsSubMenus()));

	connect(saagharWidget->tableViewWidget, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClick(QTableWidgetItem *)));
	connect(saagharWidget->tableViewWidget, SIGNAL(itemPressed(QTableWidgetItem *)), this, SLOT(tableItemPress(QTableWidgetItem *)));
	connect(saagharWidget->tableViewWidget, SIGNAL(itemEntered(QTableWidgetItem *)), this, SLOT(tableItemMouseOver(QTableWidgetItem *)));
	connect(saagharWidget->tableViewWidget, SIGNAL(currentItemChanged(QTableWidgetItem *,QTableWidgetItem *)), this, SLOT(tableCurrentItemChanged(QTableWidgetItem *,QTableWidgetItem *)));
	
	//Enable/Disable navigation actions
	connect(saagharWidget, SIGNAL(navPreviousActionState(bool)),	actionInstance("actionPreviousPoem"), SLOT(setEnabled(bool)) );
	connect(saagharWidget, SIGNAL(navNextActionState(bool)),	actionInstance("actionNextPoem"), SLOT(setEnabled(bool)) );
	actionInstance("actionPreviousPoem")->setEnabled( !SaagharWidget::ganjoorDataBase->getPreviousPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull() );
	actionInstance("actionNextPoem")->setEnabled( !SaagharWidget::ganjoorDataBase->getNextPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull() );
	
	//Updating table on changing of selection
	connect(saagharWidget->tableViewWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectChanged()));

	tabGridLayout->setContentsMargins(3,4,3,3);
	tabGridLayout->addWidget(tabTableWidget, 0, 0, 1, 1);

	mainTabWidget->setUpdatesEnabled(false);

	mainTabWidget->setCurrentIndex(mainTabWidget->addTab(tabContent,""));//add and gets focus to it
	if (mainTabWidget->count() > 1)
		mainTabWidget->setTabsClosable(true);//there are two or more tabs opened

	mainTabWidget->setUpdatesEnabled(true);
}

void MainWindow::newTabForItem(QString type, int id, bool noError)
{
	insertNewTab();
	saagharWidget->processClickedItem(type, id, noError);
	emit loadingStatusText(tr("<i><b>\"%1\" was loaded!</b></i>").arg(QGanjoorDbBrowser::snippedText(saagharWidget->currentCaption.mid(saagharWidget->currentCaption.lastIndexOf(":")+1), "", 0, 6, false, Qt::ElideRight)));
	//qDebug() << "emit updateTabsSubMenus()--848";
	//resolved by signal SaagharWidget::captionChanged()
	//updateTabsSubMenus();
}

void MainWindow::updateCaption()
{
	QString newTabCaption = QGanjoorDbBrowser::snippedText(saagharWidget->currentCaption, "", 0, 6, true, Qt::ElideRight)+QString(QChar(0x200F));
	mainTabWidget->setTabText(mainTabWidget->currentIndex(), newTabCaption );
	setWindowTitle(QString(QChar(0x202B))+QString::fromLocal8Bit("ساغر")+":"+saagharWidget->currentCaption+QString(QChar(0x202C)));
}

void MainWindow::tableSelectChanged()
{//Bug in Qt: old selection is not repainted properly
	if (saagharWidget)
		saagharWidget->tableViewWidget->viewport()->update();
}

void MainWindow::actionExportAsPDFClicked()
{
	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setColorMode(QPrinter::Color);
	printer.setPageSize(QPrinter::A4);
	printer.setCreator("Saaghar");
	printer.setPrinterName("Saaghar");
	printer.setFontEmbeddingEnabled(true);
	printPreview(&printer);
}

void MainWindow::actionExportClicked()
{
	QString exportedFileName = QFileDialog::getSaveFileName(this, tr("Export As"), QDir::homePath(), "HTML Document (*.html);;TeX - XePersian (*.tex);;Tab Separated (*.csv);;UTF-8 Text (*.txt)");
	if (exportedFileName.endsWith(".html", Qt::CaseInsensitive))
	{
		QString poemAsHTML = convertToHtml(saagharWidget);
		if (!poemAsHTML.isEmpty())
		{
			writeToFile(exportedFileName, poemAsHTML);
		}
		return;
	}

	if (exportedFileName.endsWith(".txt", Qt::CaseInsensitive))
	{
		QString tableAsText = tableToString(saagharWidget->tableViewWidget, "          "/*ten blank spaces*/, "\n", 0, 0, saagharWidget->tableViewWidget->rowCount(), saagharWidget->tableViewWidget->columnCount() );
		writeToFile(exportedFileName, tableAsText);
		return;
	}

	if (exportedFileName.endsWith(".csv", Qt::CaseInsensitive))
	{
		QString tableAsText = tableToString(saagharWidget->tableViewWidget, "\t", "\n", 0, 0, saagharWidget->tableViewWidget->rowCount(), saagharWidget->tableViewWidget->columnCount() );
		writeToFile(exportedFileName, tableAsText);
		return;
	}

	if (exportedFileName.endsWith(".tex", Qt::CaseInsensitive))
	{
		QString poemAsTeX = convertToTeX(saagharWidget);
		if (!poemAsTeX.isEmpty())
		{
			writeToFile(exportedFileName, poemAsTeX);
		}
		return;
	}
}

void MainWindow::writeToFile(QString fileName, QString textToWrite)
{
	QFile exportFile(fileName);
	if ( !exportFile.open( QIODevice::WriteOnly ) )
		{
		QMessageBox::warning( this,tr("Error"),tr("The file could not be saved. Please check if you have write permission."));
		return;
		}
	QTextStream out( &exportFile );
	out.setCodec("UTF-8");
	out << textToWrite;
	exportFile.close();
}

void MainWindow::print(QPrinter *printer)
{
	QPainter painter;
	painter.begin(printer);

	const int rows = saagharWidget->tableViewWidget->rowCount();
	const int columns = saagharWidget->tableViewWidget->columnCount();
	double totalWidth = 0.0;
	double currentHeight = 10.0;

	if (columns == 4)
	{
		totalWidth = qMax(saagharWidget->tableViewWidget->columnWidth(3), saagharWidget->minMesraWidth)+saagharWidget->tableViewWidget->columnWidth(2)+qMax(saagharWidget->tableViewWidget->columnWidth(1), saagharWidget->minMesraWidth);
	}
	else if (columns == 1)
	{
		totalWidth = saagharWidget->tableViewWidget->columnWidth(0);
	}
	int pageWidth = printer->pageRect().width();
	int pageHeight = printer->pageRect().height();

	const int extendedWidth = SaagharWidget::tableFont.pointSize()*4;
	const qreal scale = pageWidth/(totalWidth+extendedWidth );//printer->logicalDpiX() / 96;
	painter.scale(scale, scale);

	for (int row = 0; row < rows; ++row)
	{
		int thisRowHeight = saagharWidget->tableViewWidget->rowHeight(row);
		for (int col = 0; col < columns; ++col)
		{
			QTableWidgetItem *item = saagharWidget->tableViewWidget->item(row, col);
			int span = saagharWidget->tableViewWidget->columnSpan(row, col);
			QString text = "";

			if (item)
			{
				int rectX1=0;
				int rectX2=0;
				if (columns == 4)
				{
					if (span == 1)
					{
						if (col == 1)
						{
							rectX1 = saagharWidget->tableViewWidget->columnWidth(3)+saagharWidget->tableViewWidget->columnWidth(2);
							rectX2 = saagharWidget->tableViewWidget->columnWidth(3)+saagharWidget->tableViewWidget->columnWidth(2)+saagharWidget->tableViewWidget->columnWidth(1);
						}
						else if (col == 3)
						{
							rectX1 = 0;
							rectX2 = saagharWidget->tableViewWidget->columnWidth(3);
						}
					}
					else
					{
						rectX1 = 0;
						rectX2 = saagharWidget->tableViewWidget->columnWidth(3)+saagharWidget->tableViewWidget->columnWidth(2)+saagharWidget->tableViewWidget->columnWidth(1);
						int textWidth = saagharWidget->tableViewWidget->fontMetrics().boundingRect(item->text()).width();
						int numOfLine = textWidth/rectX2;
						thisRowHeight = thisRowHeight+(saagharWidget->tableViewWidget->fontMetrics().height()*numOfLine);
					}
				}
				else if (columns == 1)
				{
					rectX1 = 0;
					rectX2 = saagharWidget->tableViewWidget->columnWidth(0);
				}

				QRect mesraRect;
				mesraRect.setCoords(rectX1,currentHeight, rectX2, currentHeight+thisRowHeight);
				col = col+span-1;
				text = item->text();

				QFont fnt = SaagharWidget::tableFont;
				fnt.setPointSizeF(fnt.pointSizeF()/scale);
				painter.setFont(fnt);
				painter.setPen(QColor(SaagharWidget::textColor));
				painter.setLayoutDirection(Qt::RightToLeft);
				int flags = item->textAlignment();
				if (span > 1)
					flags |= Qt::TextWordWrap;
				painter.drawText(mesraRect, flags, text);
			}
		}

		currentHeight += thisRowHeight;
		if ((currentHeight+50)*scale >= pageHeight)
		{
			printer->newPage();
			currentHeight = 10.0;
		}
	}
	painter.end();
}

void MainWindow::actionPrintClicked()
{
	if (!defaultPrinter)
		defaultPrinter = new QPrinter(QPrinter::HighResolution);
	QPrintDialog printDialog(defaultPrinter, this);
	if (printDialog.exec() == QDialog::Accepted)
		print(defaultPrinter);
}

void MainWindow::actionPrintPreviewClicked()
{
	if (!defaultPrinter)
		defaultPrinter = new QPrinter(QPrinter::HighResolution);
	printPreview(defaultPrinter);
}

void MainWindow::printPreview(QPrinter *printer)
{
	QPrintPreviewDialog *previewDialog = new QPrintPreviewDialog(printer, this);
	connect( previewDialog, SIGNAL(paintRequested(QPrinter *) ), this, SLOT(print(QPrinter *)) );
	previewDialog->exec();
}

QString MainWindow::convertToHtml(SaagharWidget *saagharObject)
{
	GanjoorPoem curPoem = SaagharWidget::ganjoorDataBase->getPoem(saagharObject->currentPoem);
	if (curPoem.isNull()) return "";
	QString columnGroupFormat = QString("<COL WIDTH=%1>").arg(saagharObject->tableViewWidget->columnWidth(0));
	int numberOfCols = saagharObject->tableViewWidget->columnCount()-1;
	QString tableBody = "";
	if ( numberOfCols == 3)
	{
		columnGroupFormat = QString("<COL WIDTH=%1><COL WIDTH=%2><COL WIDTH=%3>").arg(saagharObject->tableViewWidget->columnWidth(1)).arg(saagharObject->tableViewWidget->columnWidth(2)).arg(saagharObject->tableViewWidget->columnWidth(3));
		for (int row = 0; row < saagharObject->tableViewWidget->rowCount(); ++row)
		{
			int span = saagharObject->tableViewWidget->columnSpan(row, 1);
			if (span == 1)
			{
				QTableWidgetItem *firstItem = saagharObject->tableViewWidget->item(row, 1);
				QTableWidgetItem *secondItem = saagharObject->tableViewWidget->item(row, 3);
				QString firstText = "";
				QString secondText = "";
				if (firstItem)
					firstText = firstItem->text();
				if (secondItem)
					secondText = secondItem->text();
				tableBody += QString("\n<TR>\n<TD HEIGHT=%1 ALIGN=LEFT>%2</TD>\n<TD></TD>\n<TD ALIGN=RIGHT>%3</TD>\n</TR>\n").arg(saagharObject->tableViewWidget->rowHeight(row)).arg(firstText).arg(secondText);
			}
			else
			{
				QTableWidgetItem *item = saagharObject->tableViewWidget->item(row, 1);
				QString mesraText = "";

				QString align = "RIGHT";
				if (SaagharWidget::centeredView)
					align = "CENTER";

				QStringList itemDataList;
				if (item)
				{
					mesraText = item->text();
					QVariant data = item->data(Qt::UserRole);
					if (data.isValid() && !data.isNull())
						itemDataList = data.toString().split("|", QString::SkipEmptyParts);
				}

				VersePosition itemPosition = CenteredVerse1;
				if (itemDataList.size() == 4)
				{
					itemPosition = (VersePosition)itemDataList.at(3).toInt();
				}

				switch (itemPosition)
				{
					case Paragraph :
					case Single :
							mesraText.replace(" ", "&nbsp;");
							align = "RIGHT";
						break;

					default:
						break;
				}
				tableBody += QString("\n<TR>\n<TD COLSPAN=3 HEIGHT=%1 WIDTH=%2 ALIGN=%3>%4</TD>\n</TR>\n").arg(saagharObject->tableViewWidget->rowHeight(row)).arg( saagharObject->tableViewWidget->width() ).arg(align).arg(mesraText);
			}
		}
	}
	else
	{
		return "";
	}

	QString tableAsHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n<HTML>\n<HEAD>\n<META HTTP-EQUIV=\"CONTENT-TYPE\" CONTENT=\"text/html; charset=utf-8\">\n<TITLE>%1</TITLE>\n<META NAME=\"GENERATOR\" CONTENT=\"Saaghar, a Persian poetry software, http://pojh.iBlogger.org/saaghar\">\n</HEAD>\n\n<BODY TEXT=%2>\n<FONT FACE=\"%3\">\n<TABLE ALIGN=%4 DIR=RTL FRAME=VOID CELLSPACING=0 COLS=%5 RULES=NONE BORDER=0>\n<COLGROUP>%6</COLGROUP>\n<TBODY>\n%7\n</TBODY>\n</TABLE>\n</BODY>\n</HTML>\n")
		.arg(curPoem._Title).arg(SaagharWidget::textColor.name()).arg(SaagharWidget::tableFont.family()).arg("CENTER").arg(numberOfCols).arg(columnGroupFormat).arg(tableBody);
	return tableAsHTML;
	/*******************************************************
	%1: title
	%2: Text Color, e.g: "#CC55AA"
	%3: Font Face, e.g: "XB Zar"
	%4: Table Align
	%5: number of columns
	%6: column group format, e.g: <COL WIDTH=259><COL WIDTH=10><COL WIDTH=293>
	%7: table body, e.g: <TR><TD HEIGHT=17 ALIGN=LEFT>Column1</TD><TD></TD><TD ALIGN=LEFT>Column3</TD></TR>
	*********************************************************/
}

QString MainWindow::convertToTeX(SaagharWidget *saagharObject)
{
	GanjoorPoem curPoem = SaagharWidget::ganjoorDataBase->getPoem(saagharObject->currentPoem);
	if (curPoem.isNull()) return "";
	int numberOfCols = saagharObject->tableViewWidget->columnCount()-1;
	QString poemType = "traditionalpoem";
	QString tableBody = "";
	bool poemEnvironmentEnded = false;
	if ( numberOfCols == 3)
	{
		for (int row = 1; row < saagharObject->tableViewWidget->rowCount(); ++row)
		{
			int span = saagharObject->tableViewWidget->columnSpan(row, 1);
			if (span == 1)
			{
				QTableWidgetItem *firstItem = saagharObject->tableViewWidget->item(row, 1);
				QTableWidgetItem *secondItem = saagharObject->tableViewWidget->item(row, 3);
				QString firstText = "";
				QString secondText = "";
				if (firstItem)
					firstText = firstItem->text();
				if (secondItem)
					secondText = secondItem->text();
				
				tableBody += QString("%1 & %2 ").arg(firstText).arg(secondText);
				if (row+1 != saagharObject->tableViewWidget->rowCount())
					tableBody+="\\\\\n";
			}
			else
			{
				QString align = "r";
				if (row == 0 && SaagharWidget::centeredView)
					align = "c";
				QTableWidgetItem *item = saagharObject->tableViewWidget->item(row, 1);
				QString mesraText = "";
				QStringList itemDataList;
				if (item)
				{
					mesraText = item->text();
					QVariant data = item->data(Qt::UserRole);
					if (data.isValid() && !data.isNull())
						itemDataList = data.toString().split("|", QString::SkipEmptyParts);
				}

				VersePosition itemPosition = CenteredVerse1;
				if (itemDataList.size() == 4)
				{
					itemPosition = (VersePosition)itemDataList.at(3).toInt();
				}
				switch (itemPosition)
				{
					case Paragraph :
							tableBody += QString("%Empty Line: a tricky method, last beyt has TeX newline \"\\\\\"\n");//tricky, last beyt has TeX newline "\\"
							tableBody += QString("\n\\end{%1}\n").arg(poemType);
							tableBody += QString("%1").arg(mesraText);
							poemEnvironmentEnded = true;
							if (row+1 != saagharObject->tableViewWidget->rowCount())
							{
								tableBody+="\\\\\n";
								tableBody += QString("\n\\begin{%1}\n").arg(poemType);
								poemEnvironmentEnded = false;
							}
						break;
		
					case Single :
							mesraText.replace(" ","\\ ");
							poemType = "modernpoem";

					default:
							tableBody += QString("%1").arg(mesraText);
							if (row+1 != saagharObject->tableViewWidget->rowCount())
								tableBody+="\\\\\n";
						break;
				}
			}
		}
	}
	else
	{
		return "";
	}
	
	QString endOfEnvironment = "";
	if (!poemEnvironmentEnded)
		endOfEnvironment = QString("\\end{%1}\n").arg(poemType);

	QString	tableAsTeX = QString("%%%%%\n%This file is generated automatically by Saaghar %1, 2010 http://pojh.iBlogger.org\n%%%%%\n%XePersian and bidipoem packages must have been installed on your TeX distribution for compiling this document\n%You can compile this document by running XeLaTeX on it, twice.\n%%%%%\n\\documentclass{article}\n\\usepackage{hyperref}%\n\\usepackage[Kashida]{xepersian}\n\\usepackage{bidipoem}\n\\settextfont{%2}\n\\hypersetup{\npdftitle={%3},%\npdfsubject={Poem},%\npdfkeywords={Poem, Persian},%\npdfcreator={Saaghar, a Persian poetry software, http://pojh.iBlogger.org/saaghar},%\npdfview=FitV,\n}\n\\renewcommand{\\poemcolsepskip}{1.5cm}\n\\begin{document}\n\\begin{center}\n%3\\\\\n\\end{center}\n\\begin{%4}\n%5\n%6\\end{document}\n%End of document\n")
								 .arg(VER_FILEVERSION_STR).arg(SaagharWidget::tableFont.family()).arg(curPoem._Title).arg(poemType).arg(tableBody).arg(endOfEnvironment);
	return tableAsTeX;
	/*******************************************************
	%1: Saaghar Version: VER_FILEVERSION_STR
	%2: Font Family
	%3: Poem Title
	%4: Poem Type, traditionalpoem or modernpoem environment
	%5: Poem body, e.g:
	مصراع۱سطر۱‏ ‎&%
	مصراع۲سطر۱‏ \\ 
	بند ‎\\ 
	%6: End of environment if it's not ended
	*********************************************************/
}

void MainWindow::actionNewWindowClicked()
{
	QProcess::startDetached("\""+QCoreApplication::applicationFilePath()+"\"",QStringList() << "-fresh");
}

void MainWindow::actionNewTabClicked()
{
	insertNewTab();
}

void MainWindow::actFullScreenClicked(bool checked)
{
	setUpdatesEnabled(false);
	if (checked)
	{
		setWindowState(windowState() | Qt::WindowFullScreen);
		actionInstance("actionFullScreen")->setText(tr("Exit &Full Screen"));
		actionInstance("actionFullScreen")->setIcon(QIcon(currentIconThemePath()+"/no-fullscreen.png"));
	}
	else
	{
		setWindowState(windowState() & ~Qt::WindowFullScreen);
		actionInstance("actionFullScreen")->setText(tr("&Full Screen"));
		actionInstance("actionFullScreen")->setIcon(QIcon(currentIconThemePath()+"/fullscreen.png"));
	}
	setUpdatesEnabled(true);
}

void MainWindow::actionFaalRandomClicked()
{
	QAction *triggeredAction = qobject_cast<QAction *>(sender());
	int actionData = -1;//Faal
	if (triggeredAction && triggeredAction->data().isValid())
		actionData = triggeredAction->data().toInt();
	if (actionData == -1)
		actionData = 24;
	else
	{
		if (selectedRandomRange.contains("CURRENT_TAB_SUBSECTIONS"))
		{
			if (saagharWidget->currentPoem != 0)
				actionData = SaagharWidget::ganjoorDataBase->getPoem(saagharWidget->currentPoem)._CatID;
			else
				actionData = saagharWidget->currentCat;
		}
		else if (selectedRandomRange.contains("0")) //all
			actionData = 0;//home
		else
		{
			if (selectedRandomRange.isEmpty())
				actionData = 0;//all
			else
			{
				int randIndex = QGanjoorDbBrowser::getRandomNumber(0, selectedRandomRange.size()-1);
				actionData = SaagharWidget::ganjoorDataBase->getPoet(selectedRandomRange.at(randIndex).toInt())._CatID;
				qDebug() << "actionData=" << actionData << "randIndex=" << randIndex << SaagharWidget::ganjoorDataBase->getPoet(selectedRandomRange.at(randIndex))._Name;
			}
		}
	}

	int PoemID = SaagharWidget::ganjoorDataBase->getRandomPoemID(&actionData);
	GanjoorPoem poem = SaagharWidget::ganjoorDataBase->getPoem(PoemID);
	if (!poem.isNull() && poem._CatID == actionData)
		if (randomOpenInNewTab)
			newTabForItem("PoemID", poem._ID, true);
		else
			saagharWidget->processClickedItem("PoemID", poem._ID, true);
	else
		actionFaalRandomClicked();//not any random id exists, so repeat until finding a valid id
}

void MainWindow::aboutSaaghar()
{
	QMessageBox about(this);
	QPixmap pixmap(":/resources/images/saaghar.png");
	about.setIconPixmap(pixmap);
	about.setWindowTitle(tr("About Saaghar"));
	about.setTextFormat(Qt::RichText);
	about.setText(tr("<br />%1 is a persian poem viewer software, it uses \"ganjoor.net\" database, and some of its codes are ported to C++ and Qt from \"desktop ganjoor\" that is a C# .NET application written by %2.<br /><br />Logo Designer: %3<br /><br />Author: %4,<br /><br />Home Page: %5<br />Mailing List: %6<br />Saaghar in FaceBook:%7<br /><br />Version: %8<br />Build Time: %9")
		.arg("<a href=\"http://pojh.iBlogger.org/saaghar\">"+tr("Saaghar")+"</a>").arg("<a href=\"http://www.gozir.com/\">"+tr("Hamid Reza Mohammadi")+"</a>").arg("<a href=\"http://www.phototak.com/\">"+tr("S. Nasser Alavizadeh")+"</a>").arg("<a href=\"http://pojh.iBlogger.org/\">"+tr("S. Razi Alavizadeh")+"</a>").arg("<a href=\"http://pojh.iBlogger.org/saaghar\">http://pojh.iBlogger.org/saaghar</a>").arg("<a href=\"http://groups.google.com/group/saaghar/\">http://groups.google.com/group/saaghar</a>").arg("<a href=\"http://www.facebook.com/saaghar.p\">http://www.facebook.com/saaghar.p</a>").arg(VER_FILEVERSION_STR).arg(VER_FILEBUILDTIME_STR));
	about.setStandardButtons(QMessageBox::Ok);
	about.setEscapeButton(QMessageBox::Ok);

	about.exec();
}

void MainWindow::helpContents()
{
	if ( !QDesktopServices::openUrl("file:///"+resourcesPath+"/Saaghar-Manual.pdf") )
		QMessageBox::warning( this,tr("Error"),tr("Help file not found!"));
}

void MainWindow::closeCurrentTab()
{
	if (mainTabWidget->count() <= 1)
		close();

	tabCloser(mainTabWidget->currentIndex());
	return;
}

void MainWindow::actionImportNewSet()
{
	QMessageBox warnAboutBackup(this);
	warnAboutBackup.setWindowTitle(tr("Please Notice!"));
	warnAboutBackup.setIcon(QMessageBox::Information);
	warnAboutBackup.setText(tr("This feature is in beta state, before continue you should create a backup from your database."));
	warnAboutBackup.addButton(tr("Continue"), QMessageBox::AcceptRole);
	warnAboutBackup.setStandardButtons(QMessageBox::Cancel);
	warnAboutBackup.setEscapeButton(QMessageBox::Cancel);
	warnAboutBackup.setDefaultButton(QMessageBox::Cancel);
	int ret = warnAboutBackup.exec();
	if ( ret == QMessageBox::Cancel)
		return;

	QString dataBaseName = QFileDialog::getOpenFileName(this,tr("Browse for a new set"), QDir::homePath(),"Ganjoor DataBase (*.gdb *.s3db);;All files (*.*)");
	if (!dataBaseName.isEmpty())
		importDataBase(dataBaseName);
}

void MainWindow::setupUi()
{
	QString iconThemePath = currentIconThemePath();

	if (Settings::READ("MainWindowState").isNull())
	{
		insertToolBarBreak(ui->mainToolBar);
	}

	skipSearchToolBarResize = false;
	lineEditSearchText = 0;
	comboBoxSearchRegion = 0;
	setupSearchToolBarUi();
	ui->searchToolBar->hide();

	//remove menuToolBar if it's not needed!
	bool flagUnityGlobalMenu = false;
	foreach(QObject *o, ui->menuBar->children())
	{
		if (o->inherits("QDBusServiceWatcher"))
		{
			flagUnityGlobalMenu = true;
			break;
		}
	}

	if (flagUnityGlobalMenu)
	{
		qDebug() << "It seems that you are running Unity Desktop with global menubar!";
		delete ui->menuToolBar;
		ui->menuToolBar = 0;
	}

	#ifdef Q_WS_MAC
		delete ui->menuToolBar;
		ui->menuToolBar = 0;
	#endif

	if (ui->menuToolBar)
	{
		QHBoxLayout *menuBarLayout = new QHBoxLayout();
		menuBarLayout->addWidget(ui->menuBar,0,Qt::AlignCenter|Qt::AlignLeft);
		menuBarLayout->setContentsMargins(1,1,1,1);
		QWidget *menuBarContainer = new QWidget(this);
		menuBarContainer->setLayout(menuBarLayout);
		ui->menuToolBar->addWidget(menuBarContainer);
	}

	//Initialize main menu items
	menuFile = new QMenu(tr("&File"), ui->menuBar);
	menuFile->setObjectName(QString::fromUtf8("menuFile"));

	//submenus
	menuOpenedTabs = new QMenu(tr("&Opened Tabs"), menuFile);
	menuOpenedTabs->setObjectName(QString::fromUtf8("menuOpenedTabs"));
	menuClosedTabs = new QMenu(tr("&Closed Tabs"), menuFile);
	menuClosedTabs->setObjectName(QString::fromUtf8("menuClosedTabs"));

	menuNavigation = new QMenu(tr("&Navigation"), ui->menuBar);
	menuNavigation->setObjectName(QString::fromUtf8("menuNavigation"));
	menuView = new QMenu(tr("&View"), ui->menuBar);
	menuView->setObjectName(QString::fromUtf8("menuView"));
	menuBookmarks = new QMenu(tr("&Bookmarks"), ui->menuBar);
	menuBookmarks->setObjectName(QString::fromUtf8("menuBookmarks"));
	menuTools = new QMenu(tr("&Tools"), ui->menuBar);
	menuTools->setObjectName(QString::fromUtf8("menuTools"));
	menuHelp = new QMenu(tr("&Help"), ui->menuBar);
	menuHelp->setObjectName(QString::fromUtf8("menuHelp"));

	ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	//initialize BookmarkManager Ui
	setupBookmarkManagerUi();

	//Initialize Actions
	actionInstance("actionHome", iconThemePath+"/home.png", tr("&Home") );

	if (ui->mainToolBar->layoutDirection() == Qt::LeftToRight)
	{
		actionInstance("actionPreviousPoem", iconThemePath+"/previous.png", tr("&Previous") )->setShortcuts(QKeySequence::Back);
		actionInstance("actionNextPoem", iconThemePath+"/next.png", tr("&Next") )->setShortcuts(QKeySequence::Forward);
	}
	else
	{
		actionInstance("actionPreviousPoem", iconThemePath+"/next.png", tr("&Previous") )->setShortcuts(QKeySequence::Forward);
		actionInstance("actionNextPoem", iconThemePath+"/previous.png", tr("&Next") )->setShortcuts(QKeySequence::Back);
	}

	actionInstance("actionCopy", iconThemePath+"/copy.png", tr("&Copy") )->setShortcuts(QKeySequence::Copy);

	allActionMap.insert("searchToolbarAction", ui->searchToolBar->toggleViewAction());
	actionInstance("searchToolbarAction")->setIcon(QIcon(iconThemePath+"/search.png"));
	actionInstance("searchToolbarAction")->setObjectName(QString::fromUtf8("searchToolbarAction"));
	actionInstance("searchToolbarAction")->setShortcuts(QKeySequence::Find);

	actionInstance("actionSettings", iconThemePath+"/settings.png", tr("S&ettings") )->setMenuRole(QAction::PreferencesRole);//needed for Mac OS X

	actionInstance("actionViewInGanjoorSite", iconThemePath+"/browse_net.png", tr("View in \"&ganjoor.net\"") );

	actionInstance("actionExit", iconThemePath+"/exit.png", tr("E&xit") )->setMenuRole(QAction::QuitRole);//needed for Mac OS X
	actionInstance("actionExit")->setShortcut(Qt::CTRL | Qt::Key_Q);

	actionInstance("actionNewTab", iconThemePath+"/new_tab.png", tr("New &Tab") )->setShortcuts(QKeySequence::AddTab);

	actionInstance("actionNewWindow", iconThemePath+"/new_window.png", tr("&New Window") )->setShortcuts(QKeySequence::New);

	actionInstance("actionAboutSaaghar", ":/resources/images/saaghar.png", tr("&About") )->setMenuRole(QAction::AboutRole);//needed for Mac OS X

	actionInstance("actionAboutQt", iconThemePath+"/qt-logo.png", tr("About &Qt") )->setMenuRole(QAction::AboutQtRole);//needed for Mac OS X

	actionInstance("actionFaal", iconThemePath+"/faal.png", tr("&Faal"))->setData("-1");

	actionInstance("actionPrint", iconThemePath+"/print.png", tr("&Print...") )->setShortcuts(QKeySequence::Print);

	actionInstance("actionPrintPreview", iconThemePath+"/print-preview.png", tr("Print Pre&view...") );

	actionInstance("actionExport", iconThemePath+"/export.png", tr("&Export As...") )->setShortcuts(QKeySequence::SaveAs);

	actionInstance("actionExportAsPDF", iconThemePath+"/export-pdf.png", tr("Exp&ort As PDF...") );

	actionInstance("actionHelpContents", ":/resources/images/saaghar.png", tr("&Help Contents...") )->setShortcuts(QKeySequence::HelpContents);

	actionInstance("actionCloseTab", iconThemePath+"/close-tab.png", tr("&Close Tab") )->setShortcuts(QKeySequence::Close);

	actionInstance("actionRandom", iconThemePath+"/random.png", tr("&Random"))->setShortcut(Qt::CTRL | Qt::Key_R);
	actionInstance("actionRandom")->setData("1");

	actionInstance("actionImportNewSet", iconThemePath+"/import-to-database.png", tr("Insert New &Set..."));

	actionInstance("actionRemovePoet", iconThemePath+"/remove-poet.png", tr("&Remove Poet..."));

	actionInstance("actionFullScreen", iconThemePath+"/fullscreen.png", tr("&Full Screen") )->setShortcut(Qt::Key_F11);
	actionInstance("actionFullScreen")->setCheckable(true);

	actionInstance("actionCheckUpdates", iconThemePath+"/check-updates.png", tr("Check for &Updates") );

	//The following actions are processed in 'namedActionTriggered()' slot
	actionInstance("Show Photo at Home", iconThemePath+"/show-photo-home.png", tr("&Show Photo at Home"))->setCheckable(true);
	actionInstance("Show Photo at Home")->setChecked( Settings::READ("Show Photo at Home", true).toBool() );

	actionInstance("Lock ToolBars", iconThemePath+"/lock-toolbars.png", tr("&Lock ToolBars"))->setCheckable(true);
	actionInstance("Lock ToolBars")->setChecked( Settings::READ("Lock ToolBars", true).toBool() );
	ui->mainToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());
	if (ui->menuToolBar)
		ui->menuToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());
	ui->searchToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());
	parentCatsToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());

	actionInstance("Ganjoor Verification", iconThemePath+"/ocr-verification.png", tr("&OCR Verification") );

	//undo/redo actions
	globalRedoAction = undoGroup->createRedoAction(this, tr("&Redo"));
	globalRedoAction->setObjectName("globalRedoAction");
	globalUndoAction = undoGroup->createUndoAction(this, tr("&Undo"));
	globalUndoAction->setObjectName("globalUndoAction");
	if (ui->mainToolBar->layoutDirection() == Qt::LeftToRight)
	{
		globalRedoAction->setIcon(QIcon(currentIconThemePath()+"/redo.png"));
		globalUndoAction->setIcon(QIcon(currentIconThemePath()+"/undo.png"));
	}
	else
	{
		globalRedoAction->setIcon(QIcon(currentIconThemePath()+"/undo.png"));
		globalUndoAction->setIcon(QIcon(currentIconThemePath()+"/redo.png"));
	}

	globalRedoAction->setShortcuts(QKeySequence::Redo);
	globalUndoAction->setShortcuts(QKeySequence::Undo);

	actionInstance("fixedNameRedoAction", "", tr("&Redo"))->setIcon(globalRedoAction->icon());
	actionInstance("fixedNameUndoAction", "", tr("&Undo"))->setIcon(globalUndoAction->icon());
	actionInstance("fixedNameRedoAction")->setEnabled(globalRedoAction->isEnabled());
	actionInstance("fixedNameUndoAction")->setEnabled(globalUndoAction->isEnabled());

	actionInstance("actionCopy", iconThemePath+"/copy.png", tr("&Copy") )->setShortcuts(QKeySequence::Copy);



	//Inserting main menu items
	ui->menuBar->addMenu(menuFile);
	ui->menuBar->addMenu(menuNavigation);
	ui->menuBar->addMenu(menuView);
	ui->menuBar->addMenu(menuBookmarks);
	ui->menuBar->addMenu(menuTools);
	ui->menuBar->addMenu(menuHelp);

	//QMenuBar style sheet
	ui->menuBar->setStyleSheet("QMenuBar {background-color: transparent;}"
	"QMenuBar::item {spacing: 3px;padding: 1px 7px 4px 7px;background: transparent;border-radius: 4px;}"
	"QMenuBar::item:selected { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadada, stop: 0.4 #d4d4d4, stop: 0.5 #c7c7c7, stop: 1.0 #dadada); border: 1px; }"
	"QMenuBar::item:pressed {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #9a9a9a, stop: 0.4 #949494, stop: 0.5 #a7a7a7, stop: 1.0 #9a9a9a); color: #fefefe; border: 1px; }");

	int menuBarWidth = 0;
	foreach(QAction *act, ui->menuBar->actions())
	{
		if (act)
		{
			menuBarWidth+=ui->menuBar->fontMetrics().boundingRect(act->text()).width()+7;
		}
	}

	menuBarWidth =menuBarWidth-7-3;//last spacing!
	ui->menuBar->setMaximumWidth(menuBarWidth);


	//Inserting items of menus
	menuFile->addAction(actionInstance("actionNewTab"));
	menuFile->addAction(actionInstance("actionNewWindow"));
	menuFile->addAction(actionInstance("actionCloseTab"));
	menuFile->addSeparator();
	//submenus
	menuFile->addMenu(menuOpenedTabs);
	menuFile->addMenu(menuClosedTabs);
	menuFile->addSeparator();
	menuFile->addAction(actionInstance("actionExportAsPDF"));
	menuFile->addAction(actionInstance("actionExport"));
	menuFile->addSeparator();
	menuFile->addAction(actionInstance("actionPrintPreview"));
	menuFile->addAction(actionInstance("actionPrint"));
	menuFile->addSeparator();
	menuFile->addAction(actionInstance("actionExit"));

	menuNavigation->addAction(globalUndoAction);
	menuNavigation->addAction(globalRedoAction);
	menuNavigation->addSeparator();
	menuNavigation->addAction(actionInstance("actionHome"));
	menuNavigation->addSeparator();
	menuNavigation->addAction(actionInstance("actionPreviousPoem"));
	menuNavigation->addAction(actionInstance("actionNextPoem"));
	menuNavigation->addSeparator();
	menuNavigation->addAction(actionInstance("actionFaal"));
	menuNavigation->addAction(actionInstance("actionRandom"));

	menuView->addAction(actionInstance("Show Photo at Home"));

	menuView->addSeparator();
	QMenu *toolbarsView = new QMenu(tr("ToolBars"), menuView);
	if (ui->menuToolBar)
		toolbarsView->addAction(ui->menuToolBar->toggleViewAction());
	toolbarsView->addAction(ui->mainToolBar->toggleViewAction());
	toolbarsView->addAction(ui->searchToolBar->toggleViewAction());
	toolbarsView->addAction(parentCatsToolBar->toggleViewAction());
	toolbarsView->addSeparator();
	toolbarsView->addAction(actionInstance("Lock ToolBars"));
	menuView->addMenu(toolbarsView);

	toolBarViewActions(ui->mainToolBar, menuView, true);
	menuView->addSeparator();
	//checked actions, must be after above line
	toolbarViewChanges(actionInstance(Settings::READ("MainToolBar Style").toString()));
	toolbarViewChanges(actionInstance(Settings::READ("MainToolBar Size").toString()));

	menuView->addAction(actionInstance("actionFullScreen"));

	menuBookmarks->addAction(actionInstance("bookmarkManagerDockAction"));
	menuBookmarks->addSeparator();

	menuTools->addAction(actionInstance("searchToolbarAction"));
	menuTools->addSeparator();
	menuTools->addAction(actionInstance("actionViewInGanjoorSite"));
	menuTools->addAction(actionInstance("Ganjoor Verification"));
	menuTools->addSeparator();
	menuTools->addAction(actionInstance("actionImportNewSet"));
	menuTools->addAction(actionInstance("actionRemovePoet"));
	menuTools->addSeparator();
	menuTools->addAction(actionInstance("actionSettings"));

	menuHelp->addAction(actionInstance("actionHelpContents"));
	menuHelp->addSeparator();
	menuHelp->addAction(actionInstance("actionCheckUpdates"));
	menuHelp->addSeparator();
	menuHelp->addAction(actionInstance("actionAboutSaaghar"));
	menuHelp->addAction(actionInstance("actionAboutQt"));

	//Inserting mainToolbar's items
	for (int i=0; i<mainToolBarItems.size(); ++i)
	{
		if (allActionMap.contains(mainToolBarItems.at(i)) || mainToolBarItems.at(i).contains("separator", Qt::CaseInsensitive))
			ui->mainToolBar->addAction(actionInstance(mainToolBarItems.at(i)));
		else mainToolBarItems[i] = "";
	}

	//removing no more supported items
	QString temp = mainToolBarItems.join("|");
	mainToolBarItems = temp.split("|", QString::SkipEmptyParts);
}

void MainWindow::showSearchOptionMenu()
{
	if (!searchOptionMenu || !lineEditSearchText || !lineEditSearchText->optionsButton()) return;
	searchOptionMenu->exec(lineEditSearchText->optionsButton()->mapToGlobal(QPoint(0, lineEditSearchText->optionsButton()->height()))/*QCursor::pos()*/);
}

//void MainWindow::newSearchNonAlphabetChanged(bool checked)
//{
//	SaagharWidget::newSearchSkipNonAlphabet = checked;
//}

QAction *MainWindow::actionInstance(const QString actionObjectName, QString iconPath, QString displayName)
{
	QAction *action;

	if (actionObjectName.isEmpty() || actionObjectName.contains("separator", Qt::CaseInsensitive) )
	{
		action = new QAction(this);
		action->setSeparator(true);
		return action;
	}
	action = allActionMap.value(actionObjectName);
	if (!action)
	{
		if (displayName.isEmpty())
		{
			displayName = actionObjectName;
			displayName.remove("action");
			displayName.replace("__", "&");
			displayName.replace("_", " ");
			displayName = tr(displayName.toUtf8().data());
		}

		/*if (iconPath.isEmpty())
		{
			QString iconThemePath=":/resources/images/";
			if (!settingsIconThemePath.isEmpty() && settingsIconThemeState)
				iconThemePath = settingsIconThemePath;
			iconPath = iconThemePath+"/"+actionObjectName+".png";
		}*/
		
		action = new QAction(displayName, this);
		if ( QFile::exists(iconPath) )
			action->setIcon(QIcon(iconPath));

		action->setObjectName(actionObjectName);
		allActionMap.insert(actionObjectName, action);
	}
	connect(action, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));
	return action;
}

void MainWindow::createConnections()
{
		//File
	connect(actionInstance("actionNewTab")				,	SIGNAL(triggered())		,	this, SLOT(actionNewTabClicked())		);
	connect(actionInstance("actionNewWindow")			,	SIGNAL(triggered())		,	this, SLOT(actionNewWindowClicked())	);
	connect(actionInstance("actionCloseTab")			,	SIGNAL(triggered())		,	this, SLOT(closeCurrentTab())			);
	connect(actionInstance("actionExportAsPDF")			,	SIGNAL(triggered())		,	this, SLOT(actionExportAsPDFClicked())	);
	connect(actionInstance("actionExport")				,	SIGNAL(triggered())		,	this, SLOT(actionExportClicked())		);
	connect(actionInstance("actionPrintPreview")		,	SIGNAL(triggered())		,	this, SLOT(actionPrintPreviewClicked())	);
	connect(actionInstance("actionPrint")				,	SIGNAL(triggered())		,	this, SLOT(actionPrintClicked())		);
	connect(actionInstance("actionExit")				,	SIGNAL(triggered())		,	this, SLOT(close())						);

		//Navigation
	connect(globalRedoAction							,	SIGNAL(changed())		,	this, SLOT(namedActionTriggered())		);
	connect(globalUndoAction							,	SIGNAL(changed())		,	this, SLOT(namedActionTriggered())		);
	connect(actionInstance("actionHome")				,	SIGNAL(triggered())		,	this, SLOT(actionHomeClicked())			);
	connect(actionInstance("actionPreviousPoem")		,	SIGNAL(triggered())		,	this, SLOT(actionPreviousPoemClicked())	);
	connect(actionInstance("actionNextPoem")			,	SIGNAL(triggered())		,	this, SLOT(actionNextPoemClicked())		);
	connect(actionInstance("actionFaal")				,	SIGNAL(triggered())		,	this, SLOT(actionFaalRandomClicked())	);
	connect(actionInstance("actionRandom")				,	SIGNAL(triggered())		,	this, SLOT(actionFaalRandomClicked())	);

		//View
	connect(actionInstance("actionFullScreen")			,	SIGNAL(toggled(bool))	,	this, SLOT(actFullScreenClicked(bool))	);

		//Tools
	connect(actionInstance("actionViewInGanjoorSite")	,	SIGNAL(triggered())		,	this, SLOT(actionGanjoorSiteClicked())	);
	connect(actionInstance("actionCopy")				,	SIGNAL(triggered())		,	this, SLOT(copySelectedItems())			);
	connect(actionInstance("actionSettings")			,	SIGNAL(triggered())		,	this, SLOT(globalSettings())			);
	connect(actionInstance("actionRemovePoet")			,	SIGNAL(triggered())		,	this, SLOT(actionRemovePoet())			);
	connect(actionInstance("actionImportNewSet")		,	SIGNAL(triggered())		,	this, SLOT(actionImportNewSet())		);

		//Help
	connect(actionInstance("actionHelpContents")		,	SIGNAL(triggered())		,	this, SLOT(helpContents())				);
	connect(actionInstance("actionCheckUpdates")		,	SIGNAL(triggered())		,	this, SLOT(checkForUpdates())			);
	connect(actionInstance("actionAboutSaaghar")		,	SIGNAL(triggered())		,	this, SLOT(aboutSaaghar())				);
	connect(actionInstance("actionAboutQt")				,	SIGNAL(triggered())		,	qApp, SLOT(aboutQt())					);

		//searchToolBar connections
	connect(lineEditSearchText							,	SIGNAL(returnPressed())	,	this, SLOT(searchStart())				);
	//connect(pushButtonSearch							,	SIGNAL(clicked())		,	this, SLOT(searchStart())				);
}

//Settings Dialog
void MainWindow::globalSettings()
{
	Settings *settingsDlg = new Settings(this);
	connect(settingsDlg->ui->pushButtonRandomOptions, SIGNAL(clicked()), this, SLOT(customizeRandomDialog()));

	/************initializing saved state of Settings dialog object!************/
	settingsDlg->ui->pushButtonActionBottom->setIcon(QIcon(currentIconThemePath()+"/down.png"));
	settingsDlg->ui->pushButtonActionTop->setIcon(QIcon(currentIconThemePath()+"/up.png"));

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		settingsDlg->ui->pushButtonActionAdd->setIcon(QIcon(currentIconThemePath()+"/left.png"));
		settingsDlg->ui->pushButtonActionRemove->setIcon(QIcon(currentIconThemePath()+"/right.png"));
	}
	else
	{
		settingsDlg->ui->pushButtonActionAdd->setIcon(QIcon(currentIconThemePath()+"/right.png"));
		settingsDlg->ui->pushButtonActionRemove->setIcon(QIcon(currentIconThemePath()+"/left.png"));
	}

	settingsDlg->ui->spinBoxPoetsPerGroup->setValue(SaagharWidget::maxPoetsPerGroup);

	settingsDlg->ui->checkBoxAutoUpdates->setChecked(MainWindow::autoCheckForUpdatesState);

	//database
	settingsDlg->ui->lineEditDataBasePath->setText(QGanjoorDbBrowser::dataBasePath.join(";"));
	connect( settingsDlg->ui->pushButtonDataBasePath, SIGNAL(clicked()), settingsDlg, SLOT(browseForDataBasePath()));

	//font
	/*
	settingsDlg->ui->comboBoxFontFamily->setWritingSystem(QFontDatabase::Arabic);
	*/
	settingsDlg->ui->comboBoxFontFamily->setCurrentFont(SaagharWidget::tableFont);
	settingsDlg->ui->spinBoxFontSize->setValue(SaagharWidget::tableFont.pointSize());
	
	settingsDlg->ui->checkBoxBeytNumbers->setChecked(SaagharWidget::showBeytNumbers);

	settingsDlg->ui->checkBoxBackground->setChecked(SaagharWidget::backgroundImageState);
	settingsDlg->ui->lineEditBackground->setText(SaagharWidget::backgroundImagePath);
	settingsDlg->ui->lineEditBackground->setEnabled(SaagharWidget::backgroundImageState);
	settingsDlg->ui->pushButtonBackground->setEnabled(SaagharWidget::backgroundImageState);
	connect( settingsDlg->ui->pushButtonBackground, SIGNAL(clicked()), settingsDlg, SLOT(browseForBackground()));
	
	settingsDlg->ui->checkBoxIconTheme->setChecked(settingsIconThemeState);
	settingsDlg->ui->lineEditIconTheme->setText(settingsIconThemePath);
	settingsDlg->ui->lineEditIconTheme->setEnabled(settingsIconThemeState);
	settingsDlg->ui->pushButtonIconTheme->setEnabled(settingsIconThemeState);
	connect( settingsDlg->ui->pushButtonIconTheme, SIGNAL(clicked()), settingsDlg, SLOT(browseForIconTheme()));

	//colors
	connect( settingsDlg->ui->pushButtonBackgroundColor, SIGNAL(clicked()), settingsDlg, SLOT(getColorForPushButton()));
	connect( settingsDlg->ui->pushButtonMatchedTextColor, SIGNAL(clicked()), settingsDlg, SLOT(getColorForPushButton()));
	connect( settingsDlg->ui->pushButtonTextColor, SIGNAL(clicked()), settingsDlg, SLOT(getColorForPushButton()));
	settingsDlg->ui->pushButtonBackgroundColor->setPalette(QPalette(SaagharWidget::backgroundColor));
	settingsDlg->ui->pushButtonBackgroundColor->setAutoFillBackground(true);
	settingsDlg->ui->pushButtonMatchedTextColor->setPalette(QPalette(SaagharWidget::matchedTextColor));
	settingsDlg->ui->pushButtonMatchedTextColor->setAutoFillBackground(true);
	settingsDlg->ui->pushButtonTextColor->setPalette(QPalette(SaagharWidget::textColor));
	settingsDlg->ui->pushButtonTextColor->setAutoFillBackground(true);

	//initialize Action's Tables
	settingsDlg->initializeActionTables(allActionMap, mainToolBarItems);

	connect(settingsDlg->ui->pushButtonActionBottom, SIGNAL(clicked()), settingsDlg, SLOT(bottomAction()));
	connect(settingsDlg->ui->pushButtonActionTop, SIGNAL(clicked()), settingsDlg, SLOT(topAction()));
	connect(settingsDlg->ui->pushButtonActionAdd, SIGNAL(clicked()), settingsDlg, SLOT(addActionToToolbarTable()));
	connect(settingsDlg->ui->pushButtonActionRemove, SIGNAL(clicked()), settingsDlg, SLOT(removeActionFromToolbarTable()));
	/************end of initialization************/

	if (settingsDlg->exec())
	{
		/************setup new settings************/
		SaagharWidget::maxPoetsPerGroup = settingsDlg->ui->spinBoxPoetsPerGroup->value();
	
		MainWindow::autoCheckForUpdatesState = settingsDlg->ui->checkBoxAutoUpdates->isChecked();
	
		//database path
		QGanjoorDbBrowser::dataBasePath = settingsDlg->ui->lineEditDataBasePath->text().split(";", QString::SkipEmptyParts);
	
		//font
		//QFont font(settingsDlg->ui->comboBoxFontFamily->currentText(), settingsDlg->ui->spinBoxFontSize->value());
		SaagharWidget::tableFont = settingsDlg->ui->comboBoxFontFamily->currentFont(); //font;
		SaagharWidget::tableFont.setPointSize(settingsDlg->ui->spinBoxFontSize->value());
		
		SaagharWidget::showBeytNumbers = settingsDlg->ui->checkBoxBeytNumbers->isChecked();
	
		SaagharWidget::backgroundImageState = settingsDlg->ui->checkBoxBackground->isChecked();
		SaagharWidget::backgroundImagePath = settingsDlg->ui->lineEditBackground->text();
	
		settingsIconThemeState = settingsDlg->ui->checkBoxIconTheme->isChecked();
		settingsIconThemePath = settingsDlg->ui->lineEditIconTheme->text();
		
		//colors
		SaagharWidget::backgroundColor = settingsDlg->ui->pushButtonBackgroundColor->palette().background().color();
		SaagharWidget::textColor = settingsDlg->ui->pushButtonTextColor->palette().background().color();
		if (settingsDlg->ui->pushButtonMatchedTextColor->palette().background().color() != SaagharWidget::textColor)//they must be different.
			SaagharWidget::matchedTextColor = settingsDlg->ui->pushButtonMatchedTextColor->palette().background().color();
	
		//toolbar items
		mainToolBarItems.clear();
		for (int i=0; i<settingsDlg->ui->tableWidgetToolBarActions->rowCount(); ++i)
		{
			QTableWidgetItem *item = settingsDlg->ui->tableWidgetToolBarActions->item(i, 0);
			if (item)
			{
				mainToolBarItems.append(item->data(Qt::UserRole+1).toString());
			}
		}

		Settings::WRITE("Main ToolBar Items", mainToolBarItems.join("|"));

		/************end of setup new settings************/

		ui->mainToolBar->clear();
		//Inserting main toolbar Items
		for (int i=0; i<mainToolBarItems.size(); ++i)
		{
			ui->mainToolBar->addAction(actionInstance(mainToolBarItems.at(i)));
		}

		if (saagharWidget)
		{
			saagharWidget->loadSettings();

			if (saagharWidget->tableViewWidget->columnCount() == 1 && saagharWidget->tableViewWidget->rowCount() > 0 && saagharWidget->currentCat != 0)
			{
				QTableWidgetItem *item = saagharWidget->tableViewWidget->item(0,0);
				if (item)
				{
					QString text = item->text();
					int textWidth = saagharWidget->tableViewWidget->fontMetrics().boundingRect(text).width();
					int totalWidth = saagharWidget->tableViewWidget->columnWidth(0);
					saagharWidget->tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(saagharWidget->tableViewWidget->fontMetrics(), textWidth, totalWidth));
				}
			}
			else if ( saagharWidget->currentCat == 0 && saagharWidget->currentPoem == 0 ) //it's Home.
			{
				saagharWidget->homeResizeColsRows();
			//	saagharWidget->tableViewWidget->resizeRowsToContents();
			}
		}
#ifndef Q_WS_MAC //This doesn't work on MACX and moved to "SaagharWidget::loadSettings()"
		//apply new text and background color, and also background picture
		loadTabWidgetSettings();
#endif
		saveSettings();//save new settings to disk

		//////////////////////////////////////////////////////
		//set for refreshed all pages
		for (int i = 0; i < mainTabWidget->count(); ++i)
		{
			SaagharWidget *tmp = getSaagharWidget(i);
			tmp->setDirty();//needs to be refreshed
		}
		saagharWidget->refresh();
		//////////////////////////////////////////////////////
	}
}

QSettings *MainWindow::getSettingsObject()
{
	QString organization = "Pojh";
	QString settingsName = "Saaghar";
	QSettings::Format settingsFormat = QSettings::NativeFormat;

	if (isPortable)
	{
		settingsFormat = QSettings::IniFormat;
		QSettings::setPath(settingsFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
		organization = ".";
		settingsName = "settings";
	}
	return	new QSettings(settingsFormat, QSettings::UserScope, organization, settingsName);
}

void MainWindow::loadGlobalSettings()
{
	QSettings *config = getSettingsObject();

	Settings::LOAD_VARIABLES(config->value("VariableHash").toHash());

	restoreState( Settings::READ("MainWindowState").toByteArray(), 1);
	restoreGeometry(Settings::READ("Mainwindow Geometry").toByteArray());

	//openedTabs = config->value("openedTabs", "").toStringList();

	SaagharWidget::maxPoetsPerGroup = config->value("Max Poets Per Group", 12).toInt();
	
	//search options
	//SaagharWidget::newSearchFlag = config->value("New Search",true).toBool();
	//SaagharWidget::newSearchSkipNonAlphabet  = config->value("New Search non-Alphabet",false).toBool();
	SearchResultWidget::maxItemPerPage  = config->value("Max Search Results Per Page", 100).toInt();

	SaagharWidget::backgroundImageState = Settings::READ("Background State", true).toBool();
	SaagharWidget::backgroundImagePath = Settings::READ("Background Path", resourcesPath+"/themes/backgrounds/saaghar-pattern_1.png").toString();
	settingsIconThemeState = Settings::READ("Icon Theme State",false).toBool();
	settingsIconThemePath = Settings::READ("Icon Theme Path", resourcesPath+"/themes/iconsets/light-gray/").toString();

	QGanjoorDbBrowser::dataBasePath = config->value("DataBase Path", "").toString().split(";", QString::SkipEmptyParts);

	//The "XB Sols" is embeded in executable.
	QString fontFamily=config->value("Font Family", "XB Sols").toString();
	int fontSize=config->value( "Font Size", 18).toInt();
	QFont fnt(fontFamily,fontSize);
	SaagharWidget::tableFont = fnt;
	SaagharWidget::showBeytNumbers = config->value("Show Beyt Numbers",true).toBool();
	SaagharWidget::textColor = Settings::READ("Text Color",QColor(0x23,0x65, 0xFF)).value<QColor>();
	SaagharWidget::matchedTextColor = Settings::READ("Matched Text Color",QColor(0x5E, 0xFF, 0x13)).value<QColor>();
	SaagharWidget::backgroundColor = Settings::READ("Background Color",QColor(0xFE, 0xFD, 0xF2)).value<QColor>();

	autoCheckForUpdatesState = config->value("Auto Check For Updates",true).toBool();

	selectedRandomRange = config->value("Selected Random Range", "").toStringList();
	selectedSearchRange = config->value("Selected Search Range", "").toStringList();

	randomOpenInNewTab = config->value("Random Open New Tab", true).toBool();

//	mainToolBarItems = config->value("Main ToolBar Items", "actionHome|Separator|actionPreviousPoem|actionNextPoem|Separator|actionFaal|actionRandom|Separator|actionExportAsPDF|actionCopy|searchToolbarAction|actionNewTab|actionFullScreen|Separator|actionSettings|actionHelpContents").toString().split("|", QString::SkipEmptyParts);
//	QString tmp = mainToolBarItems.join("");
//	tmp.remove("action", Qt::CaseInsensitive);
//	tmp.remove("Separator", Qt::CaseInsensitive);
//	if (tmp.isEmpty() || tmp.contains(" "))
//		mainToolBarItems = QString("actionHome|Separator|actionPreviousPoem|actionNextPoem|Separator|actionFaal|actionRandom|Separator|actionExportAsPDF|actionCopy|searchToolbarAction|actionNewTab|actionFullScreen|Separator|actionSettings|actionHelpContents").split("|", QString::SkipEmptyParts);


	//initialize variableHash
//	S_R("Show Photo at Home", true);
//	S_R("Lock ToolBars", true);//default locked

	//config->setValue("VariableHash", QVariant(Settings::VariablesHash));
	QHash<QString, QVariant> vHash = config->value("VariableHash").toHash();

	mainToolBarItems = Settings::READ("Main ToolBar Items", "actionHome|Separator|actionPreviousPoem|actionNextPoem|Separator|actionFaal|actionRandom|Separator|actionCopy|bookmarkManagerDockAction|searchToolbarAction|actionNewTab|actionFullScreen|Separator|actionSettings|Ganjoor Verification").toString().split("|", QString::SkipEmptyParts);
//	QString tmp = mainToolBarItems.join("");
//	tmp.remove("action", Qt::CaseInsensitive);
//	tmp.remove("Separator", Qt::CaseInsensitive);
//	if (tmp.isEmpty() || tmp.contains(" "))
//		mainToolBarItems = QString("actionHome|Separator|actionPreviousPoem|actionNextPoem|Separator|actionFaal|actionRandom|Separator|actionCopy|searchToolbarAction|actionNewTab|actionFullScreen|Separator|actionSettings|actionHelpContents").split("|", QString::SkipEmptyParts);

///////////////
	//variable hash
	QHash<QString, QVariant>::const_iterator it = /*Settings::VariablesHash*/vHash.constBegin();
	while (it != vHash.constEnd())
	{
		qDebug() << "vHash--key=" << it.key() << "vHash--Value=" << it.value();
		++it;
	}
//////////////////////
}

void MainWindow::loadTabWidgetSettings()
{
	QPalette p(mainTabWidget->palette());
	if ( SaagharWidget::backgroundImageState && !SaagharWidget::backgroundImagePath.isEmpty() )
	{
		p.setBrush(QPalette::Base, QBrush(QPixmap(SaagharWidget::backgroundImagePath)) );
	}
	else
	{
		p.setColor(QPalette::Base, SaagharWidget::backgroundColor );
	}
	p.setColor(QPalette::Text, SaagharWidget::textColor );
	mainTabWidget->setPalette(p);
}

void MainWindow::saveSettings()
{
	QFile bookmarkFile(resourcesPath+"/bookmarks.xbel");
	if (!bookmarkFile.open(QFile::WriteOnly | QFile::Text))
	{
		QMessageBox::warning(this, tr("Bookmarks"), tr("Can not write the bookmark file %1:\n%2.")
							 .arg(bookmarkFile.fileName())
							 .arg(bookmarkFile.errorString()));
	}
	else
	{
		if (SaagharWidget::bookmarks)
			SaagharWidget::bookmarks->write(&bookmarkFile);
	}

	QSettings *config = getSettingsObject();

	//config->setValue("Main ToolBar Items", mainToolBarItems.join("|"));

	config->setValue("Max Poets Per Group", SaagharWidget::maxPoetsPerGroup);

	Settings::WRITE("Background State", SaagharWidget::backgroundImageState);
	Settings::WRITE("Background Path", SaagharWidget::backgroundImagePath);
	Settings::WRITE("Icon Theme State", settingsIconThemeState);
	Settings::WRITE("Icon Theme Path", settingsIconThemePath);

	//font
	config->setValue("Font Family", SaagharWidget::tableFont.family());
	config->setValue( "Font Size", SaagharWidget::tableFont.pointSize());

	config->setValue("Show Beyt Numbers", SaagharWidget::showBeytNumbers);

	//colors
	Settings::WRITE("Text Color", SaagharWidget::textColor);
	Settings::WRITE("Matched Text Color", SaagharWidget::matchedTextColor);
	Settings::WRITE("Background Color", SaagharWidget::backgroundColor);

	config->setValue("Auto Check For Updates", autoCheckForUpdatesState);

	///////////////////////////////////////////////////
	/////////////////////save state////////////////////
	Settings::WRITE("MainWindowState", saveState(1));
	Settings::WRITE("Mainwindow Geometry", saveGeometry());

	QStringList openedTabs;
	//openedTabs.clear();
	for (int i = 0; i < mainTabWidget->count(); ++i)
	{
		SaagharWidget *tmp = getSaagharWidget(i);
		QString tabViewType;
		if (tmp->currentPoem > 0)
			tabViewType = "PoemID="+QString::number(tmp->currentPoem);
		else
			tabViewType = "CatID="+QString::number(tmp->currentCat);
		
		openedTabs << tabViewType;
	}
	Settings::WRITE("Opened tabs from last session", openedTabs);
	//config->setValue("openedTabs", openedTabs);

	//database path
	config->setValue("DataBase Path", QDir::toNativeSeparators( QGanjoorDbBrowser::dataBasePath.join(";") ) );

	//search options
	//config->setValue("New Search", SaagharWidget::newSearchFlag);
	//config->setValue("New Search non-Alphabet", SaagharWidget::newSearchSkipNonAlphabet);
	config->setValue("Max Search Results Per Page", SearchResultWidget::maxItemPerPage);

	//Search and Random Range
	config->setValue("Selected Random Range", selectedRandomRange);

	QList<QListWidgetItem *> selectedItems = selectSearchRange->getSelectedItemList();

	selectedSearchRange.clear();
	foreach(QListWidgetItem *item, selectedItems)
	{
		selectedSearchRange << item->data(Qt::UserRole).toString();
	}

	config->setValue("Selected Search Range", selectedSearchRange);

	config->setValue("Random Open New Tab", randomOpenInNewTab);

	/////////////////////save state////////////////////
	///////////////////////////////////////////////////

	//variable hash
	config->setValue("VariableHash", Settings::GET_VARIABLES_VARIANT());
//	QHash<QString, QVariant> vHash = Settings::GET_VARIABLES_VARIANT().toHash();
//	QHash<QString, QVariant>::const_iterator it = vHash.constBegin();
//	while (it != vHash.constEnd())
//	{
//		config->setValue(it.key(), it.value());
//		++it;
//	}
}

QString MainWindow::tableToString(QTableWidget *table, QString mesraSeparator, QString beytSeparator, int startRow, int startColumn, int endRow, int endColumn)
{
	QString tableAsString = "";
	for(int row = startRow; row < endRow ; ++row )
	{
		bool secondColumn = true;
		for(int col = startColumn; col < endColumn ; ++col )
		{
			if (col == 0) continue;
			QTableWidgetItem *currentItem = table->item(row , col);
			QString text = "";
			if (currentItem)
			{
				QVariant data = currentItem->data(Qt::DisplayRole);
				if ( data.isValid() )
					text = data.toString();
			}
			tableAsString.append(text);
			if (secondColumn)
			{
				secondColumn = false;
				tableAsString.append(mesraSeparator);
			}
		}
		tableAsString.append(beytSeparator);
	}
	return tableAsString;
}

void MainWindow::copySelectedItems()
{
	if (!saagharWidget || !saagharWidget->tableViewWidget) return;
	QString selectedText="";
	QList<QTableWidgetSelectionRange> selectedRanges = saagharWidget->tableViewWidget->selectedRanges();
	if (selectedRanges.isEmpty())
		return;
	foreach(QTableWidgetSelectionRange selectedRange, selectedRanges)
	{
		if ( selectedRange.rowCount() == 0 || selectedRange.columnCount() == 0 )
			return;
		selectedText += tableToString(saagharWidget->tableViewWidget, "          "/*ten blank spaces-Mesra separator*/, "\n"/*Beyt separator*/, selectedRange.topRow(), selectedRange.leftColumn(), selectedRange.bottomRow()+1, selectedRange.rightColumn()+1);
	}
	QApplication::clipboard()->setText(selectedText);
}

//Navigation
void MainWindow::actionHomeClicked()
{
	saagharWidget->showHome();
}

void MainWindow::actionPreviousPoemClicked()
{
	saagharWidget->previousPoem();
}

void MainWindow::actionNextPoemClicked()
{
	saagharWidget->nextPoem();
}

//Tools
void MainWindow::actionGanjoorSiteClicked()
{
	if ( saagharWidget->currentPageGanjoorUrl().isEmpty() )
	{
		QMessageBox::critical(this, tr("Error!"), tr("There is no equivalent page there at ganjoor website."), QMessageBox::Ok, QMessageBox::Ok );
	}
	else
	{
		QDesktopServices::openUrl(QString("http://saaghar.sourceforge.net/redirect.php?sender=Saaghar&section=ganjoornet&url=%1").arg(saagharWidget->currentPageGanjoorUrl()));
	}
}

//void MainWindow::emitReSizeEvent()
//{
//	maybe a Qt BUG
//	before 'QMainWindow::show()' the computation of width of QMainWindow is not correct!
//	resizeEvent(0);
//	eventFilter(saagharWidget->tableViewWidget->viewport(), 0);
//}

//void MainWindow::resizeEvent( QResizeEvent * event )
//{
//	saagharWidget->resizeTable(saagharWidget->tableViewWidget);
//	if (event)
//		QMainWindow::resizeEvent(event);
//}

void MainWindow::closeEvent( QCloseEvent * event )
{
	QFontDatabase::removeAllApplicationFonts();
	if (lineEditSearchText)
		lineEditSearchText->searchStop();
	saveSettings();
	event->accept();
}

void MainWindow::tableItemMouseOver(QTableWidgetItem *item)
{
	if (!item)
		return;
	QTableWidget *senderTable = item->tableWidget();
	if (!saagharWidget || !senderTable)	return;

	QVariant itemData = item->data(Qt::UserRole);

	if (itemData.isValid() && !itemData.isNull())
	{
		QString dataString = itemData.toString();
		if (dataString.startsWith("CatID") || dataString.startsWith("PoemID"))
		{
			senderTable->setCursor(QCursor(Qt::PointingHandCursor));

			if (!(item->flags() & Qt::ItemIsSelectable))
			{
				senderTable->setCurrentItem(item);
				QImage image(":/resources/images/select-mask.png");
				item->setBackground(QBrush(image.scaledToHeight( senderTable->rowHeight( item->row() ) )));
			}
			else
			{
				QImage image(":/resources/images/select-mask.png");
				item->setBackground(QBrush(image.scaledToHeight( senderTable->rowHeight( item->row() ) )));
			}
		}
		else
			senderTable->unsetCursor();
	}
	else
		senderTable->unsetCursor();

	if (SaagharWidget::lastOveredItem && SaagharWidget::lastOveredItem!=item /*&& (item->flags() & Qt::ItemIsSelectable)*/ )
	{
		SaagharWidget::lastOveredItem->setBackground(QBrush(QImage()));//unset background
	}

	SaagharWidget::lastOveredItem = item;
}

void MainWindow::tableCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
	if (previous)
	{
		QImage image(":/resources/images/select-mask.png");
		previous->setBackground(QBrush(QImage()));
	}
	if (current)//maybe create a bug! check older codes!!
	{
		QImage image(":/resources/images/select-mask.png");
		current->setBackground(QBrush(image));
	}
}

void MainWindow::tableItemPress(QTableWidgetItem *)
{
	pressedMouseButton = QApplication::mouseButtons();
}

void MainWindow::tableItemClick(QTableWidgetItem *item)
{
	QTableWidget *senderTable = item->tableWidget();
	if (!saagharWidget || !senderTable)	return;

	disconnect(senderTable, SIGNAL(itemClicked(QTableWidgetItem *)), 0, 0);
	QStringList itemData = item->data(Qt::UserRole).toString().split("=", QString::SkipEmptyParts);
	
	//qDebug() << "itemData=" << item->data(Qt::UserRole).toString() << "-itemData-Size=" << itemData.size();

	if (itemData.size()!=2 || itemData.at(0) == "VerseData" )
	{
		connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClick(QTableWidgetItem *)));
		return;
	}

	//search data
	QStringList searchDataList = item->data(ITEM_SEARCH_DATA).toStringList();
//	QStringList searchDataList = QStringList();
	QString searchPhraseData = "";//searchData.toString();
	QString searchVerseData = "";
	if (searchDataList.size() == 2)
	{
		searchPhraseData = searchDataList.at(0);
		searchVerseData = searchDataList.at(1);
	}
//	if (searchData.isValid() && !searchData.isNull())
//	{
//		searchDataList = searchData.toString().split("|", QString::SkipEmptyParts);

//		searchPhraseData = searchDataList.at(0);
//	}

	bool OK = false;
	int idData = itemData.at(1).toInt(&OK);
	bool noError = false;

	if (OK && SaagharWidget::ganjoorDataBase)
		noError = true;

	//right click event
	if (pressedMouseButton == Qt::RightButton)
	{
		//qDebug() << "tableItemClick-Qt::RightButton";

		newTabForItem(itemData.at(0), idData, noError);

		saagharWidget->scrollToFirstItemContains(searchVerseData, false);
		SaagharItemDelegate *searchDelegate = new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), searchPhraseData);
		saagharWidget->tableViewWidget->setItemDelegate(searchDelegate);
		connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), searchDelegate, SLOT(keywordChanged(const QString &)) );

		connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClick(QTableWidgetItem *)));
		return;
	}

	saagharWidget->processClickedItem(itemData.at(0), idData, noError);

	saagharWidget->scrollToFirstItemContains(searchVerseData, false);
	SaagharItemDelegate *searchDelegate = new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), searchPhraseData);
	saagharWidget->tableViewWidget->setItemDelegate(searchDelegate);
	connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), searchDelegate, SLOT(keywordChanged(const QString &)) );

	//qDebug() << "emit updateTabsSubMenus()--2306";
	//resolved by signal SaagharWidget::captionChanged()
	//updateTabsSubMenus();
	connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClick(QTableWidgetItem *)));
}

void MainWindow::importDataBase(const QString fileName)
{
	QFileInfo dataBaseFile(QSqlDatabase::database("ganjoor.s3db").databaseName());
	if (!dataBaseFile.isWritable())
	{
		QMessageBox::warning(this, tr("Error!"), tr("You have not write permission to database file, the import procedure can not proceed.\nDataBase Path: %2").arg(dataBaseFile.fileName()));
		return;
	}
	QList<GanjoorPoet *> poetsConflictList = SaagharWidget::ganjoorDataBase->getConflictingPoets(fileName);
	
	SaagharWidget::ganjoorDataBase->dBConnection.transaction();

	if ( !poetsConflictList.isEmpty() )
	{
		QMessageBox warnAboutConflict(this);
		warnAboutConflict.setWindowTitle(tr("Warning!"));
		warnAboutConflict.setIcon(QMessageBox::Warning);
		warnAboutConflict.setText(tr("There are some conflict with your installed database. If you continue, these poets will be removed!"));
		QString details = tr("These poets are present in installed database:\n");
		for (int i = 0; i<poetsConflictList.size(); ++i)
		{
			details += poetsConflictList.at(i)->_Name+"\n";
		}
		warnAboutConflict.setDetailedText(details);
		warnAboutConflict.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
		warnAboutConflict.setEscapeButton(QMessageBox::Cancel);
		warnAboutConflict.setDefaultButton(QMessageBox::Cancel);
		int ret = warnAboutConflict.exec();
		if ( ret == QMessageBox::Cancel)
			return;

		foreach(GanjoorPoet *poet, poetsConflictList)
			SaagharWidget::ganjoorDataBase->removePoetFromDataBase(poet->_ID);
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if (SaagharWidget::ganjoorDataBase->importDataBase(fileName))
	{
		SaagharWidget::ganjoorDataBase->dBConnection.commit();
		comboBoxSearchRegion->clear();
		QListWidgetItem *item = selectSearchRange->insertRow(0, tr("All Opened Tab"), true, "ALL_OPENED_TAB", Qt::UserRole, true);
		multiSelectObjectInitialize(selectSearchRange, selectedSearchRange);
		item->setCheckState(selectedSearchRange.contains("ALL_OPENED_TAB") ? Qt::Checked : Qt::Unchecked);
		setHomeAsDirty();
		//update visible region of searchToolBar
//		bool tmpFlag = labelMaxResultAction->isVisible();
//		labelMaxResultAction->setVisible(!tmpFlag);
//		ui->searchToolBar->update();
//		QApplication::processEvents();
//		labelMaxResultAction->setVisible(tmpFlag);
	}
	else
	{
		SaagharWidget::ganjoorDataBase->dBConnection.rollback();
		QMessageBox::warning(this, tr("Error!"), tr("There are some errors, the import procedure was not completed"));
	}

	QApplication::restoreOverrideCursor();
}

void MainWindow::getMaxResultPerPage()
{
	bool Ok = false;
	int max = QInputDialog::getInt(this, tr("Results Per Page"), tr("Set max results per page:"), SearchResultWidget::maxItemPerPage, 0, 5000, 5, &Ok );
	if (Ok)
	{
		emit maxItemPerPageChanged(max);
		SearchResultWidget::maxItemPerPage = max;
		//SearchResultWidget::setMaxItemPerPage(max);
	}
}

void MainWindow::toolBarContextMenu(const QPoint &/*pos*/)
{
	QMenu *contextMenu = new QMenu(0);
	toolBarViewActions(ui->mainToolBar, contextMenu, false);

	//it has not a friendly view!
//	contextMenu->addSeparator();
//	QMenu *otherActions =	contextMenu->addMenu(tr("Other Actions"));
//	QMap<QString, QAction *>::const_iterator actIterator = allActionMap.constBegin();
//	while (actIterator != allActionMap.constEnd())
//	{
//		if (!mainToolBarItems.contains(actIterator.key()))
//		{
//			otherActions->addAction(actIterator.value());
//		}
//		++actIterator;
//	}

	QAction *customizeRandom = 0;
	if (mainToolBarItems.contains("actionFaal", Qt::CaseInsensitive) || mainToolBarItems.contains("actionRandom", Qt::CaseInsensitive) )
	{
		contextMenu->addSeparator();
		customizeRandom = contextMenu->addAction(tr("Customize Faal && Random...")/*, customizeRandomButtons()*/);
	}
//	else
//	{
//		customizeRandom = otherActions->addAction(tr("Customize Faal && Random...")/*, customizeRandomButtons()*/);
//	}

	if (contextMenu->exec(QCursor::pos()) == customizeRandom)
	{
		customizeRandomDialog();
	}

	delete contextMenu;
}

void MainWindow::customizeRandomDialog()
{
	CustomizeRandomDialog *randomSetting = new CustomizeRandomDialog(this, randomOpenInNewTab);

	QListWidgetItem *firstItem = randomSetting->selectRandomRange->insertRow(0, tr("Current tab's subsections"), true, "CURRENT_TAB_SUBSECTIONS", Qt::UserRole, true);
	multiSelectObjectInitialize(randomSetting->selectRandomRange, selectedRandomRange);
	firstItem->setCheckState(selectedRandomRange.contains("CURRENT_TAB_SUBSECTIONS") ? Qt::Checked : Qt::Unchecked);

	if (randomSetting->exec())
	{
		randomSetting->acceptSettings(&randomOpenInNewTab);
		//randomSetting->selectRandomRange->updateSelectedLists();
		QList<QListWidgetItem *> selectedItems = randomSetting->selectRandomRange->getSelectedItemList();

		selectedRandomRange.clear();
		foreach(QListWidgetItem *item, selectedItems)
		{
			selectedRandomRange << item->data(Qt::UserRole).toString();
		}

		qDebug() << "selectedItems-size="<<selectedItems.size();
		qDebug() << "STR="<<randomSetting->selectRandomRange->getSelectedString();
		qDebug() << selectedRandomRange.join("|");
		qDebug() << "randomOpenInNewTab=" << randomOpenInNewTab;
	}
	delete randomSetting;
	randomSetting = 0;
}
#include<QActionGroup>
void MainWindow::toolBarViewActions(QToolBar *toolBar, QMenu *menu, bool subMenu)
{
	QMenu *toolbarMenu = menu;
	if (subMenu)
	{
		toolbarMenu = new QMenu( toolBar->windowTitle() , menu );
		menu->addMenu(toolbarMenu);
	}

	toolbarMenu->addAction(actionInstance("actionToolBarSizeLargeIcon","",QObject::tr("&Large Icon")) );
	QActionGroup *toolBarIconSize = actionInstance("actionToolBarSizeLargeIcon")->actionGroup();
	if (!toolBarIconSize)
	{
		toolBarIconSize = new QActionGroup(this);
		qDebug() << "toolBarIconSize CREATED";
	}
	const int normalSize = toolBar->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
	const int largeSize = toolBar->style()->pixelMetric(QStyle::PM_LargeIconSize);
	const int smallSize = toolBar->style()->pixelMetric(QStyle::PM_SmallIconSize);

	actionInstance("actionToolBarSizeLargeIcon")->setParent(toolBar);
	actionInstance("actionToolBarSizeLargeIcon")->setActionGroup(toolBarIconSize);
	actionInstance("actionToolBarSizeLargeIcon")->setCheckable(true);
	actionInstance("actionToolBarSizeLargeIcon")->setData(largeSize);

	toolbarMenu->addAction(actionInstance("actionToolBarSizeMediumIcon","",QObject::tr("&Medium Icon")) );
	actionInstance("actionToolBarSizeMediumIcon")->setParent(toolBar);
	actionInstance("actionToolBarSizeMediumIcon")->setActionGroup(toolBarIconSize);
	actionInstance("actionToolBarSizeMediumIcon")->setCheckable(true);
	actionInstance("actionToolBarSizeMediumIcon")->setData(normalSize);

	toolbarMenu->addAction(actionInstance("actionToolBarSizeSmallIcon","",QObject::tr("&Small Icon")) );
	actionInstance("actionToolBarSizeSmallIcon")->setParent(toolBar);
	actionInstance("actionToolBarSizeSmallIcon")->setActionGroup(toolBarIconSize);
	actionInstance("actionToolBarSizeSmallIcon")->setCheckable(true);
	actionInstance("actionToolBarSizeSmallIcon")->setData(smallSize);//qMax(normalSize-16, normalSize/2) );//qMin(normalSize, 16) == 16 ? normalSize-16 : normalSize/2 );//we sure we don't set it to zero!

	toolbarMenu->addSeparator();

	toolbarMenu->addAction(actionInstance("actionToolBarStyleOnlyIcon","",QObject::tr("Only &Icon")) );
	QActionGroup *toolBarButtonStyle = actionInstance("actionToolBarStyleOnlyIcon")->actionGroup();
	if (!toolBarButtonStyle)
	{
		toolBarButtonStyle = new QActionGroup(this);
		qDebug() << "toolBarButtonStyle CREATED";
	}
	actionInstance("actionToolBarStyleOnlyIcon")->setParent(toolBar);
	actionInstance("actionToolBarStyleOnlyIcon")->setActionGroup(toolBarButtonStyle);
	actionInstance("actionToolBarStyleOnlyIcon")->setCheckable(true);
	actionInstance("actionToolBarStyleOnlyIcon")->setData(Qt::ToolButtonIconOnly);

	toolbarMenu->addAction(actionInstance("actionToolBarStyleOnlyText","",QObject::tr("Only &Text")) );
	actionInstance("actionToolBarStyleOnlyText")->setParent(toolBar);
	actionInstance("actionToolBarStyleOnlyText")->setActionGroup(toolBarButtonStyle);
	actionInstance("actionToolBarStyleOnlyText")->setCheckable(true);
	actionInstance("actionToolBarStyleOnlyText")->setData(Qt::ToolButtonTextOnly);

	toolbarMenu->addAction(actionInstance("actionToolBarStyleTextIcon","",QObject::tr("&Both Text && Icon")) );
	actionInstance("actionToolBarStyleTextIcon")->setParent(toolBar);
	actionInstance("actionToolBarStyleTextIcon")->setActionGroup(toolBarButtonStyle);
	actionInstance("actionToolBarStyleTextIcon")->setCheckable(true);
	actionInstance("actionToolBarStyleTextIcon")->setData(Qt::ToolButtonTextUnderIcon);

	//checked actions
	actionInstance(Settings::READ("MainToolBar Style", "actionToolBarStyleTextIcon").toString())->setChecked(true);
	actionInstance(Settings::READ("MainToolBar Size", "actionToolBarSizeMediumIcon").toString())->setChecked(true);

	//create actiongroups connections
	connect(toolBarIconSize, SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));
	connect(toolBarButtonStyle, SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));
}

void MainWindow::toolbarViewChanges(QAction *action)
{
	if (!action) return;
	QToolBar *toolbar = qobject_cast<QToolBar *>(action->parent());
	if (!toolbar)
	{
		qDebug() << "toolbar is not recived";
		return;
	}

	bool ok = false;
	int data = action->data().toInt(&ok);
	if (!ok)
	{
		qDebug() << "data is nor recived";
		return;
	}

	disconnect(action->actionGroup(), SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));

	QString actionId = allActionMap.key(action);
	QString actionType = actionId.contains("Size") ? "SizeType" : "StyleType";
	qDebug() << "actionType=" << actionType;
	if (actionType == "SizeType")
	{
		toolbar->setIconSize(QSize(data, data));
		Settings::WRITE("MainToolBar Size", actionId);
	}
	else if (actionType == "StyleType")
	{
		toolbar->setToolButtonStyle((Qt::ToolButtonStyle)data);
		Settings::WRITE("MainToolBar Style", actionId);
	}

	connect(action->actionGroup(), SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));
}

bool MainWindow::eventFilter(QObject* receiver, QEvent* event)
{
//	if (skipSearchToolBarResize)
//	{
//		qDebug() << "skipSearchToolBarResize=TRUE";
//		skipSearchToolBarResize = false;
//		return false;
//	}
//	else
//		skipSearchToolBarResize = true;
	QEvent::Type eventType = QEvent::None;
	if (event)
		eventType = event->type();
		
	switch (eventType/*event->type()*/)
	{
		case QEvent::Resize :
		case QEvent::None :
		{
//			QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
			if (saagharWidget &&
				saagharWidget->tableViewWidget &&
				receiver == saagharWidget->tableViewWidget->viewport())
			{
				saagharWidget->resizeTable(saagharWidget->tableViewWidget);
			}
		}
			break;
		case QEvent::Move :
		{
			QMoveEvent *resEvent = static_cast<QMoveEvent*>(event);
			if (resEvent && receiver == ui->searchToolBar)
			{
				if (searchToolBarBoxLayout->direction() != QBoxLayout::LeftToRight && ui->searchToolBar->isFloating())
				{
					qDebug()<<"isFloating()=" << ui->searchToolBar->isFloating();
					searchToolBarBoxLayout->setDirection(QBoxLayout::LeftToRight);
					searchToolBarBoxLayout->setSpacing(4);
					searchToolBarBoxLayout->setContentsMargins(1,1,1,1);
					ui->searchToolBar->adjustSize();
				}
				int height = comboBoxSearchRegion->height()+lineEditSearchText->height()+1+1+2;//margins and spacing
				if (ui->searchToolBar->size().height()>=height && !ui->searchToolBar->isFloating())
				{
					searchToolBarBoxLayout->setDirection(QBoxLayout::TopToBottom);
					searchToolBarBoxLayout->setSpacing(2);
					searchToolBarBoxLayout->setContentsMargins(1,1,1,1);
				}
//				else
//				{
//					searchToolBarBoxLayout->setDirection(QBoxLayout::RightToLeft);
//				}
			}
		}
			break;
		default:
			break;
	}
	skipSearchToolBarResize = false;
	return false;
}

void MainWindow::setupSearchToolBarUi()
{
	//initialize Search ToolBar
	QString clearIconPath = currentIconThemePath()+"/clear-left.png";
	if (layoutDirection() == Qt::RightToLeft)
		clearIconPath = currentIconThemePath()+"/clear-right.png";
	lineEditSearchText = new QSearchLineEdit(ui->searchToolBar, clearIconPath, currentIconThemePath()+"/search-options.png", currentIconThemePath()+"/cancel.png");
	lineEditSearchText->setObjectName(QString::fromUtf8("lineEditSearchText"));
	lineEditSearchText->setMaximumSize(QSize(170, 16777215));
	lineEditSearchText->setLayoutDirection(Qt::RightToLeft);
	
	selectSearchRange = new QMultiSelectWidget(ui->searchToolBar);
	comboBoxSearchRegion = selectSearchRange->getComboWidgetInstance();
	comboBoxSearchRegion->setObjectName(QString::fromUtf8("comboBoxSearchRegion"));
	comboBoxSearchRegion->setLayoutDirection(Qt::RightToLeft);
	comboBoxSearchRegion->setMaximumSize(QSize(170, 16777215));
	comboBoxSearchRegion->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
	comboBoxSearchRegion->setEditable(true);
#if QT_VERSION > 0x040700
	lineEditSearchText->setPlaceholderText(tr("Enter Search Phrase"));
	comboBoxSearchRegion->lineEdit()->setPlaceholderText(tr("Select Search Scope..."));
#else
	lineEditSearchText->setToolTip(tr("Enter Search Phrase"));
	comboBoxSearchRegion->lineEdit()->setToolTip(tr("Select Search Scope..."));
#endif
	
	//create layout and add widgets to it!
	searchToolBarBoxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	
	searchToolBarBoxLayout->setSpacing(4);
	searchToolBarBoxLayout->setContentsMargins(1,1,1,1);
	
	searchToolBarBoxLayout->addWidget(lineEditSearchText);
	searchToolBarBoxLayout->addWidget(comboBoxSearchRegion);
	QSpacerItem *horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
	searchToolBarBoxLayout->addItem(horizontalSpacer);
	
	QWidget *searchToolBarContent = new QWidget();

	QHBoxLayout *horizontalStretch = new QHBoxLayout;
	horizontalStretch->setSpacing(0);
	horizontalStretch->setContentsMargins(0,0,0,0);
	horizontalStretch->addLayout(searchToolBarBoxLayout);
	horizontalStretch->addStretch(10000);
	searchToolBarContent->setLayout(horizontalStretch);

	ui->searchToolBar->addWidget(searchToolBarContent);

	searchOptionMenu = new QMenu(ui->searchToolBar);
	
	actionInstance("actionGetMaxResultPerPage", "", tr("Max Result per Page...") );
	connect(actionInstance("actionGetMaxResultPerPage"), SIGNAL(triggered()), this, SLOT(getMaxResultPerPage()));
	
	actionInstance("actionSearchTips", "", tr("&Search Tips...") );

	//connect(actionInstance("actionSearchTips"), SIGNAL(toggled(bool)), this, SLOT(newSearchNonAlphabetChanged(bool)));

	searchOptionMenu->addAction(actionInstance("actionGetMaxResultPerPage"));
	searchOptionMenu->addAction(actionInstance("separator"));
	searchOptionMenu->addAction(actionInstance("actionSearchTips"));

	connect(lineEditSearchText->optionsButton(), SIGNAL(clicked()), this, SLOT(showSearchOptionMenu()));
}

void MainWindow::namedActionTriggered(bool checked)
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action) return;
	disconnect(action, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));

	QString actionName = action->objectName();
	//qDebug() << "triggred=" << actionName << "checked=" << checked;
	QString text = action->text();text.remove("&");
	if ( actionName == "Show Photo at Home" )
	{
		Settings::WRITE(actionName, checked);
		//TODO: reload Home page!!!!!!!
		setHomeAsDirty();
	}
	else if ( actionName == "menuOpenedTabsActions" )
	{
		QObject *obj = qvariant_cast<QObject *>(action->data());
		QWidget *tabWidget = qobject_cast<QWidget *>(obj);
		if (tabWidget)
		{
			mainTabWidget->setCurrentWidget(tabWidget);
		}
		return;//action is deleted by 'updateTabsSubMenus()'
	}
	else if ( actionName == "Lock ToolBars" )
	{
		Settings::WRITE(actionName, checked);
		ui->mainToolBar->setMovable(!checked);
		if (ui->menuToolBar)
			ui->menuToolBar->setMovable(!checked);
		ui->searchToolBar->setMovable(!checked);
		parentCatsToolBar->setMovable(!checked);
	}
	else if ( actionName == "actionSearchTips" )
	{
		QString searchTips = tr("<b>Tip1:</b> Search operators and commands:"
					"%5"
					"<TR><TD%3 id=\"and-operator\"><b>%1+%2</b></TD>%8<TD%4>Mesras containing both <b>%1</b> and <b>%2</b>, <i>at any order</i>.</TD></TR>"
					"<TR><TD%3><b>%1**%2</b></TD>%8<TD%4>Mesras containing both <b>%1</b> and <b>%2</b>, <i>at this order</i>.</TD></TR>"
					"<TR><TD%3><b>%1 %2</b></TD>%8<TD%4>Same as <b><a href=\"#and-operator\">%1+%2</a></b>.</TD></TR>"
					"<TR><TD%3><b>%1 | %2</b></TD>%8<TD%4>Mesras containing <b>%1</b> or <b>%2</b>, or both.</TD></TR>"
					"<TR><TD%3><b>%1 -%2</b></TD>%8<TD%4>Mesras containing <b>%1</b>, but not <b>%2</b>.</TD></TR>"
					"<TR><TD%3><b>\"%1\"</b></TD>%8<TD%4>Mesras containing the whole word <b>%1</b>.</TD></TR>"
					"<TR><TD%3><b>\"%1 %2\"</b></TD>%8<TD%4>Mesras containing the whole mixed word <b>%1 %2</b>.</TD></TR>"
					"<TR><TD%3><b>Sp*ng</b></TD>%8<TD%4>Mesras containing any phrase started with <b>Sp</b> and ended with <b>ng</b>; i.e: Mesras containing <b>Spring</b> or <b>Spying</b> or <b>Spoking</b> or...</TD></TR>"
					"</TBODY></TABLE><br />"
					"<br /><b>Tip2:</b> All search queries are case insensitive.<br />"
					"<br /><b>Tip3:</b> User can use an operator more than once;"
						"<br />i.e: <b>%1+%2+%6</b>, <b>%1 -%6 -%7</b>, <b>%1**%2**%7</b> and <b>S*r*g</b> are valid search terms.<br />"
					"<br /><b>Tip4:</b> User can use operators mixed together;"
						"<br />i.e: <b>\"%1\"+\"%2\"+%6</b>, <b>%1+%2|%6 -%7</b>, <b>\"%1\"**\"%2\"</b>, <b>S*r*g -Spring</b> and <b>\"Gr*en\"</b> are valid search terms.<br />"
					"<br />")
				.arg(tr("Spring")).arg(tr("Flower")).arg(tr(" ALIGN=CENTER")).arg(tr(" ALIGN=Left"))
				.arg(tr("<TABLE DIR=LTR FRAME=VOID CELLSPACING=5 COLS=3 RULES=ROWS BORDER=0><TBODY>")).arg(tr("Rain")).arg(tr("Sunny"))
				.arg(tr("<TD  ALIGN=Left>:</TD>"));
		QTextBrowserDialog searchTipsDialog(this, tr("Search Tips..."), searchTips, QPixmap(currentIconThemePath()+"/search.png").scaledToHeight(64, Qt::SmoothTransformation));
		searchTipsDialog.exec();
	}
	else if ( actionName == "Ganjoor Verification" )
	{
		QDesktopServices::openUrl(QString("http://saaghar.sourceforge.net/redirect.php?sender=Saaghar&section=ganjoor-vocr&url=%1").arg("http://v.ganjoor.net"));
	}
	else if (actionName == "fixedNameRedoAction" ||
			 actionName == "fixedNameUndoAction" ||
			 actionName == "globalRedoAction" ||
			 actionName == "globalUndoAction" )
	{
		if (actionName == "fixedNameRedoAction")
			emit globalRedoAction->activate(QAction::Trigger);
		if (actionName == "fixedNameUndoAction")
			emit globalUndoAction->activate(QAction::Trigger);

		actionInstance("fixedNameRedoAction")->setEnabled(globalRedoAction->isEnabled());
		actionInstance("fixedNameUndoAction")->setEnabled(globalUndoAction->isEnabled());
	}

	connect(action, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));
}

void MainWindow::updateTabsSubMenus()
{
	menuOpenedTabs->clear();
	//menuOpenedTabs->setTearOffEnabled(true);
	QAction *tabAction;
	int numOfTabs = mainTabWidget->count();
	for (int i=0; i<numOfTabs; ++i)
	{
		tabAction = new QAction(mainTabWidget->tabText(i), menuOpenedTabs);

		QObject *obj = mainTabWidget->widget(i);
		QVariant data = QVariant::fromValue(obj);
		tabAction->setData(data);

		tabAction->setObjectName("menuOpenedTabsActions");
		connect(tabAction, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));
		menuOpenedTabs->addAction(tabAction);
	}
	qDebug() << "end::updateTabsSubMenus";
}

void MainWindow::actionClosedTabsClicked()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action) return;
	//QString tabType = action->data().toString();
	QString type = action->data().toStringList().at(0);
	int id = action->data().toStringList().at(1).toInt();

	if ( (type != "PoemID" && type != "CatID") || id <0 )
		newTabForItem(type, id, false);
	else
		newTabForItem(type, id, true);
	//QStringList tabViewData = action->data().toString().split("=", QString::SkipEmptyParts);
//	if (tabViewData.size() == 2 && (tabViewData.at(0) == "PoemID" || tabViewData.at(0) == "CatID") )
//	{
//		bool Ok = false;
//		int id = tabViewData.at(1).toInt(&Ok);
//		if (Ok)
//			newTabForItem(tabViewData.at(0), id, true);
//	}
	menuClosedTabs->removeAction(action);
}

//bool MainWindow::dataFromIdentifier(const QString &identifier, QString *type, int *id)
//{
//	QStringList tabViewData = identifier.split("=", QString::SkipEmptyParts);
//	if (tabViewData.size() == 2 && (tabViewData.at(0) == "PoemID" || tabViewData.at(0) == "CatID") )
//	{
//		bool Ok = false;
//		int ID = tabViewData.at(1).toInt(&Ok);
//		if (Ok)
//		{
//			if (type) *type = tabViewData.at(0);
//			if (id) *id = ID;
//			return true;
//		}
//	}
//	return false;
//}

QString MainWindow::currentIconThemePath()
{
	if (!settingsIconThemePath.isEmpty() && settingsIconThemeState)
		return settingsIconThemePath;
	return ":/resources/images/";
}

void MainWindow::setHomeAsDirty()
{
	for (int i = 0; i < mainTabWidget->count(); ++i)
	{
		SaagharWidget *tmp = getSaagharWidget(i);
		if (tmp && tmp->currentCat==0)
		{
			tmp->setDirty();//needs to be refreshed
		}
	}

	if (saagharWidget && saagharWidget->isDirty())
		saagharWidget->showHome();
}

void MainWindow::setupBookmarkManagerUi()
{
	QDockWidget *bookmarkManagerWidget = new QDockWidget(tr("Bookmarks"), this);
	bookmarkManagerWidget->setObjectName("bookMarkWidget");
	bookmarkManagerWidget->hide();

	QWidget *bookmarkContainer = new QWidget(bookmarkManagerWidget);
	QVBoxLayout *bookmarkMainLayout = new QVBoxLayout;
	QHBoxLayout *bookmarkToolsLayout = new QHBoxLayout;

	SaagharWidget::bookmarks = new Bookmarks(this);

	connect(SaagharWidget::bookmarks, SIGNAL(showBookmarkedItem(QString,QString,QString,bool)), this, SLOT(ensureVisibleBookmarkedItem(QString,QString,QString,bool)));

	QString clearIconPath = currentIconThemePath()+"/clear-left.png";
	if (layoutDirection() == Qt::RightToLeft)
		clearIconPath = currentIconThemePath()+"/clear-right.png";

	QLabel *bookmarkFilterLabel = new QLabel(bookmarkManagerWidget);
	bookmarkFilterLabel->setObjectName(QString::fromUtf8("bookmarkFilterLabel"));
	bookmarkFilterLabel->setText(tr("Filter:"));
	QSearchLineEdit *bookmarkFilter = new QSearchLineEdit(bookmarkManagerWidget, clearIconPath, currentIconThemePath()+"/filter.png");
	bookmarkFilter->setObjectName("bookmarkFilter");
#if QT_VERSION > 0x040700
	bookmarkFilter->setPlaceholderText(tr("Filter"));
#else
	bookmarkFilter->setToolTip(tr("Filter"));
#endif
	connect(bookmarkFilter, SIGNAL(textChanged(const QString &)), SaagharWidget::bookmarks, SLOT(filterItems(const QString &)));

	QToolButton *unBookmarkButton = new QToolButton(bookmarkManagerWidget);
	unBookmarkButton->setObjectName("unBookmarkButton");
	unBookmarkButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

	unBookmarkButton->setIcon(QIcon(currentIconThemePath()+"/un-bookmark.png"));
	connect(unBookmarkButton, SIGNAL(clicked()), SaagharWidget::bookmarks, SLOT(unBookmarkItem()));

	QSpacerItem *filterHorizSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	bookmarkToolsLayout->addWidget(bookmarkFilterLabel, 0, Qt::AlignRight|Qt::AlignCenter);
	bookmarkToolsLayout->addWidget(bookmarkFilter);
	bookmarkToolsLayout->addItem(filterHorizSpacer);
	bookmarkToolsLayout->addWidget(unBookmarkButton);
	//bookmarkMainLayout->addWidget(bookmarkFilter);
	bookmarkMainLayout->addLayout(bookmarkToolsLayout);
	bookmarkMainLayout->addWidget(SaagharWidget::bookmarks);
	bookmarkContainer->setLayout(bookmarkMainLayout);

	QFile bookmarkFile(resourcesPath+"/bookmarks.xbel");

	bool readBookmarkFile = true;

	if (bookmarkFile.open(QFile::ReadOnly | QFile::Text))
	{
		if (!SaagharWidget::bookmarks->read(&bookmarkFile))
		{
			readBookmarkFile = false;
		}
	}
	else
		readBookmarkFile = false;

	if (!readBookmarkFile)
	{
		bookmarkFile.close();
		if (bookmarkFile.open(QFile::WriteOnly | QFile::Text))
		{
			//create an empty XBEL file.
			QTextStream out(&bookmarkFile);
			out.setCodec("utf-8");
			out << "<?xml version='1.0' encoding='UTF-8'?>\n"
				<<	"<!DOCTYPE xbel>\n"
				<<	"<xbel version=\"1.0\">\n"
				<<	"</xbel>";
			bookmarkFile.close();
			if (bookmarkFile.open(QFile::ReadOnly | QFile::Text))
			{
				if (SaagharWidget::bookmarks->read(&bookmarkFile))
					readBookmarkFile = true;
			}
		}
	}

	if (readBookmarkFile)
	{
		bookmarkManagerWidget->setWidget(bookmarkContainer /*SaagharWidget::bookmarks*/);
		addDockWidget(Qt::LeftDockWidgetArea, bookmarkManagerWidget);
		allActionMap.insert("bookmarkManagerDockAction", bookmarkManagerWidget->toggleViewAction());
		actionInstance("bookmarkManagerDockAction")->setIcon(QIcon(currentIconThemePath()+"/bookmark-folder.png"));
		actionInstance("bookmarkManagerDockAction")->setObjectName(QString::fromUtf8("bookmarkManagerDockAction"));
	}
	else
	{
		//bookmark not loaded!
		delete SaagharWidget::bookmarks;
		delete bookmarkContainer;
		bookmarkContainer = 0;
		SaagharWidget::bookmarks = 0;
		delete bookmarkManagerWidget;
		bookmarkManagerWidget = 0;
	}
}

void MainWindow::ensureVisibleBookmarkedItem(const QString &type, const QString &itemText, const QString &data, bool ensureVisible)
{
	if (type == "Verses" || type == tr("Verses"))
	{
		int poemID = data.split("|").at(0).toInt();
		//QString poemIdentifier = "PoemID="+data.split("|").at(0);
		for (int i = 0; i < mainTabWidget->count(); ++i)
		{
			SaagharWidget *tmp = getSaagharWidget(i);
			if (tmp)
			{
				if (tmp->identifier().at(0) == "PoemID" && tmp->identifier().at(1).toInt() == poemID)
				{
					if (ensureVisible)
					{
						mainTabWidget->setCurrentIndex(i);
						saagharWidget->scrollToFirstItemContains(itemText, false, true);
						return;
					}

					QTableWidgetItem *item = tmp->scrollToFirstItemContains(itemText, false, ensureVisible);
					if (!ensureVisible)
					{
						//un-bookmarking operation!!
						if (item)
						{
							QTableWidgetItem *numItem = tmp->tableViewWidget->item(item->row(), 0);
							if (numItem)
							{
								QPixmap starOff(":/resources/images/bookmark-off.png");
								starOff = starOff.scaledToHeight(qMin(tmp->tableViewWidget->rowHeight(numItem->row())-1, 22), Qt::SmoothTransformation);
								//QIcon bookmarkIcon;
								//bookmarkIcon.addPixmap(starOff, QIcon::Disabled, QIcon::Off);
								//= numItem->icon().pixmap(starOff.size(), QIcon::Disabled, QIcon::Off);

								numItem->setIcon(QIcon(starOff));
								const int ITEM_BOOKMARKED_STATE = Qt::UserRole+20;
								numItem->setData(ITEM_BOOKMARKED_STATE, false);

//								QIcon bookmarkIcon;
//								bookmarkIcon.addPixmap(star, QIcon::Active, QIcon::On);
//								bookmarkIcon.addPixmap(starOff, QIcon::Disabled, QIcon::Off);
//								bool bookmarkState = !item->data(ITEM_BOOKMARKED_STATE).toBool();
					
//								if (bookmarkState)
//									bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Active, QIcon::On);
//								else
//									bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Disabled, QIcon::Off);
							}
						}
					}
				}
			}
		}

		if (ensureVisible)
		{
			newTabForItem("PoemID", poemID, true);
			saagharWidget->scrollToFirstItemContains(itemText, false, true);
		}
	}
}
