/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>       *
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

#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QTableWidget>
#include <QFontDatabase>
#include <QClipboard>
#include <QProcess>
#include <QToolButton>
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

const int ITEM_SEARCH_DATA = Qt::UserRole+10;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
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
	QString applicationFontsPath = resourcesPath + "/fonts/";
	QDir fontsDir(resourcesPath + "/fonts/");
	QStringList fontsList = fontsDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
	for (int i=0; i<fontsList.size(); ++i)
	{
		QFontDatabase::addApplicationFont(resourcesPath + "/fonts/"+fontsList.at(i));
	}

	//create Parent Categories ToolBar
	parentCatsToolBar = new QToolBar(this);
	parentCatsToolBar->setObjectName(QString::fromUtf8("parentCatsToolBar"));
	parentCatsToolBar->setLayoutDirection(Qt::RightToLeft);
	parentCatsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
	addToolBar(Qt::BottomToolBarArea, parentCatsToolBar);
	parentCatsToolBar->setWindowTitle(QApplication::translate("MainWindow", "Parent Categories", 0, QApplication::UnicodeUTF8));

	ui->setupUi(this);

	loadGlobalSettings();
	setupUi();

	if (windowState() & Qt::WindowFullScreen)
		actionInstance("actionFullScreen")->setChecked(true);
	else
		actionInstance("actionFullScreen")->setChecked(false);

	//create Tab Widget
	mainTabWidget = new QTabWidget(ui->centralWidget);
	mainTabWidget->setDocumentMode(true);
	mainTabWidget->setUsesScrollButtons(true);
	mainTabWidget->setMovable(true);

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

	ui->gridLayout->setMargin(0);
	
	ui->gridLayout->addWidget(mainTabWidget, 0, 0, 1, 1);

	if (!openedTabs.isEmpty())
	{
		QStringList openedTabsList = openedTabs.split("|", QString::SkipEmptyParts);
		if (!openedTabsList.isEmpty())
		{
			for (int i=0; i<openedTabsList.size(); ++i)
			{
				QStringList tabViewData = openedTabsList.at(i).split("=", QString::SkipEmptyParts);
				if (tabViewData.size() == 2 && (tabViewData.at(0) == "PoemID" || tabViewData.at(0) == "CatID") )
				{
					bool Ok = false;
					int id = tabViewData.at(1).toInt(&Ok);
					if (Ok)
						newTabForItem(tabViewData.at(0), id, true);
				}
			}
		}
	}

	if (mainTabWidget->count() < 1)
		insertNewTab();

	ui->centralWidget->setLayoutDirection(Qt::RightToLeft);

	connect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
	connect(mainTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloser(int)));

	createConnections();
	previousTabIndex = mainTabWidget->currentIndex();
	currentTabChanged(mainTabWidget->currentIndex());
	searchRegionsInitialize();

	//seeding random function
	uint numOfSecs = QDateTime::currentDateTime().toTime_t();
	uint seed = QCursor::pos().x()+QCursor::pos().y()+numOfSecs+QDateTime::currentDateTime().time().msec();
	qsrand(seed);
}

MainWindow::~MainWindow()
{
	delete SaagharWidget::ganjoorDataBase;
	delete ui;
}

void MainWindow::searchStart()
{
	QString phrase = lineEditSearchText->text();
	phrase.remove(" ");
	phrase.remove("\t");
	if (phrase.isEmpty())
	{
		lineEditSearchText->setText("");
		QMessageBox::information(this, tr("Error"), tr("The search phrase can not be empty."));
		return;
	}
	QString currentItemData = comboBoxSearchRegion->itemData(comboBoxSearchRegion->currentIndex(), Qt::UserRole).toString();
	
	if (currentItemData == "ALL_OPENED_TAB")
	{
		for (int i = 0; i < mainTabWidget->count(); ++i)
		{
			SaagharWidget *tmp = getSaagharWidget(i);
			//QAbstractItemDelegate *tmpDelegate = tmp->tableViewWidget->itemDelegate();

			//delete tmpDelegate;
			//tmpDelegate = 0;
			if (tmp)
				tmp->scrollToFirstItemContains(lineEditSearchText->text());
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
		showSearchResults(lineEditSearchText->text(), 0, spinBoxMaxSearchResult->value(), comboBoxSearchRegion->itemData(comboBoxSearchRegion->currentIndex(), Qt::UserRole).toInt());
	}

}

void MainWindow::searchRegionsInitialize()
{

	//comboBoxSearchRegion->addItem(tr("Current Tab"), "CURRENT_TAB");
	comboBoxSearchRegion->addItem(tr("All Opened Tab"), "ALL_OPENED_TAB");
	comboBoxSearchRegion->addItem(tr("All"), "0");

	QList<GanjoorPoet *> poets = SaagharWidget::ganjoorDataBase->getPoets();
	for(int i=0; i<poets.size(); ++i)
	{
		comboBoxSearchRegion->addItem(poets.at(i)->_Name, QString::number(poets.at(i)->_ID));
	}
}

void MainWindow::actionRemovePoet()
{
	bool ok = false;
	QStringList items;
	items << tr("Select a name...");
	QList<GanjoorPoet *> poets = SaagharWidget::ganjoorDataBase->getPoets();
	for (int i=0; i<poets.size(); ++i)
		items << poets.at(i)->_Name;
	QString item = QInputDialog::getItem(this, tr("Remove Poet"), tr("Select a poet name and click on 'OK' button, for remove it from database."), items, 0, false, &ok);
	if (ok && !item.isEmpty() && item != tr("Select a name...") )
	{
		int poetID = SaagharWidget::ganjoorDataBase->getPoet(item)._ID;
		if (poetID>0)
		{
			////
			QMessageBox warnAboutDelete(this);
			warnAboutDelete.setWindowTitle(tr("Please Notice!"));
			warnAboutDelete.setIcon(QMessageBox::Warning);
			warnAboutDelete.setText(tr("Are you sure for removing \"%1\", from database?").arg(item));
			warnAboutDelete.addButton(tr("Continue"), QMessageBox::AcceptRole);
			warnAboutDelete.setStandardButtons(QMessageBox::Cancel);
			warnAboutDelete.setEscapeButton(QMessageBox::Cancel);
			warnAboutDelete.setDefaultButton(QMessageBox::Cancel);
			int ret = warnAboutDelete.exec();
			if ( ret == QMessageBox::Cancel)
				return;
			///////
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			SaagharWidget::ganjoorDataBase->removePoetFromDataBase(poetID);
			comboBoxSearchRegion->clear();
			searchRegionsInitialize();
			//update visible region of searchToolBar
			bool tmpFlag = labelMaxResultAction->isVisible();
			labelMaxResultAction->setVisible(!tmpFlag);
			ui->searchToolBar->update();
			QApplication::processEvents();
			labelMaxResultAction->setVisible(tmpFlag);

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

		if (saagharWidget->tableViewWidget->columnCount() == 1 && saagharWidget->tableViewWidget->rowCount() > 0 && saagharWidget->currentCat != 0)
		{
			QTableWidgetItem *item = saagharWidget->tableViewWidget->item(0,0);

			if ( item && !item->icon().isNull() )
			{
				QString text = item->text();
				int textWidth = saagharWidget->tableViewWidget->fontMetrics().boundingRect(text).width();
				int verticalScrollBarWidth=0;
				if ( saagharWidget->tableViewWidget->verticalScrollBar()->isVisible() )
				{
					verticalScrollBarWidth=saagharWidget->tableViewWidget->verticalScrollBar()->width();
				}
				int totalWidth = saagharWidget->tableViewWidget->columnWidth(0)-verticalScrollBarWidth-82;
				totalWidth = qMax(82+verticalScrollBarWidth, totalWidth);
				//int numOfRow = textWidth/totalWidth ;
				saagharWidget->tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(saagharWidget->tableViewWidget->fontMetrics(), textWidth, totalWidth) );
				//saagharWidget->tableViewWidget->setRowHeight(0, 2*saagharWidget->tableViewWidget->rowHeight(0)+(saagharWidget->tableViewWidget->fontMetrics().height()*(numOfRow/*+1*/)));
			}
		}

		updateCaption();
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

void MainWindow::tabCloser(int tabIndex)
{
	if (tabIndex == mainTabWidget->currentIndex())
		SaagharWidget::lastOveredItem = 0;
	//if EditMode app need to ask about save changes!
	disconnect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

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
	tabTableWidget->horizontalHeader()->setVisible(false);
	tabTableWidget->verticalHeader()->setVisible(false);
	tabTableWidget->setMouseTracking(true);
	tabTableWidget->setAutoScroll(false);
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

	//connect(lineEditSearchText, SIGNAL(textChanged(const QString &)), saagharWidget, SLOT(scrollToFirstItemContains(const QString &)) );

	connect(saagharWidget, SIGNAL(captionChanged()), this, SLOT(updateCaption()));

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
}

void MainWindow::updateCaption()
{
	mainTabWidget->setTabText(mainTabWidget->currentIndex(), saagharWidget->currentCaption );
	setWindowTitle(QString::fromLocal8Bit("ساغر")+":"+saagharWidget->currentCaption);
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
	QProcess::startDetached("\""+QCoreApplication::applicationFilePath()+"\"");
}

void MainWindow::actionNewTabClicked()
{
	insertNewTab();
}

void MainWindow::actFullScreenClicked(bool checked)
{
	if (checked)
		setWindowState(windowState() | Qt::WindowFullScreen);
	else
		setWindowState(windowState() & ~Qt::WindowFullScreen);
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
		if (saagharWidget->currentPoem != 0)
			actionData = SaagharWidget::ganjoorDataBase->getPoem(saagharWidget->currentPoem)._CatID;
		else
			actionData = saagharWidget->currentCat;
	}

	int PoemID = SaagharWidget::ganjoorDataBase->getRandomPoemID(&actionData);
	GanjoorPoem poem = SaagharWidget::ganjoorDataBase->getPoem(PoemID);
	if (!poem.isNull() && poem._CatID == actionData)
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
	QString iconThemePath=":/resources/images/";
	if (!settingsIconThemePath.isEmpty() && settingsIconThemeState)
		iconThemePath = settingsIconThemePath;

	//initialize Search ToolBar
	QLabel *labelSearchPhrase = new QLabel(ui->searchToolBar);
	labelSearchPhrase->setObjectName(QString::fromUtf8("labelSearchPhrase"));
	labelSearchPhrase->setText(tr("Search Phrase"));

	lineEditSearchText = new QLineEdit(ui->searchToolBar);
	lineEditSearchText->setObjectName(QString::fromUtf8("lineEditSearchText"));
	lineEditSearchText->setMaximumSize(QSize(170, 16777215));
	lineEditSearchText->setLayoutDirection(Qt::RightToLeft);

	QLabel *labelSearchIn = new QLabel(ui->searchToolBar);
	labelSearchIn->setObjectName(QString::fromUtf8("labelSearchIn"));
	labelSearchIn->setText(tr("Search in"));

	comboBoxSearchRegion = new QComboBox(ui->searchToolBar);
	comboBoxSearchRegion->setObjectName(QString::fromUtf8("comboBoxSearchRegion"));
	comboBoxSearchRegion->setMinimumSize(QSize(0, 0));
	comboBoxSearchRegion->setLayoutDirection(Qt::RightToLeft);
	comboBoxSearchRegion->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	QLabel *labelMaxResult = new QLabel(ui->searchToolBar);
	labelMaxResult->setObjectName(QString::fromUtf8("labelMaxResult"));
	labelMaxResult->setText(tr("Max Result per Page"));

	spinBoxMaxSearchResult = new QSpinBox(ui->searchToolBar);
	spinBoxMaxSearchResult->setObjectName(QString::fromUtf8("spinBoxMaxSearchResult"));
	spinBoxMaxSearchResult->setValue(10);

	QToolButton *toolButtonSearchOption = new QToolButton(ui->searchToolBar);
	toolButtonSearchOption->setObjectName(QString::fromUtf8("toolButtonSearchOption"));
	toolButtonSearchOption->setIcon(QIcon(iconThemePath+"/options.png"));
	toolButtonSearchOption->setPopupMode(QToolButton::InstantPopup);
	QMenu *searchOptionMenu = new QMenu(ui->searchToolBar);
	actionInstance("actionNewSearchFlag", "", tr("&New Search Method") )->setCheckable(true);
	actionInstance("actionNewSearchSkipNonAlphabet", "", tr("&Skip non-Alphabet") )->setCheckable(true);
	actionInstance("actionNewSearchFlag")->setChecked(SaagharWidget::newSearchFlag);
	actionInstance("actionNewSearchSkipNonAlphabet")->setChecked(SaagharWidget::newSearchSkipNonAlphabet);
	actionInstance("actionNewSearchSkipNonAlphabet")->setEnabled(actionInstance("actionNewSearchFlag")->isChecked());

	connect(actionInstance("actionNewSearchFlag"), SIGNAL(toggled(bool)), this, SLOT(newSearchFlagChanged(bool)));
	connect(actionInstance("actionNewSearchSkipNonAlphabet"), SIGNAL(toggled(bool)), this, SLOT(newSearchNonAlphabetChanged(bool)));
	searchOptionMenu->addAction(actionInstance("actionNewSearchFlag"));
	searchOptionMenu->addAction(actionInstance("actionNewSearchSkipNonAlphabet"));
	toolButtonSearchOption->setMenu(searchOptionMenu);
	//pushButtonSearch->setText(tr("Search"));

	ui->searchToolBar->addWidget(labelSearchPhrase);
	ui->searchToolBar->addWidget(lineEditSearchText);
	ui->searchToolBar->addSeparator();
	ui->searchToolBar->addWidget(labelSearchIn);
	ui->searchToolBar->addWidget(comboBoxSearchRegion);
	labelMaxResultSeparator = ui->searchToolBar->addSeparator();
	labelMaxResultAction = ui->searchToolBar->addWidget(labelMaxResult);
	spinBoxMaxSearchResultAction = ui->searchToolBar->addWidget(spinBoxMaxSearchResult);
	ui->searchToolBar->addSeparator();
	ui->searchToolBar->addWidget(toolButtonSearchOption);
	
	//max result spinbox visibility
	labelMaxResultSeparator->setVisible(!actionInstance("actionNewSearchFlag")->isChecked());
	labelMaxResultAction->setVisible(!actionInstance("actionNewSearchFlag")->isChecked());
	spinBoxMaxSearchResultAction->setVisible(!actionInstance("actionNewSearchFlag")->isChecked());

	ui->searchToolBar->hide();

	//Initialize main menu items
	menuFile = new QMenu(tr("&File"), ui->menuBar);
	menuFile->setObjectName(QString::fromUtf8("menuFile"));
	menuNavigation = new QMenu(tr("&Navigation"), ui->menuBar);
	menuNavigation->setObjectName(QString::fromUtf8("menuNavigation"));
	menuView = new QMenu(tr("&View"), ui->menuBar);
	menuView->setObjectName(QString::fromUtf8("menuView"));
	menuTools = new QMenu(tr("&Tools"), ui->menuBar);
	menuTools->setObjectName(QString::fromUtf8("menuTools"));
	menuHelp = new QMenu(tr("&Help"), ui->menuBar);
	menuHelp->setObjectName(QString::fromUtf8("menuHelp"));

	ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

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

	actionInstance("actionAboutQt", ":/resources/images/qt-logo.png", tr("About &Qt") )->setMenuRole(QAction::AboutQtRole);//needed for Mac OS X

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

	//Inserting main menu items
	ui->menuBar->addMenu(menuFile);
	ui->menuBar->addMenu(menuNavigation);
	ui->menuBar->addMenu(menuView);
	ui->menuBar->addMenu(menuTools);
	ui->menuBar->addMenu(menuHelp);

////////////////
	//movable menubar
	/*QToolBar *menuToolBar = new QToolBar(this);
	menuToolBar->addWidget(ui->menuBar);
	addToolBar(Qt::TopToolBarArea, menuToolBar);*/
////////////////////////

	//Inserting items of menus
	menuFile->addAction(actionInstance("actionNewTab"));
	menuFile->addAction(actionInstance("actionNewWindow"));
	menuFile->addAction(actionInstance("actionCloseTab"));
	menuFile->addSeparator();
	menuFile->addAction(actionInstance("actionExportAsPDF"));
	menuFile->addAction(actionInstance("actionExport"));
	menuFile->addSeparator();
	menuFile->addAction(actionInstance("actionPrintPreview"));
	menuFile->addAction(actionInstance("actionPrint"));
	menuFile->addSeparator();
	menuFile->addAction(actionInstance("actionExit"));

	menuNavigation->addAction(actionInstance("actionHome"));
	menuNavigation->addSeparator();
	menuNavigation->addAction(actionInstance("actionPreviousPoem"));
	menuNavigation->addAction(actionInstance("actionNextPoem"));
	menuNavigation->addSeparator();
	menuNavigation->addAction(actionInstance("actionFaal"));
	menuNavigation->addAction(actionInstance("actionRandom"));

	menuView->addAction(actionInstance("actionFullScreen"));

	menuTools->addAction(actionInstance("searchToolbarAction"));
	menuTools->addSeparator();
	menuTools->addAction(actionInstance("actionViewInGanjoorSite"));
	menuTools->addSeparator();
	menuTools->addAction(actionInstance("actionImportNewSet"));
	menuTools->addAction(actionInstance("actionRemovePoet"));
	menuTools->addSeparator();
	menuTools->addAction(actionInstance("actionSettings"));

	menuHelp->addAction(actionInstance("actionHelpContents"));
	menuHelp->addSeparator();
	menuHelp->addAction(actionInstance("actionAboutSaaghar"));
	menuHelp->addAction(actionInstance("actionAboutQt"));

	//Inserting mainToolbar's items
	for (int i=0; i<mainToolBarItems.size(); ++i)
	{
		ui->mainToolBar->addAction(actionInstance(mainToolBarItems.at(i)));
	}
}

void MainWindow::newSearchFlagChanged(bool checked)
{
	actionInstance("actionNewSearchSkipNonAlphabet")->setEnabled(checked);
	labelMaxResultSeparator->setVisible(!checked);
	labelMaxResultAction->setVisible(!checked);
	spinBoxMaxSearchResultAction->setVisible(!checked);
	SaagharWidget::newSearchFlag = checked;
}

void MainWindow::newSearchNonAlphabetChanged(bool checked)
{
	//actionInstance("actionNewSearchSkipNonAlphabet")->setEnabled(checked);
	SaagharWidget::newSearchSkipNonAlphabet = checked;
}

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

		action = new QAction(QIcon(iconPath), displayName, this);
		action->setObjectName(actionObjectName);
		allActionMap.insert(actionObjectName, action);
	}
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
	connect(actionInstance("actionAboutSaaghar")		,	SIGNAL(triggered())		,	this, SLOT(aboutSaaghar())				);
	connect(actionInstance("actionAboutQt")				,	SIGNAL(triggered())		,	qApp, SLOT(aboutQt())					);

		//searchToolBar connections
	connect(lineEditSearchText							,	SIGNAL(returnPressed())	,	this, SLOT(searchStart())				);
	//connect(pushButtonSearch							,	SIGNAL(clicked())		,	this, SLOT(searchStart())				);
}

//Settings Dialog
void MainWindow::globalSettings()
{

	Settings *settingsDlg = new Settings(this, settingsIconThemeState, settingsIconThemePath, &allActionMap, mainToolBarItems);
	if (settingsDlg->exec())
	{
		settingsDlg->acceptSettings( &settingsIconThemeState, &settingsIconThemePath, &mainToolBarItems);
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
		saveGlobalSettings();//save new settings to disk
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

	restoreState( config->value("MainWindowState").toByteArray(), 1);
	restoreGeometry(config->value("Mainwindow Geometry").toByteArray());

	mainToolBarItems = config->value("Main ToolBar Items", "actionHome|Separator|actionPreviousPoem|actionNextPoem|Separator|actionFaal|actionRandom|Separator|actionExportAsPDF|actionCopy|searchToolbarAction|actionNewTab|Separator|actionSettings").toString().split("|", QString::SkipEmptyParts);
	QString tmp = mainToolBarItems.join("");
	tmp.remove("action", Qt::CaseInsensitive);
	tmp.remove("Separator", Qt::CaseInsensitive);
	if (tmp.isEmpty() || tmp.contains(" "))
		mainToolBarItems = QString("actionHome|Separator|actionPreviousPoem|actionNextPoem|Separator|actionFaal|actionRandom|Separator|actionExportAsPDF|actionCopy|searchToolbarAction|actionNewTab|Separator|actionSettings").split("|", QString::SkipEmptyParts);

	openedTabs = config->value("openedTabs", "").toString();

	SaagharWidget::maxPoetsPerGroup = config->value("Max Poets Per Group", 12).toInt();
	
	//search options
	SaagharWidget::newSearchFlag = config->value("New Search",true).toBool();
	SaagharWidget::newSearchSkipNonAlphabet  = config->value("New Search non-Alphabet",false).toBool();

	SaagharWidget::backgroundImageState = config->value("Background State",false).toBool();
	SaagharWidget::backgroundImagePath = config->value("Background Path", "").toString();
	settingsIconThemeState = config->value("Icon Theme State",false).toBool();
	settingsIconThemePath = config->value("Icon Theme Path", "").toString();

	QGanjoorDbBrowser::dataBasePath = config->value("DataBase Path", "").toString().split(";", QString::SkipEmptyParts);

	//The "XB Sols" is embeded in executable.
	QString fontFamily=config->value("Font Family", "XB Sols").toString();
	int fontSize=config->value( "Font Size", 18).toInt();
	QFont fnt(fontFamily,fontSize);
	SaagharWidget::tableFont = fnt;
	SaagharWidget::showBeytNumbers = config->value("Show Beyt Numbers",true).toBool();
	SaagharWidget::textColor=config->value("Text Color",QColor(0x23,0x65, 0xFF)).value<QColor>();
	SaagharWidget::matchedTextColor=config->value("Matched Text Color",QColor(0x5E, 0xFF, 0x13)).value<QColor>();
	SaagharWidget::backgroundColor=config->value("Background Color",QColor(0xFE, 0xFD, 0xF2)).value<QColor>();
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

void MainWindow::saveGlobalSettings()
{
	QSettings *config = getSettingsObject();

	config->setValue("Main ToolBar Items", mainToolBarItems.join("|"));

	config->setValue("Max Poets Per Group", SaagharWidget::maxPoetsPerGroup);

	config->setValue("Background State", SaagharWidget::backgroundImageState);
	config->setValue("Background Path", SaagharWidget::backgroundImagePath);
	config->setValue("Icon Theme State", settingsIconThemeState);
	config->setValue("Icon Theme Path", settingsIconThemePath);

	//font
	config->setValue("Font Family", SaagharWidget::tableFont.family());
	config->setValue( "Font Size", SaagharWidget::tableFont.pointSize());

	config->setValue("Show Beyt Numbers", SaagharWidget::showBeytNumbers);
	config->setValue("Text Color", SaagharWidget::textColor);
	config->setValue("Matched Text Color", SaagharWidget::matchedTextColor);
	config->setValue("Background Color", SaagharWidget::backgroundColor);
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
		 QDesktopServices::openUrl(saagharWidget->currentPageGanjoorUrl());
	}
}

void MainWindow::emitReSizeEvent()
{
	//maybe a Qt BUG
	//before 'QMainWindow::show()' the computation of width of QMainWindow is not correct!
	resizeEvent(0);
}

void MainWindow::resizeEvent( QResizeEvent * event )
{
	saagharWidget->resizeTable(saagharWidget->tableViewWidget);
	if (event)
		QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent( QCloseEvent * event )
{
	QFontDatabase::removeAllApplicationFonts();
	saveSaagharState();
	event->accept();
}

void MainWindow::saveSaagharState()
{
	QSettings *config = getSettingsObject();

	config->setValue("MainWindowState", saveState(1));
	config->setValue("Mainwindow Geometry", saveGeometry());

	QString openedTabs = "";
	for (int i = 0; i < mainTabWidget->count(); ++i)
	{
		SaagharWidget *tmp = getSaagharWidget(i);
		QString tabViewType;
		if (tmp->currentPoem > 0)
			tabViewType = "PoemID="+QString::number(tmp->currentPoem);
		else
			tabViewType = "CatID="+QString::number(tmp->currentCat);
		
		openedTabs += tabViewType+"|";
	}
	config->setValue("openedTabs", openedTabs);

	//database path
	config->setValue("DataBase Path", QGanjoorDbBrowser::dataBasePath.join(";"));

	//search options
	config->setValue("New Search", SaagharWidget::newSearchFlag);
	config->setValue("New Search non-Alphabet", SaagharWidget::newSearchSkipNonAlphabet);
}

//Search
void MainWindow::showSearchResults(QString phrase, int PageStart, int count, int PoetID, QWidget *dockWidget)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();

	QList<int> poemIDsList;
	//new search method
	QProgressDialog progress(tr("Initializing Results Table..."), tr("Cancel"), 0, 100, this);
	progress.setWindowModality(Qt::WindowModal);
	if (SaagharWidget::newSearchFlag)
	{
		phrase = QGanjoorDbBrowser::cleanString(phrase, SaagharWidget::newSearchSkipNonAlphabet);
		poemIDsList = SaagharWidget::ganjoorDataBase->getPoemIDsContainingPhrase_NewMethod(phrase, PoetID, SaagharWidget::newSearchSkipNonAlphabet);
		progress.setMaximum(poemIDsList.size());
	}
	else
		poemIDsList = SaagharWidget::ganjoorDataBase->getPoemIDsContainingPhrase(phrase, PageStart, count+1, PoetID);
	int tmpCount = count;
	bool HasMore = count>0 && poemIDsList.size() == count+1;
	bool navButtonsNeeded = false;
	if (poemIDsList.size() <= count)
	{
		count = poemIDsList.size();
		if (PageStart>0)
			navButtonsNeeded = true;//almost one of navigation button needs to be enabled
	}
	else
	{
		count = HasMore ? count : poemIDsList.size() - 1;
		navButtonsNeeded = true;//almost one of navigation button needs to be enabled
	}
	
	//new search method show all results in one page
	if (SaagharWidget::newSearchFlag)
		count = poemIDsList.size();

	if (count < 1)
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::information(this, tr("Search"), tr("No match found."));
		return;
	}
	QTableWidget *searchTable;
	QToolButton *searchNextPage, *searchPreviousPage;
	QAction *actSearchNextPage, *actSearchPreviousPage;
	QDockWidget *searchResultWidget = 0;
	QWidget *searchResultContents;
	bool fromOtherPage = false;

	if (dockWidget)
	{
		fromOtherPage = true;
		qDeleteAll(dockWidget->children());
	}

	if (!fromOtherPage)
	{
		QString poet = "";
		if (PoetID == 0)
			poet = tr("All");
		else
		{
			poet = SaagharWidget::ganjoorDataBase->getPoet(PoetID)._Name;
		}
		searchResultWidget = new QDockWidget(poet+":"+phrase, this);
		searchResultWidget->setObjectName(QString::fromUtf8("searchResultWidget_new"));//object name for created instance, it renames to 'searchResultWidget_old'
		searchResultWidget->setLayoutDirection(Qt::RightToLeft);
		searchResultWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
		searchResultWidget->setAttribute(Qt::WA_DeleteOnClose, true);
		searchResultContents = new QWidget();
		searchResultContents->setObjectName(QString::fromUtf8("searchResultContents"));
		searchResultContents->setAttribute(Qt::WA_DeleteOnClose, true);
	}
	else
	{
		searchResultContents = dockWidget;
	}

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
	//create QTableWidget
	searchTable = new QTableWidget(searchResultContents);
	searchTable->setObjectName(QString::fromUtf8("searchTable"));
	searchTable->setAlternatingRowColors(true);
	searchTable->setSelectionMode(QAbstractItemView::NoSelection);
	searchTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	searchTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	searchTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	searchTable->setRowCount(count);
	searchTable->setColumnCount(3);
	searchTable->verticalHeader()->setVisible(false);
	searchTable->horizontalHeader()->setVisible(false);
	searchTable->horizontalHeader()->setHighlightSections(false);
	searchTable->horizontalHeader()->setStretchLastSection(true);

	//create connections for mouse signals
	connect(searchTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClick(QTableWidgetItem *)));
	connect(searchTable, SIGNAL(itemPressed(QTableWidgetItem *)), this, SLOT(tableItemPress(QTableWidgetItem *)));

	//install delagate on third column
	searchTable->setItemDelegateForColumn(2, new SaagharItemDelegate(searchTable, searchTable->style(), phrase));

	searchTableGridLayout->addWidget(searchTable, 0, 0, 1, 1);

	if (!SaagharWidget::newSearchFlag)
	{//by the new search method there's just one page result!
		QVBoxLayout *searchNavVerticalLayout = new QVBoxLayout();
		searchNavVerticalLayout->setSpacing(6);
		searchNavVerticalLayout->setObjectName(QString::fromUtf8("searchNavVerticalLayout"));
		searchNextPage = new QToolButton(searchResultContents);
		searchNextPage->setObjectName(QString::fromUtf8("searchNextPage"));

		actSearchNextPage = new QAction(searchResultContents);
		actSearchNextPage->setIcon(actionInstance("actionNextPoem")->icon());
		searchNextPage->setDefaultAction(actSearchNextPage);

		connect(searchNextPage, SIGNAL(triggered(QAction *)), this, SLOT(searchPageNavigationClicked(QAction *)));

		searchNextPage->setEnabled(false);
		searchNextPage->hide();
		
		searchNavVerticalLayout->addWidget(searchNextPage);

		searchPreviousPage = new QToolButton(searchResultContents);
		searchPreviousPage->setObjectName(QString::fromUtf8("searchPreviousPage"));
		
		actSearchPreviousPage = new QAction(searchResultContents);
		actSearchPreviousPage->setIcon(actionInstance("actionPreviousPoem")->icon());
		searchPreviousPage->setDefaultAction(actSearchPreviousPage);

		connect(searchPreviousPage, SIGNAL(triggered(QAction *)), this, SLOT(searchPageNavigationClicked(QAction *)));

		searchPreviousPage->setEnabled(false);
		searchPreviousPage->hide();

		searchNavVerticalLayout->addWidget(searchPreviousPage);
		
		if (navButtonsNeeded)
		{//almost one of navigation button needs to be enabled
			searchNextPage->show();
			searchPreviousPage->show();
		}

		QSpacerItem *searchNavVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

		searchNavVerticalLayout->addItem(searchNavVerticalSpacer);

		horizontalLayout->addLayout(searchNavVerticalLayout);
	} //if (!SaagharWidget::newSearchFlag)

	horizontalLayout->addLayout(searchTableGridLayout);

	searchGridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

	if (!fromOtherPage)
	{
		QDockWidget *tmpDockWidget = 0;
		QObjectList mainWindowChildren = this->children();
		for (int i=0; i < mainWindowChildren.size(); ++i)
		{
			tmpDockWidget = qobject_cast<QDockWidget *>(mainWindowChildren.at(i));
			if (tmpDockWidget)
			{
				if (mainWindowChildren.at(i)->objectName() == "searchResultWidget_old")
				break;
			}
		}

		searchResultWidget->setWidget(searchResultContents);
		this->addDockWidget(Qt::LeftDockWidgetArea, searchResultWidget);
		
		if (tmpDockWidget && tmpDockWidget->objectName() == "searchResultWidget_old") //there is another search results dock-widget present
			tabifyDockWidget(tmpDockWidget, searchResultWidget);

		searchResultWidget->setObjectName("searchResultWidget_old");

		searchResultWidget->show();
		searchResultWidget->raise();
	}

	searchTable->setColumnWidth(0, searchTable->fontMetrics().boundingRect(QString::number((PageStart+count)*100)).width());
	int maxPoemWidth = 0, maxVerseWidth = 0;

	for (int i = 0; i < count; ++i)
	{
		progress.setValue(i);
		 if (progress.wasCanceled())
			break;

		GanjoorPoem poem = SaagharWidget::ganjoorDataBase->getPoem(poemIDsList.at(i));

		poem._HighlightText = phrase;
		
		//we need a modified verion of showParentCategory
		//showParentCategory(SaagharWidget::ganjoorDataBase->getCategory(poem._CatID));
		//adding Numbers
		QString localizedNumber = SaagharWidget::persianIranLocal.toString(PageStart+i+1);
		QTableWidgetItem *numItem = new QTableWidgetItem(localizedNumber);
		numItem->setFlags(Qt::NoItemFlags /*Qt::ItemIsEnabled*/);

		searchTable->setItem(i, 0, numItem);

		//add Items
		QTableWidgetItem *poemItem = new QTableWidgetItem(poem._Title);
		poemItem->setFlags(Qt::ItemIsEnabled);
		poemItem->setData(Qt::UserRole, "PoemID="+QString::number(poem._ID));

		int tmpWidth = searchTable->fontMetrics().boundingRect(poem._Title).width();
		if ( tmpWidth > maxPoemWidth )
			maxPoemWidth = tmpWidth;

		QString firstVerse = "";
		//new search
		if (SaagharWidget::newSearchFlag)
		{
			QStringList verseTexts = SaagharWidget::ganjoorDataBase->getVerseListContainingPhrase(poem._ID, phrase.at(0));
			for (int k=0; k<verseTexts.size();++k)
			{
		
				QString foundedVerse = QGanjoorDbBrowser::cleanString(verseTexts.at(k), SaagharWidget::newSearchSkipNonAlphabet);	


				if (foundedVerse.contains(phrase, Qt::CaseInsensitive))
				{
					firstVerse = verseTexts.at(k);
					break;//we need just first result
				}
			}
		}
		else
			firstVerse = SaagharWidget::ganjoorDataBase->getFirstVerseContainingPhrase(poem._ID, phrase);

		QTableWidgetItem *verseItem = new QTableWidgetItem(firstVerse);
		verseItem->setFlags(Qt::ItemIsEnabled/*Qt::NoItemFlags*/);
		verseItem->setData(Qt::UserRole, "PoemID="+QString::number(poem._ID));
		//set search data
		verseItem->setData(ITEM_SEARCH_DATA, phrase);
		poemItem->setData(ITEM_SEARCH_DATA, phrase);

		//insert items to table
		searchTable->setItem(i, 1, poemItem);
		searchTable->setItem(i, 2, verseItem);

		tmpWidth = searchTable->fontMetrics().boundingRect(firstVerse).width();
		if ( tmpWidth > maxVerseWidth )
			maxVerseWidth = tmpWidth;
	}

	searchTable->setColumnWidth(1, maxPoemWidth+searchTable->fontMetrics().boundingRect("00").width() );
	searchTable->setColumnWidth(2, maxVerseWidth+searchTable->fontMetrics().boundingRect("00").width() );

	if (!SaagharWidget::newSearchFlag)
	{//by the new search method there's just one page result!
		if (PageStart > 0)
		{
			searchPreviousPage->setEnabled(true);
			actSearchPreviousPage->setData("actSearchPreviousPage|"+phrase+"|"+QString::number(PageStart - tmpCount)+"|"+QString::number(tmpCount)+"|"+QString::number(PoetID));
		}

		if (HasMore)
		{
			searchNextPage->setEnabled(true);
			actSearchNextPage->setData("actSearchNextPage|"+phrase+"|"+QString::number(PageStart + count)+"|"+QString::number(count)+"|"+QString::number(PoetID));
		}
	}

	QApplication::restoreOverrideCursor();
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
}

void MainWindow::tableItemPress(QTableWidgetItem *)
{
	pressedMouseButton = QApplication::mouseButtons();
}

void MainWindow::searchPageNavigationClicked(QAction *action)
{
	QWidget *actParent = action->parentWidget();

	QVariant actionData = action->data();
	if ( !actionData.isValid() || actionData.isNull() ) return;
	QStringList dataList = actionData.toString().split("|", QString::SkipEmptyParts);
	if (actParent)
		showSearchResults(dataList.at(1), dataList.at(2).toInt(), dataList.at(3).toInt(), dataList.at(4).toInt(), actParent);
	else	
		showSearchResults(dataList.at(1), dataList.at(2).toInt(), dataList.at(3).toInt(), dataList.at(4).toInt(), 0);
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
	QVariant searchData = item->data(ITEM_SEARCH_DATA);
	QStringList searchDataList = QStringList();
	bool setupSaagharItemDelegate = false;
	if (searchData.isValid() && !searchData.isNull())
	{
		searchDataList = searchData.toString().split("|", QString::SkipEmptyParts);
		if (!searchDataList.at(0).isEmpty())
			setupSaagharItemDelegate = true;
	}

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
		if (setupSaagharItemDelegate)
		{
			saagharWidget->scrollToFirstItemContains(searchDataList.at(0));
			saagharWidget->tableViewWidget->setItemDelegate(new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), searchDataList.at(0)));
		}
		connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClick(QTableWidgetItem *)));
		return;
	}

	saagharWidget->processClickedItem(itemData.at(0), idData, noError);
	if (setupSaagharItemDelegate)
	{
		saagharWidget->scrollToFirstItemContains(searchDataList.at(0));
		saagharWidget->tableViewWidget->setItemDelegate(new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), searchDataList.at(0)));
	}

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
		searchRegionsInitialize();
		//update visible region of searchToolBar
		bool tmpFlag = labelMaxResultAction->isVisible();
		labelMaxResultAction->setVisible(!tmpFlag);
		ui->searchToolBar->update();
		QApplication::processEvents();
		labelMaxResultAction->setVisible(tmpFlag);
	}
    else
	{
		SaagharWidget::ganjoorDataBase->dBConnection.rollback();
		QMessageBox::warning(this, tr("Error!"), tr("There are some errors, the import procedure was not completed"));
	}

	QApplication::restoreOverrideCursor();
}
