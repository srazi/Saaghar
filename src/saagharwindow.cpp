/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2015 by S. Razi Alavizadeh                          *
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

#include "saagharwindow.h"

#include "ui_saagharwindow.h"
#include "searchitemdelegate.h"
#include "version.h"
#include "searchresultwidget.h"
#include "searchpatternmanager.h"
#include "outline.h"
#include "searchoptionsdialog.h"
#include "mactoolbutton.h"
#include "databasebrowser.h"
#include "settings.h"
#include "tools.h"
#include "concurrenttasks.h"
#include "saagharapplication.h"
#include "settingsmanager.h"
#include "outlinemodel.h"

#include <QTextBrowserDialog>
#include <QSearchLineEdit>
#include <QMultiSelectWidget>
#include <QExtendedSplashScreen>

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
#include <QActionGroup>


#include "qirbreadcrumbbar.h"
#include "qirbreadcrumbbarstyle.h"
#include "breadcrumbsaagharmodel.h"

#ifdef MEDIA_PLAYER
#include "qmusicplayer.h"
#endif

SaagharWindow::SaagharWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::SaagharWindow)
    , menuBookmarks(0)
    , m_cornerMenu(0)
    , m_settingsDialog(0)
    , m_updateTaskScheduled(false)
{
    setObjectName("SaagharMainWindow");
    setWindowIcon(QIcon(":/resources/images/saaghar.png"));

    bool fresh = QCoreApplication::arguments().contains("-fresh", Qt::CaseInsensitive);

#ifdef MEDIA_PLAYER
    SaagharWidget::musicPlayer = new QMusicPlayer(this);
    SaagharWidget::musicPlayer->setWindowTitle(tr("Audio Player"));
    addDockWidget(Qt::RightDockWidgetArea, SaagharWidget::musicPlayer->albumManagerDock());
    SaagharWidget::musicPlayer->albumManagerDock()->hide();
    SaagharWidget::musicPlayer->hide();

    SaagharWidget::musicPlayer->readPlayerSettings();

    if (QMusicPlayer::albumsPathList.isEmpty()) {
        const QString &defaultFile = sApp->defaultPath(SaagharApplication::AlbumFile);
        if (!QFile::exists(defaultFile)) {
            SaagharWidget::musicPlayer->newAlbum(defaultFile, "default");
        }
        else {
            QMusicPlayer::albumsPathList.insert("default", defaultFile);
        }
    }
    SaagharWidget::musicPlayer->loadAllAlbums();

    connect(SaagharWidget::musicPlayer, SIGNAL(showTextRequested(int,int)), this, SLOT(highlightTextOnPoem(int,int)));
    connect(this, SIGNAL(highlightedTextChanged(QString)), SaagharWidget::musicPlayer, SIGNAL(highlightedTextChange(QString)));
    connect(this, SIGNAL(verseSelected(int)), SaagharWidget::musicPlayer, SIGNAL(verseSelectedByUser(int)));
#endif

    //loading application fonts
    //QString applicationFontsPath = resourcesPath + "/fonts/";
    QDir fontsDir(sApp->defaultPath(SaagharApplication::ResourcesDir) + "/fonts/");
    QStringList fontsList = fontsDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (int i = 0; i < fontsList.size(); ++i) {
        QFontDatabase::addApplicationFont(sApp->defaultPath(SaagharApplication::ResourcesDir)
                                          + "/fonts/" + fontsList.at(i));
    }

    if (splashScreen(QExtendedSplashScreen) && !fresh) {
        if (VARB("General/DisplaySplashScreen")) {
            splashScreen(QExtendedSplashScreen)->show();
            showStatusText(QObject::tr("<i><b>Loading...</b></i>"));
        }
        else {
            splashScreen(QExtendedSplashScreen)->finish(this);
        }
    }

#ifdef D_MSVC_CC
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1256"));
#endif
#ifdef D_MINGW_CC
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
#endif

    SaagharWidget::persianIranLocal = QLocale(QLocale::Persian, QLocale::Iran);
    SaagharWidget::persianIranLocal.setNumberOptions(QLocale::OmitGroupSeparator);

    defaultPrinter = 0;

    //Undo FrameWork
    undoGroup = new QUndoGroup(this);

    saagharWidget = 0;
    pressedMouseButton = Qt::LeftButton;

    skipContextMenu = false;

    SaagharWidget::poetsImagesDir = sApp->defaultPath(SaagharApplication::ResourcesDir) + "/poets_images/";

    ui->setupUi(this);

    const QString toolbarCSS = QLatin1String("QToolBar { background-image:url(\":/resources/images/transp.png\"); border:none; padding: 3px; spacing: 0px; /* spacing between items in the tool bar */ }");
    //create Parent Categories ToolBar
    parentCatsToolBar = new QToolBar(this);
    parentCatsToolBar->setObjectName(QString::fromUtf8("ClassicBreadCrumbToolBar"));
    parentCatsToolBar->setLayoutDirection(Qt::RightToLeft);
    parentCatsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    parentCatsToolBar->setWindowTitle(tr("Classic navigation bar"));
    parentCatsToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea | Qt::NoToolBarArea);
    parentCatsToolBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    parentCatsToolBar->setStyleSheet(toolbarCSS);

    // create Bread Crumb ToolBar
    m_breadCrumbBar = new QIrBreadCrumbBar(this);
    m_breadCrumbBar->setSubStyle(new QIrStyledBreadCrumbBarStyle);
    m_breadCrumbBar->setModel(new BreadCrumbSaagharModel);
    m_breadCrumbBar->setMinimumWidth(300);
    m_breadCrumbBar->setSizePolicy(QSizePolicy::Expanding, m_breadCrumbBar->sizePolicy().verticalPolicy());
    connect(m_breadCrumbBar, SIGNAL(locationChanged(QString)), this, SLOT(openPath(QString)));

    m_breadCrumbToolBar = new QToolBar(this);
    m_breadCrumbToolBar->setObjectName(QString::fromUtf8("ModernBreadCrumbToolBar"));
    m_breadCrumbToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_breadCrumbToolBar->setWindowTitle(tr("Modern navigation bar"));
    m_breadCrumbToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea | Qt::NoToolBarArea);
    m_breadCrumbToolBar->setStyleSheet(toolbarCSS);
    m_breadCrumbToolBar->addWidget(m_breadCrumbBar);

    //create Tab Widget
    mainTabWidget = ui->tabWidget;

    setupUi();

    //setup corner widget
    mainTabWidget->getTabBar()->addTabButton()->setIcon(QIcon(ICON_PATH + "/add-tab.png"));
    connect(mainTabWidget->getTabBar()->addTabButton(), SIGNAL(clicked()), this, SLOT(actionNewTabClicked()));

    MacToolButton* cornerMenuButton = new MacToolButton(mainTabWidget);
    cornerMenuButton->setStyleSheet("QToolButton::menu-indicator{image: none;}");
    cornerMenuButton->setAutoRaise(true);
    cornerMenuButton->setIcon(QIcon(ICON_PATH + "/arrow-down.png"));
    cornerMenuButton->setFixedWidth(cornerMenuButton->iconSize().width());
    cornerMenuButton->setMenu(cornerMenu());

    QWidget* cornerWidget = new QWidget(mainTabWidget);
    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(cornerMenuButton);
    cornerWidget->setLayout(hLayout);
    connect(cornerMenuButton, SIGNAL(clicked()), cornerMenuButton, SLOT(showMenu()));
    mainTabWidget->setCornerWidget(cornerWidget);

    loadTabWidgetSettings();

    outlineTree->refreshTree();
    connect(outlineTree, SIGNAL(openParentRequested(int)), this, SLOT(openParentPage(int)));
    connect(outlineTree, SIGNAL(newParentRequested(int)), this, SLOT(newTabForItem(int)));
    connect(outlineTree, SIGNAL(openRandomRequested(int,bool)), this, SLOT(openRandomPoem(int,bool)));

    if (!fresh) {
        QStringList openedTabs = VAR("SaagharWindow/LastSessionTabs").toStringList();
        showStatusText(tr("<i><b>Saaghar is starting...</b></i>"), openedTabs.size());
        for (int i = 0; i < openedTabs.size(); ++i) {
            QStringList tabViewData = openedTabs.at(i).split("=", QString::SkipEmptyParts);
            if (tabViewData.size() == 2 && (tabViewData.at(0) == "PoemID" || tabViewData.at(0) == "CatID")) {
                bool Ok = false;
                int id = tabViewData.at(1).toInt(&Ok);
                if (Ok) {
                    newTabForItem(id, tabViewData.at(0), true, false);
                }
            }
        }
    }

    if (mainTabWidget->count() < 1) {
        insertNewTab();
    }

    connect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    connect(mainTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloser(int)));

    connect(ui->mainToolBar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(toolBarContextMenu(QPoint)));
    //connect(ui->searchToolBar, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(searchToolBarView()));
    ui->searchToolBar->installEventFilter(this);

    createConnections();
    previousTabIndex = mainTabWidget->currentIndex();
    currentTabChanged(mainTabWidget->currentIndex());

    multiSelectInsertItems(selectSearchRange);

    //seeding random function
    uint numOfSecs = QDateTime::currentDateTime().toTime_t();
    uint seed = QCursor::pos().x() + QCursor::pos().y() + numOfSecs + QDateTime::currentDateTime().time().msec();
    qsrand(seed);

    restoreState(VAR("SaagharWindow/State0").toByteArray(), 0);
    restoreGeometry(VAR("SaagharWindow/Geometry").toByteArray());

//  if (!splitter->restoreState(VAR("SplitterState").toByteArray()))
//  {
//      int bigOne = (width()*3)/4;
//      splitter->setSizes( QList<int>() <<  width()-bigOne << bigOne);
//  }


    if (windowState() & Qt::WindowFullScreen) {
        actionInstance("actionFullScreen")->setChecked(true);
        actionInstance("actionFullScreen")->setText(tr("Exit &Full Screen"));
        actionInstance("actionFullScreen")->setIcon(QIcon(ICON_PATH + "/no-fullscreen.png"));
    }
    else {
        actionInstance("actionFullScreen")->setChecked(false);
        actionInstance("actionFullScreen")->setText(tr("&Full Screen"));
        actionInstance("actionFullScreen")->setIcon(QIcon(ICON_PATH + "/fullscreen.png"));
        //apply transparent effect just in windowed mode!
        QtWin::easyBlurUnBlur(this, VARB("SaagharWindow/UseTransparecy"));
    }

    connect(sApp->databaseBrowser(), SIGNAL(databaseUpdated()), this, SLOT(onDatabaseUpdate()));

    showStatusText(tr("<i><b>Saaghar is starting...</b></i>"), -1);


    if (VARB("General/AutoCheckUpdates") && !fresh) {
        QTimer::singleShot(10000, this, SLOT(checkForUpdates()));
    }
}

SaagharWindow::~SaagharWindow()
{
    delete ui;
}

void SaagharWindow::searchStart()
{
    bool byPressEnter = sender() == SaagharWidget::lineEditSearchText;
    if (byPressEnter) {
        SaagharWidget::lineEditSearchText->resetNotFound();
        SaagharWidget::lineEditSearchText->searchStop();
    }

    if (!SaagharWidget::lineEditSearchText->searchWasCanceled() && !byPressEnter) {
        return;
    }

    QString phrase = SaagharWidget::lineEditSearchText->text();
    phrase.remove(" ");
    phrase.remove("\t");
    if (phrase.isEmpty()) {
        SaagharWidget::lineEditSearchText->setText("");
        QMessageBox::information(this, tr("Error"), tr("The search phrase can not be empty."));
        return;
    }
    phrase = SaagharWidget::lineEditSearchText->text();
    phrase.replace(QChar(0x200C), "", Qt::CaseInsensitive);//replace ZWNJ by ""

    //QString currentItemData = comboBoxSearchRegion->itemData(comboBoxSearchRegion->currentIndex(), Qt::UserRole).toString();

    QList<QListWidgetItem*> selectList = selectSearchRange->getSelectedItemList();

    bool searchCanceled = false;
    bool slowSearch = false;//more calls of 'processEvents'
    if (selectList.size() > 1) {
        slowSearch = true;
    }

    ConcurrentTaskManager::instance()->switchToStartState();

    for (int i = 0; i < selectList.size(); ++i) {
        if (ConcurrentTaskManager::instance()->isAllTaskCanceled()) {
            break;
        }

        QVariant currentItemData = selectList.at(i)->data(Qt::UserRole);
        //QString currentItemStrData = currentItemData.toString();

        if (currentItemData.toString() == "ALL_OPENED_TAB") {
            for (int j = 0; j < mainTabWidget->count(); ++j) {
                SaagharWidget* tmp = getSaagharWidget(j);
                //QAbstractItemDelegate *tmpDelegate = tmp->tableViewWidget->itemDelegate();

                //delete tmpDelegate;
                //tmpDelegate = 0;
                if (tmp) {
                    tmp->scrollToFirstItemContains(phrase);
                }
                //tmp->tableViewWidget->setItemDelegate(new SaagharItemDelegate(tmp->tableViewWidget, saagharWidget->tableViewWidget->style(), SaagharWidget::lineEditSearchText->text()));
            }
        }
        //else if (currentItemData == "CURRENT_TAB")
        //{
        //  if (saagharWidget)
        //  {
        //      //connect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), saagharWidget, SLOT(scrollToFirstItemContains(QString)) );
        //      //QAbstractItemDelegate *currentDelegate = saagharWidget->tableViewWidget->itemDelegate();
        //      saagharWidget->scrollToFirstItemContains(SaagharWidget::lineEditSearchText->text());
        //      //SaagharItemDelegate *tmp = new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), SaagharWidget::lineEditSearchText->text());
        //      //saagharWidget->tableViewWidget->setItemDelegate(tmp);
        //      //connect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), tmp, SLOT(keywordChanged(QString)) );
        //      //delete currentDelegate;
        //      //currentDelegate = 0;
        //  }
        //}
        else {
            //phrase = Tools::cleanString(SaagharWidget::lineEditSearchText->text(), SaagharWidget::newSearchSkipNonAlphabet);
            //int poetID = comboBoxSearchRegion->itemData(comboBoxSearchRegion->currentIndex(), Qt::UserRole).toInt();
            SearchPatternManager::instance()->setInputPhrase(phrase);
            SearchPatternManager::instance()->init();
            QVector<QStringList> phraseVectorList = SearchPatternManager::instance()->outputPhrases();
            QVector<QStringList> excludedVectorList = SearchPatternManager::instance()->outputExcludedLlist();
            int vectorSize = phraseVectorList.size();

            int poetID;
            if (currentItemData.toString() == "ALL_TITLES") {
                poetID = -1000;    //reserved for titles!!
            }
            else {
                bool ok = false;
                poetID = currentItemData.toInt(&ok);
                if (!ok) {
                    if (i == selectList.size() - 1) {
                        SaagharWidget::lineEditSearchText->searchStop();
                    }
                    continue;//itemData is not int skip to next item
                }
            }

            QString poetName = "";
            if (poetID == 0) {
                poetName = tr("All");
            }
            else {
                //because of 'ok == true' we know this is  poet name
                poetName = selectList.at(i)->text();
            }

            QWidget* searchResultContents = new QWidget(this);
            searchResultContents->setObjectName(QString::fromUtf8("searchResultContents"));
            searchResultContents->setAttribute(Qt::WA_DeleteOnClose, true);
            //searchResultContents->setLayoutDirection(Qt::RightToLeft);

            phrase = Tools::cleanString(phrase);
            SearchResultWidget* searchResultWidget = new SearchResultWidget(this, ICON_PATH, searchResultContents,
                                                                            phrase, poetName);

            ConcurrentTask* searchTask = new ConcurrentTask(searchResultWidget);
            connect(searchTask, SIGNAL(concurrentResultReady(QString,QVariant)), searchResultWidget, SLOT(onConcurrentResultReady(QString,QVariant)));
            connect(searchTask, SIGNAL(searchStatusChanged(QString)), SaagharWidget::lineEditSearchText, SLOT(setSearchProgressText(QString)));

            /////////////////////////////////////
//          QProgressDialog searchProgress(tr("Searching Data Base..."),  tr("Cancel"), 0, 0, this);
//          connect( &searchProgress, SIGNAL(canceled()), &searchProgress, SLOT(hide()) );
//          searchProgress.setWindowModality(Qt::WindowModal);
//          searchProgress.setFixedSize(searchProgress.size());
//          searchProgress.setMinimumDuration(0);
//          //searchProgress.setValue(0);
//          //searchProgress.show();

            SaagharWidget::lineEditSearchText->searchStart(&searchCanceled);

            SaagharWidget::lineEditSearchText->setSearchProgressText(tr("Searching Data Base(subset= %1)...").arg(poetName));
            QApplication::processEvents();

            bool success = false;

            for (int j = 0; j < vectorSize; ++j) {
                QStringList phrases = phraseVectorList.at(j);
                QStringList excluded = excludedVectorList.at(j);
#ifdef SAAGHAR_DEBUG
                int start = QDateTime::currentDateTime().toTime_t() * 1000 + QDateTime::currentDateTime().time().msec();
#endif

                success |= sApp->databaseBrowser()->getPoemIDsByPhrase(searchTask, poetID, phrases, excluded, &searchCanceled, slowSearch);

#ifdef SAAGHAR_DEBUG
                int end = QDateTime::currentDateTime().toTime_t() * 1000 + QDateTime::currentDateTime().time().msec();
                int miliSec = end - start;
                qDebug() << "\n------------------------------------------------\n"
                         << phrases << "\tsearch-duration=" << miliSec
                         << "\n------------------------------------------------\n";
#endif

                if (searchCanceled) {
                    break;
                }
            }

            if (i == selectList.size() - 1) {
                SaagharWidget::lineEditSearchText->searchStop();
            }

            // searchResultWidget->setResultList(finalResult);
            if (!success) {
                if (i == selectList.size() - 1) {
                    SaagharWidget::lineEditSearchText->notFound();
                    connect(selectSearchRange, SIGNAL(itemCheckStateChanged(QListWidgetItem*)), SaagharWidget::lineEditSearchText, SLOT(resetNotFound()));
                    SaagharWidget::lineEditSearchText->setSearchProgressText(tr("Current Scope: %1\nNo match found.").arg(selectSearchRange->getSelectedStringList().join("-")));
                }
                delete searchResultWidget;
                searchResultWidget = 0;

                if (searchCanceled) {
                    break;    //search is canceled
                }
                else {
                    continue;
                }
            }

            connect(this, SIGNAL(maxItemPerPageChanged()), searchResultWidget, SLOT(maxItemPerPageChange()));

            //create connections for mouse signals
            connect(searchResultWidget->searchTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(tableItemClick(QTableWidgetItem*)));
            connect(searchResultWidget->searchTable, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(tableItemPress(QTableWidgetItem*)));
        }

        //we should create conection first and then break!
        if (searchCanceled) {
            break;    //search is canceled
        }
    }
}

void SaagharWindow::multiSelectObjectInitialize(QMultiSelectWidget* multiSelectWidget, const QStringList &selectedData, int insertIndex)
{
    QListWidgetItem* rootItem = multiSelectWidget->insertRow(insertIndex, tr("All"), true, "0", Qt::UserRole);
    QList<GanjoorPoet*> poets = sApp->databaseBrowser()->getPoets();

    for (int i = 0; i < poets.size(); ++i) {
        multiSelectWidget->insertRow(i + 1 + insertIndex, poets.at(i)->_Name, true,
                                     QString::number(poets.at(i)->_ID), Qt::UserRole, false, rootItem)->setCheckState(selectedData.contains(QString::number(poets.at(i)->_ID)) ? Qt::Checked : Qt::Unchecked);
    }

    if (selectedData.contains("0")) {
        rootItem->setCheckState(Qt::Checked);
    }

    multiSelectWidget->updateSelectedLists();
}

void SaagharWindow::multiSelectInsertItems(QMultiSelectWidget *multiSelectWidget)
{
    QListWidgetItem* item = multiSelectWidget->insertRow(0, tr("All Opened Tab"), true, "ALL_OPENED_TAB", Qt::UserRole, true);
    QListWidgetItem* titleSearchItem = multiSelectWidget->insertRow(1, tr("Titles"), true, "ALL_TITLES", Qt::UserRole);

    const QStringList selectedSearchRange = VAR("Search/SelectedRange").toStringList();

    multiSelectObjectInitialize(multiSelectWidget, selectedSearchRange, 2);
    item->setCheckState(selectedSearchRange.contains("ALL_OPENED_TAB") ? Qt::Checked : Qt::Unchecked);
    titleSearchItem->setCheckState(selectedSearchRange.contains("ALL_TITLES") ? Qt::Checked : Qt::Unchecked);
}

void SaagharWindow::actionRemovePoet()
{
    bool ok = false;
    QStringList items;
    items << tr("Select a name...");
    QList<GanjoorPoet*> poets = sApp->databaseBrowser()->getPoets();
    for (int i = 0; i < poets.size(); ++i) {
        items << poets.at(i)->_Name + "(" + tr("poet's code=") + QString::number(poets.at(i)->_ID) + ")";
    }
    QString item = QInputDialog::getItem(this, tr("Remove Poet"), tr("Select a poet name and click on 'OK' button, for remove it from database."), items, 0, false, &ok);
    if (ok && !item.isEmpty() && item != tr("Select a name...")) {
        int poetID = item.mid(item.indexOf("(" + tr("poet's code=")) + tr("poet's code=").size() + 1).remove(")").toInt();
        if (poetID > 0) {
            QMessageBox warnAboutDelete(this);
            warnAboutDelete.setWindowTitle(tr("Please Notice!"));
            warnAboutDelete.setIcon(QMessageBox::Warning);
            warnAboutDelete.setText(tr("Are you sure for removing \"%1\", from database?").arg(sApp->databaseBrowser()->getPoet(poetID)._Name));
            warnAboutDelete.addButton(tr("Continue"), QMessageBox::AcceptRole);
            warnAboutDelete.setStandardButtons(QMessageBox::Cancel);
            warnAboutDelete.setEscapeButton(QMessageBox::Cancel);
            warnAboutDelete.setDefaultButton(QMessageBox::Cancel);
            int ret = warnAboutDelete.exec();
            if (ret == QMessageBox::Cancel) {
                return;
            }

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            sApp->databaseBrowser()->removePoetFromDataBase(poetID);
            outlineTree->refreshTree();

            selectSearchRange->clear();
            multiSelectInsertItems(selectSearchRange);

            setHomeAsDirty();
            QApplication::restoreOverrideCursor();
            qDebug() << "end of" << Q_FUNC_INFO;
        }
    }
}

void SaagharWindow::currentTabChanged(int tabIndex)
{
    SaagharWidget* tmpSaagharWidget = getSaagharWidget(tabIndex);
    if (tmpSaagharWidget) {
        SaagharWidget* old_saagharWidget = saagharWidget;
        saagharWidget = tmpSaagharWidget;

        saagharWidget->loadSettings();

        if (saagharWidget->isDirty()) {
            saagharWidget->refresh();
        }
        saagharWidget->showParentCategory(sApp->databaseBrowser()->getCategory(saagharWidget->currentCat));//just update parentCatsToolbar
        saagharWidget->resizeTable(saagharWidget->tableViewWidget);

        connect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), saagharWidget, SLOT(scrollToFirstItemContains(QString)));

        if (previousTabIndex != tabIndex) {
            tmpSaagharWidget = getSaagharWidget(previousTabIndex);
            if (tmpSaagharWidget) {
                disconnect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), tmpSaagharWidget, SLOT(scrollToFirstItemContains(QString)));
            }
            previousTabIndex = tabIndex;
        }

        if (saagharWidget->tableViewWidget->rowCount() > 0 && saagharWidget->currentCat != 0) {
            if (saagharWidget->currentPoem == 0) {
                QTableWidgetItem* item = saagharWidget->tableViewWidget->item(0, 0);
                if (item && !item->icon().isNull() && saagharWidget->tableViewWidget->columnCount() == 1) {
                    QString text = item->text();
                    if (text.isEmpty()) {
                        QTextEdit* textEdit = qobject_cast<QTextEdit*>(saagharWidget->tableViewWidget->cellWidget(0, 0));
                        if (textEdit) {
                            text = textEdit->toPlainText();
                        }
                    }

                    int verticalScrollBarWidth = 0;
                    if (saagharWidget->tableViewWidget->verticalScrollBar()->isVisible()) {
                        verticalScrollBarWidth = saagharWidget->tableViewWidget->verticalScrollBar()->width();
                    }
                    int totalWidth = saagharWidget->tableViewWidget->columnWidth(0) - verticalScrollBarWidth - 82;

                    totalWidth = qMax(82 + verticalScrollBarWidth, totalWidth);
                    saagharWidget->resizeTable(saagharWidget->tableViewWidget);
                }
            }
            else if (saagharWidget->tableViewWidget->columnCount() > 1) {
                QTableWidgetItem* item = saagharWidget->tableViewWidget->item(0, 1);
                if (item) {
                    int verticalScrollBarWidth = 0;
                    if (saagharWidget->tableViewWidget->verticalScrollBar()->isVisible()) {
                        verticalScrollBarWidth = saagharWidget->tableViewWidget->verticalScrollBar()->width();
                    }
                    saagharWidget->resizeTable(saagharWidget->tableViewWidget);
                }
            }
        }
        else if (SaagharWidget::maxPoetsPerGroup != 0 &&
                 saagharWidget->currentCat == 0 && saagharWidget->currentPoem == 0) {
            QList<GanjoorPoet*> poets = sApp->databaseBrowser()->getPoets();
            int numOfPoets = poets.size();
            if (numOfPoets > SaagharWidget::maxPoetsPerGroup) {
                if (SaagharWidget::maxPoetsPerGroup != 1) {
                    saagharWidget->tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(QFontMetrics(SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/Titles"))), -1, -1));
                }
            }
        }

        undoGroup->setActiveStack(saagharWidget->undoStack);
        updateCaption();
        updateTabsSubMenus();

        actionInstance("actionPreviousPoem")->setEnabled(!sApp->databaseBrowser()->getPreviousPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());
        actionInstance("actionNextPoem")->setEnabled(!sApp->databaseBrowser()->getNextPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());

        loadAudioForCurrentTab(old_saagharWidget);

        parentCatsToolBar->setEnabled(true);
        m_breadCrumbToolBar->setEnabled(true);
        globalRedoAction->setEnabled(saagharWidget->undoStack->canRedo());
        globalUndoAction->setEnabled(saagharWidget->undoStack->canUndo());
        actionInstance("fixedNameRedoAction")->setEnabled(globalRedoAction->isEnabled());
        actionInstance("fixedNameUndoAction")->setEnabled(globalUndoAction->isEnabled());
        actionInstance("actionHome")->setEnabled(true);
    }
    else {
        saagharWidget = 0;
        actionInstance("actionPreviousPoem")->setEnabled(false);
        actionInstance("actionNextPoem")->setEnabled(false);
        actionInstance("actionHome")->setEnabled(false);
        parentCatsToolBar->setEnabled(false);
        m_breadCrumbToolBar->setEnabled(false);

        globalRedoAction->setEnabled(false);
        globalUndoAction->setEnabled(false);
        actionInstance("fixedNameRedoAction")->setEnabled(false);
        actionInstance("fixedNameUndoAction")->setEnabled(false);
#ifdef MEDIA_PLAYER
        if (SaagharWidget::musicPlayer) {
            SaagharWidget::musicPlayer->setEnabled(false);
            SaagharWidget::musicPlayer->stop();
        }
#endif

        updateTabsSubMenus();
    }
}

SaagharWidget* SaagharWindow::getSaagharWidget(int tabIndex)
{
    if (tabIndex == -1) {
        return 0;
    }
    QWidget* curTabContent = mainTabWidget->widget(tabIndex);
    if (!curTabContent) {
        return 0;
    }
    QObjectList tabContentList = curTabContent->children();

    for (int i = 0; i < tabContentList.size(); ++i) {
        if (qobject_cast<SaagharWidget*>(tabContentList.at(i))) {
            return qobject_cast<SaagharWidget*>(tabContentList.at(i));
        }
    }
    return 0;
}

void SaagharWindow::checkForUpdates()
{
    if (m_updateTaskScheduled) {
        return;
    }

    m_updateTaskScheduled = true;

    const bool checkByUser = qobject_cast<QAction*>(sender()) != 0;

    ConcurrentTask* updateTask = new ConcurrentTask(this);
    connect(updateTask, SIGNAL(concurrentResultReady(QString,QVariant)), this, SLOT(processUpdateData(QString,QVariant)));

    QVariantHash arguments;
    const QString taskTitle(tr("Check for update..."));

    VAR_ADD(arguments, taskTitle);
    VAR_ADD(arguments, checkByUser);

    updateTask->start("UPDATE", arguments);
}

void SaagharWindow::processUpdateData(const QString &type, const QVariant &results)
{
    m_updateTaskScheduled = false;

    if (sender()) {
        sender()->deleteLater();
    }

    if (type != "UPDATE") {
        return;
    }

    QStringList data = results.toStringList();

    if (type != "UPDATE" || data.size() != 3) {
        return;
    }

    bool error = data.takeFirst() == QLatin1String("ERROR=TRUE");
    const bool checkByUser = data.takeFirst() == QLatin1String("CHECK_BY_USER=TRUE");

    QString dataPart = data.at(0);
    dataPart.remove("DATA=");

    error = error || dataPart.isEmpty();

    if (error) {
        if (checkByUser || !VARB("General/DisplaySplashScreen")) {
            QMessageBox criticalError(QMessageBox::Critical, tr("Error"), tr("There is an error when checking for updates...\nCheck your internet connection and try again."), QMessageBox::Ok, this, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
            criticalError.exec();
        }
        else {
            showStatusText("<p style=\"color: red;\"><i><b>" + tr("Error when checking for update!") + "</b></i></p>");
        }
        return;
    }

    data = dataPart.split("|", QString::SkipEmptyParts);

    QMap<QString, QString> releaseInfo;
    for (int i = 0; i < data.size(); ++i) {
        QStringList fields = data.at(i).split("*", QString::SkipEmptyParts);

        if (fields.size() == 2) {
            releaseInfo.insert(fields.at(0), fields.at(1));
        }
    }

    if (releaseInfo.value("VERSION").isEmpty()) {
        return;
    }

    QString runningAppVersionStr = SAAGHAR_VERSION + ".0";
    runningAppVersionStr.remove('.');
    int runningAppVersion = runningAppVersionStr.toInt();
    QString serverAppVerionStr = releaseInfo.value("VERSION");
    serverAppVerionStr.remove('.');
    int serverAppVerion = serverAppVerionStr.toInt();

    if (serverAppVerion > runningAppVersion) {
        showStatusText("!QExtendedSplashScreenCommands:CLOSE");
        QMessageBox updateIsAvailable(this);
        updateIsAvailable.setWindowFlags(updateIsAvailable.windowFlags() | Qt::WindowStaysOnTopHint);
        updateIsAvailable.setTextFormat(Qt::RichText);
        updateIsAvailable.setWindowTitle(tr("New Saaghar Version Available"));
        updateIsAvailable.setIcon(QMessageBox::Information);
        QString newVersionInfo = "";
        if (!releaseInfo.value("infoLinks").isEmpty()) {
            QStringList infoLinks = releaseInfo.value("infoLinks").split('\\', QString::SkipEmptyParts);
            if (!infoLinks.isEmpty()) {
                newVersionInfo = infoLinks.join("<br />");
            }
        }
        updateIsAvailable.setText(tr("The Version <strong>%1</strong> of Saaghar is available for download.<br />%2<br />Do you want to browse download page?").arg(releaseInfo.value("VERSION")).arg(newVersionInfo));
        updateIsAvailable.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        updateIsAvailable.setEscapeButton(QMessageBox::Cancel);
        updateIsAvailable.setDefaultButton(QMessageBox::Ok);
        QString downloadLinkKey = "OTHERS";

#ifdef Q_OS_WIN
        downloadLinkKey = "WINDOWS";
#endif
#ifdef Q_OS_X11
        downloadLinkKey = "LINUX";
#endif
#ifdef Q_OS_MAC
        downloadLinkKey = "MACOSX";
#endif
        int ret = updateIsAvailable.exec();
        if (ret == QMessageBox::Ok) {
            QDesktopServices::openUrl(QUrl(releaseInfo.value(downloadLinkKey, "http://saaghar.pozh.org/downloads/")));
        }
        else {
            return;
        }
    }
    else {

        if (checkByUser || !VARB("General/DisplaySplashScreen")) {
            QMessageBox::information(this, tr("Saaghar is up to date"), tr("There is no new version available. Please check for updates later!"));
        }
        else {
            showStatusText("<i><b>" + tr("Saaghar is up to date") + "</b></i>");
        }
    }
}

void SaagharWindow::tabCloser(int tabIndex)
{
    QWidget* closedTabContent = mainTabWidget->widget(tabIndex);

    if (closedTabContent && closedTabContent->objectName().startsWith("WidgetTab")) {
        mainTabWidget->setUpdatesEnabled(false);
        mainTabWidget->removeTab(tabIndex);
        mainTabWidget->setUpdatesEnabled(true);

#if 0
        if (closedTabContent->objectName() == "WidgetTab-Registeration") {
            actionInstance("Registeration")->setData(0);
        }
#endif

        delete closedTabContent;
        closedTabContent = 0;

        if (mainTabWidget->count() == 1) { //there is just one tab present.
            mainTabWidget->setTabsClosable(false);
        }
        else if (mainTabWidget->count() < 1) {
            // because we close registeration tab programmatically
            insertNewTab();
        }

        return;
    }

    if (tabIndex == mainTabWidget->currentIndex()) {
        SaagharWidget::lastOveredItem = 0;
    }
    //if EditMode app need to ask about save changes!
    disconnect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

    QAction* tabAct = new QAction(mainTabWidget->tabText(tabIndex), menuClosedTabs);
    SaagharWidget* tmp = getSaagharWidget(tabIndex);
    tabAct->setData(tmp->identifier());
    connect(tabAct, SIGNAL(triggered()), this, SLOT(actionClosedTabsClicked()));
    menuClosedTabs->addAction(tabAct);

    mainTabWidget->setUpdatesEnabled(false);
    mainTabWidget->removeTab(tabIndex);
    mainTabWidget->setUpdatesEnabled(true);

    delete closedTabContent;

    if (mainTabWidget->count() == 1) { //there is just one tab present.
        mainTabWidget->setTabsClosable(false);
    }
    currentTabChanged(mainTabWidget->currentIndex());//maybe we don't need this line as well disconnect and connect.
    connect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
}

QWidget* SaagharWindow::insertNewTab(TabType tabType, const QString &title, int id,
                                     const QString &type, bool noError,
                                     bool pushToStack)
{
    QWidget* tabContent = new QWidget();

    if (tabType == SaagharWindow::SaagharViewerTab) {
        QGridLayout* tabGridLayout = new QGridLayout(tabContent);
        tabGridLayout->setObjectName(QString::fromUtf8("tabGridLayout"));
        tabGridLayout->setContentsMargins(0, 0, 0, 0);

        tabContent->setObjectName(QString::fromUtf8("SaagharViewerTab"));
        QTableWidget* tabTableWidget = new QTableWidget;

        //table defaults
        tabTableWidget->setObjectName(QString::fromUtf8("mainTableWidget"));
        tabTableWidget->setLayoutDirection(Qt::RightToLeft);
        tabTableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        tabTableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        tabTableWidget->setShowGrid(false);
        tabTableWidget->setGridStyle(Qt::NoPen);
        tabTableWidget->setFrameShape(QFrame::NoFrame);
        tabTableWidget->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
        tabTableWidget->horizontalHeader()->setVisible(false);
        tabTableWidget->verticalHeader()->setVisible(false);
        tabTableWidget->setMouseTracking(true);
        tabTableWidget->setAutoScroll(false);

        //install event filter on viewport
        tabTableWidget->viewport()->installEventFilter(this);
        //install 'delegate' on QTableWidget
        QAbstractItemDelegate* tmpDelegate = tabTableWidget->itemDelegate();
        delete tmpDelegate;
        tmpDelegate = 0;

        SaagharItemDelegate* searchDelegate = new SaagharItemDelegate(tabTableWidget, tabTableWidget->style(), SaagharWidget::lineEditSearchText->text());
        tabTableWidget->setItemDelegate(searchDelegate);
        connect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), searchDelegate, SLOT(keywordChanged(QString)));

        saagharWidget = new SaagharWidget(tabContent, parentCatsToolBar, tabTableWidget);
        saagharWidget->setObjectName(QString::fromUtf8("saagharWidget"));

        undoGroup->addStack(saagharWidget->undoStack);

        connect(saagharWidget, SIGNAL(loadingStatusText(QString,int)), this, SLOT(showStatusText(QString,int)));

        connect(saagharWidget, SIGNAL(captionChanged()), this, SLOT(updateCaption()));
        connect(saagharWidget, SIGNAL(currentLocationChanged(QStringList)), this, SLOT(onCurrentLocationChanged(QStringList)));
        //temp
        connect(saagharWidget, SIGNAL(captionChanged()), this, SLOT(updateTabsSubMenus()));

        connect(saagharWidget->tableViewWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(tableItemClick(QTableWidgetItem*)));
        connect(saagharWidget->tableViewWidget, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(tableItemPress(QTableWidgetItem*)));
        connect(saagharWidget->tableViewWidget, SIGNAL(itemEntered(QTableWidgetItem*)), this, SLOT(tableItemMouseOver(QTableWidgetItem*)));
        connect(saagharWidget->tableViewWidget, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)), this, SLOT(tableCurrentItemChanged(QTableWidgetItem*,QTableWidgetItem*)));

        connect(saagharWidget->tableViewWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(createCustomContextMenu(QPoint)));
        connect(saagharWidget, SIGNAL(createContextMenuRequested(QPoint)), this, SLOT(createCustomContextMenu(QPoint)));

        //\Enable/Disable navigation actions
        connect(saagharWidget, SIGNAL(navPreviousActionState(bool)),    actionInstance("actionPreviousPoem"), SLOT(setEnabled(bool)));
        connect(saagharWidget, SIGNAL(navNextActionState(bool)),    actionInstance("actionNextPoem"), SLOT(setEnabled(bool)));
        actionInstance("actionPreviousPoem")->setEnabled(!sApp->databaseBrowser()->getPreviousPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());
        actionInstance("actionNextPoem")->setEnabled(!sApp->databaseBrowser()->getNextPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());

        // Updating table on changing of selection
        connect(saagharWidget->tableViewWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectChanged()));
        if (id != -1) {
            saagharWidget->processClickedItem(type, id, noError, pushToStack);
        }
        tabGridLayout->addWidget(tabTableWidget, 0, 0, 1, 1);
        mainTabWidget->setUpdatesEnabled(false);
        mainTabWidget->setCurrentIndex(mainTabWidget->addTab(tabContent, title)); //add and gets focus to it
        mainTabWidget->setUpdatesEnabled(true);
    }
    else if (tabType == SaagharWindow::WidgetTab) {
        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setObjectName(QString::fromUtf8("WidgetTab"));

        QPalette p(scrollArea->palette());
        p.setColor(QPalette::Base, Qt::white);
        p.setColor(QPalette::Window, QColor(Qt::lightGray).lighter(131));
        p.setColor(QPalette::Text, Qt::black);
        scrollArea->setPalette(p);
        /****/
        QWidget* baseContainer = new QWidget(mainTabWidget);
        QGridLayout* baseLayout = new QGridLayout(baseContainer);
        baseLayout->addWidget(scrollArea, 0, 0, 1, 1, Qt::AlignCenter);
        baseContainer->setLayout(baseLayout);
        /****/

        QPixmap pixmap(sApp->defaultPath(SaagharApplication::ResourcesDir) + "/themes/backgrounds/saaghar-pattern_1.png");
        if (!pixmap.isNull()) {
            QPalette p(scrollArea->viewport()->palette());
            p.setBrush(QPalette::Base, QBrush(pixmap));
            scrollArea->viewport()->setPalette(p);
        }

        scrollArea->setWidget(tabContent);
        scrollArea->setAlignment(Qt::AlignCenter);
        mainTabWidget->setUpdatesEnabled(false);
        mainTabWidget->setCurrentIndex(mainTabWidget->addTab(scrollArea, title)); //add and gets focus to it
        mainTabWidget->setUpdatesEnabled(true);
    }
    else {
        return 0;
    }

    mainTabWidget->setTabsClosable(mainTabWidget->count() > 1);

    return tabContent;
}

void SaagharWindow::newTabForItem(int id, const QString &type, bool noError, bool pushToStack)
{
    insertNewTab(SaagharWindow::SaagharViewerTab, QString(), id, type, noError, pushToStack);
    showStatusText(tr("<i><b>\"%1\" was loaded!</b></i>").arg(Tools::snippedText(saagharWidget->currentCaption.mid(saagharWidget->currentCaption.lastIndexOf(":") + 1), "", 0, 6, false, Qt::ElideRight)));
}

void SaagharWindow::updateCaption()
{
    if (mainTabWidget->currentIndex() == -1 || !getSaagharWidget(mainTabWidget->currentIndex())) {
        return;
    }
    SaagharWidget* sw = getSaagharWidget(mainTabWidget->currentIndex());

    QString newTabCaption = Tools::snippedText(sw->currentCaption, "", 0, 6, true, Qt::ElideRight) + QString(QChar(0x200F));
    mainTabWidget->setTabText(mainTabWidget->currentIndex(), newTabCaption);
    mainTabWidget->setTabToolTip(mainTabWidget->currentIndex(), "<p>" + sw->currentCaption + "</p>");
    setWindowTitle(QString(QChar(0x202B)) + tr("Saaghar: ") + sw->currentCaption + QString(QChar(0x202C)));
}

void SaagharWindow::tableSelectChanged()
{
    //Bug in Qt: old selection is not repainted properly
    if (saagharWidget) {
        disconnect(saagharWidget->tableViewWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectChanged()));
        for (int row = 0; row < saagharWidget->tableViewWidget->rowCount() ; ++row) {
            QTableWidgetItem* item = saagharWidget->tableViewWidget->item(row , 1);
            if (item && item->text().isEmpty()) {
                QTextEdit* textEdit = qobject_cast<QTextEdit*>(saagharWidget->tableViewWidget->cellWidget(row, 1));
                if (textEdit) {
                    QTextCursor textCursor(textEdit->textCursor());
                    if (item->isSelected()) {
                        item->setSelected(false);
                        if (!textCursor.hasSelection()) {
                            textEdit->selectAll();
                        }
                    }
                    else if (!item->isSelected() && textCursor.selectedText() == textEdit->toPlainText()) {
                        textCursor.clearSelection();
                        textEdit->setTextCursor(textCursor);
                    }
                }
            }
        }
        saagharWidget->tableViewWidget->viewport()->update();
        connect(saagharWidget->tableViewWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectChanged()));
    }
}

void SaagharWindow::actionExportAsPDFClicked()
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

void SaagharWindow::actionExportClicked()
{
    if (!saagharWidget) {
        return;
    }
    QString exportedFileName = QFileDialog::getSaveFileName(this, tr("Export As"), QDir::homePath(), "HTML Document (*.html);;TeX - XePersian (*.tex);;Tab Separated (*.csv);;UTF-8 Text (*.txt)");
    if (exportedFileName.endsWith(".html", Qt::CaseInsensitive)) {
        QString poemAsHTML = convertToHtml(saagharWidget);
        if (!poemAsHTML.isEmpty()) {
            writeToFile(exportedFileName, poemAsHTML);
        }
        return;
    }

    if (exportedFileName.endsWith(".txt", Qt::CaseInsensitive)) {
        QString tableAsText = tableToString(saagharWidget->tableViewWidget, "          "/*ten blank spaces*/, "\n", 0, 0, saagharWidget->tableViewWidget->rowCount(), saagharWidget->tableViewWidget->columnCount());
        writeToFile(exportedFileName, tableAsText);
        return;
    }

    if (exportedFileName.endsWith(".csv", Qt::CaseInsensitive)) {
        QString tableAsText = tableToString(saagharWidget->tableViewWidget, "\t", "\n", 0, 0, saagharWidget->tableViewWidget->rowCount(), saagharWidget->tableViewWidget->columnCount());
        writeToFile(exportedFileName, tableAsText);
        return;
    }

    if (exportedFileName.endsWith(".tex", Qt::CaseInsensitive)) {
        QString poemAsTeX = convertToTeX(saagharWidget);
        if (!poemAsTeX.isEmpty()) {
            writeToFile(exportedFileName, poemAsTeX);
        }
        return;
    }
}

void SaagharWindow::writeToFile(QString fileName, QString textToWrite)
{
    QFile exportFile(fileName);
    if (!exportFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("The file could not be saved. Please check if you have write permission."));
        return;
    }
    QTextStream out(&exportFile);
    out.setCodec("UTF-8");
    out << textToWrite;
    exportFile.close();
}

void SaagharWindow::print(QPrinter* printer)
{
    if (!saagharWidget) {
        return;
    }
    QPainter painter;
    painter.begin(printer);

    const int rows = saagharWidget->tableViewWidget->rowCount();
    const int columns = saagharWidget->tableViewWidget->columnCount();
    double totalWidth = 0.0;
    double currentHeight = 10.0;

    if (columns == 4) {
        totalWidth = qMax(saagharWidget->tableViewWidget->columnWidth(3), saagharWidget->minMesraWidth) + saagharWidget->tableViewWidget->columnWidth(2) + qMax(saagharWidget->tableViewWidget->columnWidth(1), saagharWidget->minMesraWidth);
    }
    else if (columns == 1) {
        totalWidth = saagharWidget->tableViewWidget->columnWidth(0);
    }
    int pageWidth = printer->pageRect().width();
    int pageHeight = printer->pageRect().height();

    const int extendedWidth = SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/PoemText")).pointSize() * 4;
    const qreal scale = pageWidth / (totalWidth + extendedWidth); //printer->logicalDpiX() / 96;
    painter.scale(scale, scale);

    for (int row = 0; row < rows; ++row) {
        int thisRowHeight = saagharWidget->tableViewWidget->rowHeight(row);
        for (int col = 0; col < columns; ++col) {
            QTableWidgetItem* item = saagharWidget->tableViewWidget->item(row, col);
            int span = saagharWidget->tableViewWidget->columnSpan(row, col);
            QString text = "";
            QTextEdit* textEdit = 0;

            if (item) {
                QFont fnt(SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/PoemText")));
                QColor color(SaagharWidget::resolvedColor(LS("SaagharWidget/Colors/PoemText")));
                int rectX1 = 0;
                int rectX2 = 0;
                if (columns == 4) {
                    if (span == 1) {
                        if (col == 1) {
                            rectX1 = saagharWidget->tableViewWidget->columnWidth(3) + saagharWidget->tableViewWidget->columnWidth(2);
                            rectX2 = saagharWidget->tableViewWidget->columnWidth(3) + saagharWidget->tableViewWidget->columnWidth(2) + saagharWidget->tableViewWidget->columnWidth(1);
                        }
                        else if (col == 3) {
                            rectX1 = 0;
                            rectX2 = saagharWidget->tableViewWidget->columnWidth(3);
                        }
                    }
                    else {
                        //Single or Paragraph
                        rectX1 = 0;
                        rectX2 = saagharWidget->tableViewWidget->columnWidth(3) + saagharWidget->tableViewWidget->columnWidth(2) + saagharWidget->tableViewWidget->columnWidth(1);
                        int textWidth = 0;
                        QFontMetrics fontMetric(fnt);
                        if (!item->text().isEmpty()) {
                            textWidth = fontMetric.boundingRect(item->text()).width();
                        }
                        else {
                            textEdit = qobject_cast<QTextEdit*>(saagharWidget->tableViewWidget->cellWidget(row, col));
                            if (textEdit) {
                                fnt = SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/ProseText"));
                                color = SaagharWidget::resolvedColor(LS("SaagharWidget/Colors/ProseText"));
                                fontMetric = QFontMetrics(fnt);
                                textWidth = fontMetric.boundingRect(textEdit->toPlainText()).width();
                            }
                        }
                        int numOfLine = textWidth / rectX2;
                        thisRowHeight = thisRowHeight + (fontMetric.height() * numOfLine);
                    }
                }
                else if (columns == 1) {
                    rectX1 = 0;
                    rectX2 = saagharWidget->tableViewWidget->columnWidth(0);
                }

                QRect mesraRect;
                mesraRect.setCoords(rectX1, currentHeight, rectX2, currentHeight + thisRowHeight);
                col = col + span - 1;
                text = item->text();
                if (text.isEmpty() && textEdit) {
                    text = textEdit->toPlainText();
                }

                //QFont fnt = SaagharWidget::tableFont;
                fnt.setPointSizeF(fnt.pointSizeF() / scale);
                painter.setFont(fnt);
                painter.setPen(color);
                painter.setLayoutDirection(Qt::RightToLeft);
                int flags = item->textAlignment();
                if (span > 1) {
                    flags |= Qt::TextWordWrap;
                }
                painter.drawText(mesraRect, flags, text);
            }
        }

        currentHeight += thisRowHeight;
        if ((currentHeight + 50)*scale >= pageHeight) {
            printer->newPage();
            currentHeight = 10.0;
        }
    }
    painter.end();
}

void SaagharWindow::actionPrintClicked()
{
    if (!defaultPrinter) {
        defaultPrinter = new QPrinter(QPrinter::HighResolution);
    }
    QPrintDialog printDialog(defaultPrinter, this);

    if (printDialog.exec() == QDialog::Accepted) {
        print(defaultPrinter);
    }
}

void SaagharWindow::actionPrintPreviewClicked()
{
    if (!defaultPrinter) {
        defaultPrinter = new QPrinter(QPrinter::HighResolution);
    }
    printPreview(defaultPrinter);
}

void SaagharWindow::printPreview(QPrinter* printer)
{
    QPrintPreviewDialog* previewDialog = new QPrintPreviewDialog(printer, this);

    if (QtWin::isCompositionEnabled()) {
        previewDialog->setStyleSheet("QToolBar{ background-image:url(\":/resources/images/transp.png\"); border:none;}");
    }
    QtWin::easyBlurUnBlur(previewDialog, VARB("SaagharWindow/UseTransparecy"));

    connect(previewDialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
    previewDialog->exec();
}

QString SaagharWindow::convertToHtml(SaagharWidget* saagharObject)
{
    GanjoorPoem curPoem = sApp->databaseBrowser()->getPoem(saagharObject->currentPoem);
    if (curPoem.isNull()) {
        return "";
    }
    QString columnGroupFormat = QString("<COL WIDTH=%1>").arg(saagharObject->tableViewWidget->columnWidth(0));
    int numberOfCols = saagharObject->tableViewWidget->columnCount() - 1;
    int totalWidth = saagharObject->tableViewWidget->width();
    QString tableBody;
    if (numberOfCols == 3) {
        columnGroupFormat = QString("<COL WIDTH=%1><COL WIDTH=%2><COL WIDTH=%3>").arg(saagharObject->tableViewWidget->columnWidth(1)).arg(saagharObject->tableViewWidget->columnWidth(2)).arg(saagharObject->tableViewWidget->columnWidth(3));
        for (int row = 0; row < saagharObject->tableViewWidget->rowCount(); ++row) {
            int span = saagharObject->tableViewWidget->columnSpan(row, 1);
            if (span == 1 && SaagharWidget::CurrentViewStyle != SaagharWidget::SteppedHemistichLine) {
                QTableWidgetItem* firstItem = saagharObject->tableViewWidget->item(row, 1);
                QTableWidgetItem* secondItem = saagharObject->tableViewWidget->item(row, 3);
                QString firstText = "";
                QString secondText = "";
                if (firstItem) {
                    firstText = firstItem->text();
                }
                if (secondItem) {
                    secondText = secondItem->text();
                }
                tableBody += QString("\n<TR>\n<TD HEIGHT=%1 ALIGN=LEFT>%2</TD>\n<TD></TD>\n<TD ALIGN=RIGHT>%3</TD>\n</TR>\n").arg(saagharObject->tableViewWidget->rowHeight(row)).arg(firstText).arg(secondText);
            }
            else if (span == 1 && SaagharWidget::CurrentViewStyle == SaagharWidget::SteppedHemistichLine) {
                if (row == 1) {
                    totalWidth = saagharObject->tableViewWidget->columnWidth(2);
                }

                QTableWidgetItem* item = saagharObject->tableViewWidget->item(row, 2);
                QString text;
                QString align = "CENTER";
                if (item) {
                    if (item->textAlignment() & Qt::AlignRight) {
                        align = "LEFT";
                    }
                    else if (item->textAlignment() & Qt::AlignLeft) {
                        align = "RIGHT";
                    }

                    text = item->text();
                }
                tableBody += QString("\n<TR><TD HEIGHT=%1 ALIGN=%2>\n%3\n</TD>\n</TR>\n").arg(saagharObject->tableViewWidget->rowHeight(row)).arg(align).arg(text);
            }
            else {
                QTableWidgetItem* item = saagharObject->tableViewWidget->item(row, 1);
                QString mesraText = "";

                //title is centered by using each PoemViewStyle
                QString align = "CENTER";

                QStringList itemDataList;
                if (item) {
                    mesraText = item->text();
                    if (mesraText.isEmpty()) {
                        QTextEdit* textEdit = qobject_cast<QTextEdit*>(saagharObject->tableViewWidget->cellWidget(row, 1));
                        if (textEdit) {
                            mesraText = textEdit->toPlainText();
                        }
                    }
                    QVariant data = item->data(Qt::UserRole);
                    if (data.isValid() && !data.isNull()) {
                        itemDataList = data.toString().split("|", QString::SkipEmptyParts);
                    }
                }

                VersePosition itemPosition = CenteredVerse1;
                if (itemDataList.size() == 4) {
                    itemPosition = (VersePosition)itemDataList.at(3).toInt();
                }

                switch (itemPosition) {
                case Paragraph :
                case Single :
                    mesraText.replace("  ", " &nbsp;");
                    //mesraText = "<pre>"+mesraText+"</pre>";
                    align = "RIGHT";
                    break;

                default:
                    break;
                }
                tableBody += QString("\n<TR>\n<TD %5 HEIGHT=%1 WIDTH=%2 ALIGN=%3>%4</TD>\n</TR>\n").arg(saagharObject->tableViewWidget->rowHeight(qMax(0, row - 1))).arg(saagharObject->tableViewWidget->width()).arg(align).arg(mesraText)
                             .arg(SaagharWidget::CurrentViewStyle == SaagharWidget::SteppedHemistichLine ? "" : "COLSPAN=3");
            }
        }
    }
    else {
        return "";
    }

    QString tableAsHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n<HTML>\n<HEAD>\n"
                                  "<META HTTP-EQUIV=\"CONTENT-TYPE\" CONTENT=\"text/html; charset=utf-8\">\n"
                                  "<TITLE>%1</TITLE>\n<META NAME=\"GENERATOR\" CONTENT=\"Saaghar, a Persian poetry "
                                  "software, http://saaghar.pozh.org\">\n</HEAD>\n\n<BODY>\n"
                                  "<TABLE ALIGN=%2 DIR=RTL FRAME=VOID CELLSPACING=0 COLS=%3 RULES=NONE BORDER=0 "
                                  "STYLE=\"width: %4; FONT-FAMILY: %5; COLOR: %6; FONT-SIZE: %7; %8;\">\n"
                                  "<COLGROUP>%9</COLGROUP>\n<TBODY>\n%10\n</TBODY>\n"
                                  "</TABLE>\n</BODY>\n</HTML>\n")
                          .arg(curPoem._Title)
                          .arg("CENTER").arg(numberOfCols).arg(totalWidth)
                          .arg(SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/PoemText")).family())
                          .arg(SaagharWidget::resolvedColor(LS("SaagharWidget/Colors/PoemText")).name())
                          .arg(SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/PoemText")).pointSize())
                          .arg(SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/PoemText")).bold() ? "FONT-WEIGHT: bold" : "")
                          .arg(columnGroupFormat).arg(tableBody);

    return tableAsHTML;
}

QString SaagharWindow::convertToTeX(SaagharWidget* saagharObject)
{
    GanjoorPoem curPoem = sApp->databaseBrowser()->getPoem(saagharObject->currentPoem);
    if (curPoem.isNull()) {
        return "";
    }
    int numberOfCols = saagharObject->tableViewWidget->columnCount() - 1;
    QString poemType = "traditionalpoem";
    QString tableBody = "";
    bool poemEnvironmentEnded = false;
    if (numberOfCols == 3) {
        for (int row = 1; row < saagharObject->tableViewWidget->rowCount(); ++row) {
            int span = saagharObject->tableViewWidget->columnSpan(row, 1);
            if (span == 1) {
                QTableWidgetItem* firstItem = saagharObject->tableViewWidget->item(row, 1);
                QTableWidgetItem* secondItem = saagharObject->tableViewWidget->item(row, 3);
                QString firstText = "";
                QString secondText = "";
                if (firstItem) {
                    firstText = firstItem->text();
                }
                if (secondItem) {
                    secondText = secondItem->text();
                }

                tableBody += QString("%1 & %2 ").arg(firstText).arg(secondText);
                if (row + 1 != saagharObject->tableViewWidget->rowCount()) {
                    tableBody += "\\\\\n";
                }
            }
            else {
                QString align = "r";
                if (row == 0) { //title is centered by using each PoemViewStyle
                    align = "c";
                }
                QTableWidgetItem* item = saagharObject->tableViewWidget->item(row, 1);
                QString mesraText = "";
                QStringList itemDataList;
                if (item) {
                    mesraText = item->text();
                    if (mesraText.isEmpty()) {
                        QTextEdit* textEdit = qobject_cast<QTextEdit*>(saagharObject->tableViewWidget->cellWidget(row, 1));
                        if (textEdit) {
                            mesraText = textEdit->toPlainText();
                        }
                    }
                    QVariant data = item->data(Qt::UserRole);
                    if (data.isValid() && !data.isNull()) {
                        itemDataList = data.toString().split("|", QString::SkipEmptyParts);
                    }
                }

                VersePosition itemPosition = CenteredVerse1;
                if (itemDataList.size() == 4) {
                    itemPosition = (VersePosition)itemDataList.at(3).toInt();
                }
                switch (itemPosition) {
                case Paragraph :
                    tableBody += QString("%Empty Line: a tricky method, last beyt has TeX newline \"\\\\\"\n");//tricky, last beyt has TeX newline "\\"
                    tableBody += QString("\n\\end{%1}\n").arg(poemType);
                    tableBody += QString("%1").arg(mesraText);
                    poemEnvironmentEnded = true;
                    if (row + 1 != saagharObject->tableViewWidget->rowCount()) {
                        tableBody += "\\\\\n";
                        tableBody += QString("\n\\begin{%1}\n").arg(poemType);
                        poemEnvironmentEnded = false;
                    }
                    break;

                case Single :
                    mesraText.replace(" ", "\\ ");
                    poemType = "modernpoem";

                default:
                    tableBody += QString("%1").arg(mesraText);
                    if (row + 1 != saagharObject->tableViewWidget->rowCount()) {
                        tableBody += "\\\\\n";
                    }
                    break;
                }
            }
        }
    }
    else {
        return "";
    }

    QString endOfEnvironment = "";
    if (!poemEnvironmentEnded) {
        endOfEnvironment = QString("\\end{%1}\n").arg(poemType);
    }

    QString tableAsTeX = QString("%%%%%\n%This file is generated automatically by Saaghar %1, 2010 http://pozh.org\n%%%%%\n%XePersian and bidipoem packages must have been installed on your TeX distribution for compiling this document\n%You can compile this document by running XeLaTeX on it, twice.\n%%%%%\n\\documentclass{article}\n\\usepackage{hyperref}%\n\\usepackage[Kashida]{xepersian}\n\\usepackage{bidipoem}\n\\settextfont{%2}\n\\hypersetup{\npdftitle={%3},%\npdfsubject={Poem},%\npdfkeywords={Poem, Persian},%\npdfcreator={Saaghar, a Persian poetry software, http://saaghar.pozh.org},%\npdfview=FitV,\n}\n\\renewcommand{\\poemcolsepskip}{1.5cm}\n\\begin{document}\n\\begin{center}\n%3\\\\\n\\end{center}\n\\begin{%4}\n%5\n%6\\end{document}\n%End of document\n")
                         .arg(SAAGHAR_VERSION).arg(SaagharWidget::resolvedFont(LS("SaagharWidget/Fonts/PoemText")).family()).arg(curPoem._Title).arg(poemType).arg(tableBody).arg(endOfEnvironment);
    return tableAsTeX;
    /*******************************************************
    %1: Saaghar Version: SAAGHAR_VERSION
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

void SaagharWindow::actionNewWindowClicked()
{
    QProcess::startDetached("\"" + QCoreApplication::applicationFilePath() + "\"", QStringList() << "-fresh");
}

void SaagharWindow::actionNewTabClicked()
{
    insertNewTab();
}

void SaagharWindow::actFullScreenClicked(bool checked)
{
    setUpdatesEnabled(false);
    if (checked) {
        setWindowState(windowState() | Qt::WindowFullScreen);
        actionInstance("actionFullScreen")->setText(tr("Exit &Full Screen"));
        actionInstance("actionFullScreen")->setIcon(QIcon(ICON_PATH + "/no-fullscreen.png"));

        //disable transparent effect in fullscreen mode!
        QtWin::easyBlurUnBlur(this, false);
    }
    else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        actionInstance("actionFullScreen")->setText(tr("&Full Screen"));
        actionInstance("actionFullScreen")->setIcon(QIcon(ICON_PATH + "/fullscreen.png"));

        QtWin::easyBlurUnBlur(this, VARB("SaagharWindow/UseTransparecy"));
    }
    setUpdatesEnabled(true);
}

void SaagharWindow::openRandomPoem(int parentID, bool newPage)
{
    int actionData = parentID;

    int PoemID = sApp->databaseBrowser()->getRandomPoemID(&actionData);
    GanjoorPoem poem = sApp->databaseBrowser()->getPoem(PoemID);
    if (!poem.isNull() && poem._CatID == actionData)
        if (newPage || !saagharWidget) {
            newTabForItem(poem._ID, "PoemID", true);
        }
        else {
            saagharWidget->processClickedItem("PoemID", poem._ID, true);
        }
    else {
        openRandomPoem(parentID, newPage);    //not any random id exists, so repeat until finding a valid id
    }
}

void SaagharWindow::aboutSaaghar()
{
    QMessageBox about(this);

    QtWin::easyBlurUnBlur(&about, VARB("SaagharWindow/UseTransparecy"));

    QPixmap pixmap(":/resources/images/saaghar.png");
    about.setIconPixmap(pixmap);
    about.setWindowTitle(tr("About Saaghar"));
    about.setTextFormat(Qt::RichText);
    about.setText(tr("<br />%1 is a persian poem viewer software, it uses \"ganjoor.net\" database, and some of its initial codes are ported to C++ and Qt from \"desktop ganjoor\" that is a C# .NET application written by %2.<br /><br />Logo Designer: %3<br /><br />Author: %4,<br /><br />Home Page (English): %5<br />Home Page (Persian): %6<br />Mailing List: %7<br />Saaghar in FaceBook: %8<br /><br />Version: %9 - (git-rev: %10)<br />Build Time: %11")
                  .arg("<a href=\"http://saaghar.pozh.org\">" + tr("Saaghar") + "</a>")
                  .arg("<a href=\"http://www.gozir.com/\">" + tr("Hamid Reza Mohammadi") + "</a>")
                  .arg("<a href=\"http://www.phototak.com/\">" + tr("S. Nasser Alavizadeh") + "</a>")
                  .arg("<a href=\"http://pozh.org/\">" + tr("S. Razi Alavizadeh") + "</a>")
                  .arg("<a href=\"http://en.saaghar.pozh.org\">http://en.saaghar.pozh.org</a>")
                  .arg("<a href=\"http://saaghar.pozh.org\">http://saaghar.pozh.org</a>")
                  .arg("<a href=\"http://groups.google.com/group/saaghar/\">http://groups.google.com/group/saaghar</a>")
                  .arg("<a href=\"http://www.facebook.com/saaghar.p\">http://www.facebook.com/saaghar.p</a>")
                  .arg(SAAGHAR_VERSION).arg(GIT_REVISION).arg(BUILD_TIME));
    about.setStandardButtons(QMessageBox::Ok);
    about.setEscapeButton(QMessageBox::Ok);

    about.exec();
}

void SaagharWindow::helpContents()
{
    if (!QDesktopServices::openUrl("file:///" + sApp->defaultPath(SaagharApplication::ResourcesDir) + "/Saaghar-Manual.pdf")) {
        QMessageBox::warning(this, tr("Error"), tr("Help file not found!"));
    }
}

void SaagharWindow::closeCurrentTab()
{
    if (mainTabWidget->count() <= 1) {
        close();
    }

    tabCloser(mainTabWidget->currentIndex());
    return;
}

void SaagharWindow::actionImportNewSet()
{
    QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Browse for a new set"), QDir::homePath(), "Supported Files (*.gdb *.s3db *.zip);;Ganjoor DataBase (*.gdb *.s3db);;Compressed Data Sets (*.zip);;All Files (*.*)");
    if (!fileList.isEmpty()) {
        if (!DatabaseBrowser::dbUpdater) {
            DatabaseBrowser::dbUpdater = new DataBaseUpdater(this);
        }

        foreach (const QString &file, fileList) {

#ifdef SAAGHAR_DEBUG
            QMessageBox::information(0, "DEBUG!", QString("%1\n%2\n%3\n%4\n%5").arg(file, QFile::encodeName(file), file.toUtf8(), file.toLocal8Bit(), QTextCodec::codecForName("Windows-1256")->fromUnicode(file)) );
#endif

            DatabaseBrowser::dbUpdater->installItemToDB(file);
            QApplication::processEvents();
        }
    }
}

void SaagharWindow::setupUi()
{
#ifdef MEDIA_PLAYER
    if (SaagharWidget::musicPlayer) {
        allActionMap.insert("albumDockAction", SaagharWidget::musicPlayer->albumManagerDock()->toggleViewAction());
        actionInstance("albumDockAction")->setObjectName(QString::fromUtf8("albumDockAction"));
        actionInstance("albumDockAction")->setIcon(QIcon(ICON_PATH + "/album.png"));

        allActionMap.insert("toggleMusicPlayer", SaagharWidget::musicPlayer->toggleViewAction());
        actionInstance("toggleMusicPlayer")->setObjectName(QString::fromUtf8("ToggleMusicPlayerAction"));
        actionInstance("toggleMusicPlayer")->setIcon(QIcon(ICON_PATH + "/music-player.png"));

        connect(SaagharWidget::musicPlayer, SIGNAL(mediaChanged(QString,QString,int,bool)), this, SLOT(mediaInfoChanged(QString,QString,int)));
        connect(SaagharWidget::musicPlayer, SIGNAL(requestPageContainedMedia(int,bool)), this, SLOT(openChildPage(int,bool)));

        addToolBar(Qt::BottomToolBarArea, SaagharWidget::musicPlayer);
    }
#endif

    skipSearchToolBarResize = false;
    comboBoxSearchRegion = 0;
    setupSearchToolBarUi();
    ui->searchToolBar->hide();

    //remove menuToolBar if it's not needed!
    bool flagUnityGlobalMenu = false;
    foreach (QObject* o, ui->menuBar->children()) {
        if (o->inherits("QDBusServiceWatcher")) {
            flagUnityGlobalMenu = true;
            break;
        }
    }

    if (flagUnityGlobalMenu) {
        qDebug() << "Info: It seems that you are running Unity Desktop with global menubar!";
        delete ui->menuToolBar;
        ui->menuToolBar = 0;
    }

#ifdef Q_OS_MAC
    delete ui->menuToolBar;
    ui->menuToolBar = 0;
#endif

    if (ui->menuToolBar) {
        QHBoxLayout* menuBarLayout = new QHBoxLayout();
        menuBarLayout->addWidget(ui->menuBar, 0, Qt::AlignCenter | Qt::AlignLeft);
        menuBarLayout->setContentsMargins(1, 1, 1, 1);
        QWidget* menuBarContainer = new QWidget(this);
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
    // menuBookmarks is initialized in setupBookmarkManagerUi()
    menuTools = new QMenu(tr("&Tools"), ui->menuBar);
    menuTools->setObjectName(QString::fromUtf8("menuTools"));
    menuHelp = new QMenu(tr("&Help"), ui->menuBar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));

    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    //initialize BookmarkManager Ui
    setupBookmarkManagerUi();

    //Initialize Actions
    actionInstance("actionHome", ICON_PATH + "/home.png", tr("&Home"));

    if (ui->mainToolBar->layoutDirection() == Qt::LeftToRight) {
        actionInstance("actionPreviousPoem", ICON_PATH + "/previous.png", tr("&Previous"))->setShortcuts(QKeySequence::Back);
        actionInstance("actionNextPoem", ICON_PATH + "/next.png", tr("&Next"))->setShortcuts(QKeySequence::Forward);
    }
    else {
        actionInstance("actionPreviousPoem", ICON_PATH + "/next.png", tr("&Previous"))->setShortcuts(QKeySequence::Forward);
        actionInstance("actionNextPoem", ICON_PATH + "/previous.png", tr("&Next"))->setShortcuts(QKeySequence::Back);
    }

    actionInstance("actionCopy", ICON_PATH + "/copy.png", tr("&Copy"))->setShortcuts(QKeySequence::Copy);

    allActionMap.insert("searchToolbarAction", ui->searchToolBar->toggleViewAction());
    actionInstance("searchToolbarAction")->setIcon(QIcon(ICON_PATH + "/search.png"));
    actionInstance("searchToolbarAction")->setObjectName(QString::fromUtf8("searchToolbarAction"));
    actionInstance("searchToolbarAction")->setShortcuts(QKeySequence::Find);

    actionInstance("actionSettings", ICON_PATH + "/settings.png", tr("S&ettings"))->setMenuRole(QAction::PreferencesRole); //needed for Mac OS X

    actionInstance("actionViewInGanjoorSite", ICON_PATH + "/browse_net.png", tr("View in \"&ganjoor.net\""));

    actionInstance("actionExit", ICON_PATH + "/exit.png", tr("E&xit"))->setMenuRole(QAction::QuitRole); //needed for Mac OS X
    actionInstance("actionExit")->setShortcut(Qt::CTRL | Qt::Key_Q);

    actionInstance("actionNewTab", ICON_PATH + "/new_tab.png", tr("New &Tab"))->setShortcuts(QKeySequence::AddTab);

    actionInstance("actionNewWindow", ICON_PATH + "/new_window.png", tr("&New Window"))->setShortcuts(QKeySequence::New);

    actionInstance("actionAboutSaaghar", ":/resources/images/saaghar.png", tr("&About"))->setMenuRole(QAction::AboutRole); //needed for Mac OS X

    actionInstance("actionAboutQt", ICON_PATH + "/qt-logo.png", tr("About &Qt"))->setMenuRole(QAction::AboutQtRole); //needed for Mac OS X

    actionInstance("actionFaal", ICON_PATH + "/faal.png", tr("&Faal"))->setData("-1");

    actionInstance("actionPrint", ICON_PATH + "/print.png", tr("&Print..."))->setShortcuts(QKeySequence::Print);

    actionInstance("actionPrintPreview", ICON_PATH + "/print-preview.png", tr("Print Pre&view..."));

    actionInstance("actionExport", ICON_PATH + "/export.png", tr("&Export As..."))->setShortcuts(QKeySequence::SaveAs);

    actionInstance("actionExportAsPDF", ICON_PATH + "/export-pdf.png", tr("Exp&ort As PDF..."));

    actionInstance("actionHelpContents", ICON_PATH + "/help-contents.png", tr("&Help Contents..."))->setShortcuts(QKeySequence::HelpContents);

    actionInstance("actionCloseTab", ICON_PATH + "/close-tab.png", tr("&Close Tab"))->setShortcuts(QKeySequence::Close);

    actionInstance("actionRandom", ICON_PATH + "/random.png", tr("&Random"))->setShortcut(Qt::CTRL | Qt::Key_R);
    actionInstance("actionRandom")->setData("1");

    actionInstance("actionImportNewSet", ICON_PATH + "/import-to-database.png", tr("Insert New &Set..."));

    actionInstance("actionRemovePoet", ICON_PATH + "/remove-poet.png", tr("&Remove Poet..."));

    actionInstance("actionFullScreen", ICON_PATH + "/fullscreen.png", tr("&Full Screen"))->setCheckable(true);
#ifndef Q_OS_MAC
    actionInstance("actionFullScreen")->setShortcut(Qt::Key_F11);
#else
    actionInstance("actionFullScreen")->setShortcut(Qt::CTRL + Qt::Key_F11);
#endif

    actionInstance("actionCheckUpdates", ICON_PATH + "/check-updates.png", tr("Check for &Updates"));

    //The following actions are processed in 'namedActionTriggered()' slot
    actionInstance("SaagharWindow/ShowPhotoAtHome", ICON_PATH + "/show-photo-home.png", tr("&Show Photo at Home"))->setCheckable(true);
    actionInstance("SaagharWindow/ShowPhotoAtHome")->setChecked(VARB("SaagharWindow/ShowPhotoAtHome"));

    actionInstance("SaagharWindow/LockToolBars", ICON_PATH + "/lock-toolbars.png", tr("&Lock ToolBars"))->setCheckable(true);
    actionInstance("SaagharWindow/LockToolBars")->setChecked(VARB("SaagharWindow/LockToolBars"));
    ui->mainToolBar->setMovable(!VARB("SaagharWindow/LockToolBars"));
    if (ui->menuToolBar) {
        ui->menuToolBar->setMovable(!VARB("SaagharWindow/LockToolBars"));
    }
    ui->searchToolBar->setMovable(!VARB("SaagharWindow/LockToolBars"));
    parentCatsToolBar->setMovable(!VARB("SaagharWindow/LockToolBars"));
    m_breadCrumbToolBar->setMovable(!VARB("SaagharWindow/LockToolBars"));

#ifdef MEDIA_PLAYER
    SaagharWidget::musicPlayer->setMovable(!VARB("SaagharWindow/LockToolBars"));
#endif

    actionInstance("Ganjoor Verification", ICON_PATH + "/ocr-verification.png", tr("&OCR Verification"));

    //undo/redo actions
    globalRedoAction = undoGroup->createRedoAction(this, tr("&Redo"));
    globalRedoAction->setObjectName("globalRedoAction");
    globalUndoAction = undoGroup->createUndoAction(this, tr("&Undo"));
    globalUndoAction->setObjectName("globalUndoAction");
    if (ui->mainToolBar->layoutDirection() == Qt::LeftToRight) {
        globalRedoAction->setIcon(QIcon(ICON_PATH + "/redo.png"));
        globalUndoAction->setIcon(QIcon(ICON_PATH + "/undo.png"));
    }
    else {
        globalRedoAction->setIcon(QIcon(ICON_PATH + "/undo.png"));
        globalUndoAction->setIcon(QIcon(ICON_PATH + "/redo.png"));
    }

    globalRedoAction->setShortcuts(QKeySequence::Redo);
    globalUndoAction->setShortcuts(QKeySequence::Undo);

    actionInstance("fixedNameRedoAction", "", tr("&Redo"))->setIcon(globalRedoAction->icon());
    actionInstance("fixedNameUndoAction", "", tr("&Undo"))->setIcon(globalUndoAction->icon());
    actionInstance("fixedNameRedoAction")->setEnabled(globalRedoAction->isEnabled());
    actionInstance("fixedNameUndoAction")->setEnabled(globalUndoAction->isEnabled());

    // ImportGanjoorBookmarks is initialized in setupBookmarkManagerUi()

    //Poem View Styles
    QMenu* poemViewStylesMenu = new QMenu(tr("Poem View Styles"));
    QActionGroup* poemViewStylesGroup = new QActionGroup(this);

    poemViewStylesMenu->addAction(actionInstance("TwoHemistichPoemViewStyle", ICON_PATH + "/two-hemistich-line.png", QObject::tr("&Two Hemistich Line")));
    actionInstance("TwoHemistichPoemViewStyle")->setParent(poemViewStylesMenu);
    actionInstance("TwoHemistichPoemViewStyle")->setActionGroup(poemViewStylesGroup);
    actionInstance("TwoHemistichPoemViewStyle")->setCheckable(true);
    actionInstance("TwoHemistichPoemViewStyle")->setData(SaagharWidget::TwoHemistichLine);

    poemViewStylesMenu->addAction(actionInstance("OneHemistichPoemViewStyle", ICON_PATH + "/one-hemistich-line.png", QObject::tr("&One Hemistich Line")));
    actionInstance("OneHemistichPoemViewStyle")->setParent(poemViewStylesMenu);
    actionInstance("OneHemistichPoemViewStyle")->setActionGroup(poemViewStylesGroup);
    actionInstance("OneHemistichPoemViewStyle")->setCheckable(true);
    actionInstance("OneHemistichPoemViewStyle")->setData(SaagharWidget::OneHemistichLine);

    poemViewStylesMenu->addAction(actionInstance("SteppedHemistichPoemViewStyle", ICON_PATH + "/stepped-hemistich-line.png", QObject::tr("&Stepped Hemistich Line")));
    actionInstance("SteppedHemistichPoemViewStyle")->setParent(poemViewStylesMenu);
    actionInstance("SteppedHemistichPoemViewStyle")->setActionGroup(poemViewStylesGroup);
    actionInstance("SteppedHemistichPoemViewStyle")->setCheckable(true);
    actionInstance("SteppedHemistichPoemViewStyle")->setData(SaagharWidget::SteppedHemistichLine);

    outlineTree = new OutlineTree;
    m_outlineDock = new QDockWidget(tr("Outline"), this);
    m_outlineDock->setObjectName("outlineDock");
    m_outlineDock->setWidget(outlineTree);
    m_outlineDock->setStyleSheet("QDockWidget::title { background: transparent; text-align: left; padding: 0 10 0 10;}"
                                 "QDockWidget::close-button, QDockWidget::float-button { background: transparent;}");
    addDockWidget(Qt::RightDockWidgetArea, m_outlineDock);
    allActionMap.insert("outlineDockAction", m_outlineDock->toggleViewAction());
    actionInstance("outlineDockAction")->setIcon(QIcon(ICON_PATH + "/outline.png"));
    actionInstance("outlineDockAction")->setObjectName(QString::fromUtf8("outlineDockAction"));

    switch (SaagharWidget::CurrentViewStyle) {
    case SaagharWidget::TwoHemistichLine:
        actionInstance("TwoHemistichPoemViewStyle")->setChecked(true);
        break;
    case SaagharWidget::OneHemistichLine:
        actionInstance("OneHemistichPoemViewStyle")->setChecked(true);
        break;
    case SaagharWidget::SteppedHemistichLine:
        actionInstance("SteppedHemistichPoemViewStyle")->setChecked(true);
        break;

    default:
        break;
    }

    actionInstance("DownloadRepositories", ICON_PATH + "/download-sets-repositories.png", QObject::tr("&Download From Repositories..."));
#if 0
    actionInstance("Registeration", ICON_PATH + "/registeration.png", QObject::tr("&Registeration..."));
#endif

    //Inserting main menu items
    ui->menuBar->addMenu(menuFile);
    ui->menuBar->addMenu(menuNavigation);
    ui->menuBar->addMenu(menuView);
    if (menuBookmarks) {
        ui->menuBar->addMenu(menuBookmarks);
    }
    ui->menuBar->addMenu(menuTools);
    ui->menuBar->addMenu(menuHelp);

    //QMenuBar style sheet
    ui->menuBar->setStyleSheet("QMenuBar {background-color: transparent; border:none;}"
                               "QMenuBar::item {/*color: black;*/spacing: 3px;padding: 1px 7px 4px 7px;background: transparent;border-radius: 4px;}"
                               "QMenuBar::item:selected { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadada, stop: 0.4 #d4d4d4, stop: 0.5 #c7c7c7, stop: 1.0 #dadada); border: 1px; }"
                               "QMenuBar::item:pressed {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #9a9a9a, stop: 0.4 #949494, stop: 0.5 #a7a7a7, stop: 1.0 #9a9a9a); color: #fefefe;"
                               "border: 1px; border-top-left-radius: 4px; border-top-right-radius: 4px; border-bottom-left-radius: 0px; border-bottom-right-radius: 0px; border-bottom: none;}");

    int menuBarWidth = 0;
    foreach (QAction* act, ui->menuBar->actions()) {
        if (act) {
            QString actText = act->text();
            actText.remove("&");
            menuBarWidth += ui->menuBar->fontMetrics().boundingRect(actText).width() + 14 + ui->menuBar->style()->pixelMetric(QStyle::PM_MenuBarItemSpacing); //14=2*(3(spacing)+4(radius))
        }
    }

    menuBarWidth = menuBarWidth + 3; //3 is offset of first item!
    int styleWidth = ui->menuBar->style()->pixelMetric(QStyle::PM_MenuBarHMargin) * 2 + ui->menuBar->style()->pixelMetric(QStyle::PM_MenuBarPanelWidth) * 2;

    styleWidth = qMax(styleWidth, 5);
    ui->menuBar->setMinimumWidth(menuBarWidth + styleWidth);
    ui->menuBar->setMaximumWidth(menuBarWidth + 2 * styleWidth);


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



    QMenu* toolbarsView = new QMenu(tr("ToolBars"), menuView);
    if (ui->menuToolBar) {
        toolbarsView->addAction(ui->menuToolBar->toggleViewAction());
    }
    toolbarsView->addAction(ui->mainToolBar->toggleViewAction());
    toolbarsView->addAction(ui->searchToolBar->toggleViewAction());
    toolbarsView->addAction(parentCatsToolBar->toggleViewAction());
    toolbarsView->addAction(m_breadCrumbToolBar->toggleViewAction());

#ifdef MEDIA_PLAYER
    toolbarsView->addAction(actionInstance("toggleMusicPlayer"));
#endif

    toolbarsView->addSeparator();
    toolbarsView->addAction(actionInstance("SaagharWindow/LockToolBars"));
    menuView->addMenu(toolbarsView);

    QMenu* panelsView = new QMenu(tr("Panels"), menuView);
    panelsView->addAction(actionInstance("outlineDockAction"));
#ifdef MEDIA_PLAYER
    panelsView->addAction(actionInstance("albumDockAction"));
#endif
    panelsView->addAction(actionInstance("bookmarkManagerDockAction"));
    menuView->addMenu(panelsView);

    menuView->addSeparator();
    toolBarViewActions(ui->mainToolBar, menuView, true);
    //checked actions, must be after above line
    toolbarViewChanges(actionInstance(VARS("SaagharWindow/MainToolBarStyle")));
    toolbarViewChanges(actionInstance(VARS("SaagharWindow/MainToolBarSize")));

    menuView->addMenu(poemViewStylesMenu);
    menuView->addAction(actionInstance("SaagharWindow/ShowPhotoAtHome"));
    menuView->addSeparator();
    menuView->addAction(actionInstance("actionFullScreen"));

    // menuBookmarks is initialized in setupBookmarkManagerUi()

    menuTools->addAction(actionInstance("searchToolbarAction"));
    menuTools->addAction(actionInstance("actionCopy"));
    menuTools->addSeparator();
    menuTools->addAction(actionInstance("actionViewInGanjoorSite"));
    menuTools->addAction(actionInstance("Ganjoor Verification"));
    menuTools->addSeparator();
    menuTools->addAction(actionInstance("actionImportNewSet"));
    menuTools->addAction(actionInstance("actionRemovePoet"));
    menuTools->addAction(actionInstance("DownloadRepositories"));
    menuTools->addSeparator();
    menuTools->addAction(actionInstance("actionSettings"));

    menuHelp->addAction(actionInstance("actionHelpContents"));
    menuHelp->addSeparator();
    menuHelp->addAction(actionInstance("actionCheckUpdates"));
#if 0
    menuHelp->addSeparator();
    menuHelp->addAction(actionInstance("Registeration"));
#endif
    menuHelp->addSeparator();
    menuHelp->addAction(actionInstance("actionAboutSaaghar"));
    menuHelp->addAction(actionInstance("actionAboutQt"));

    QStringList mainToolBarItems = sApp->mainToolBarItems();
    mainToolBarItems.removeAll(QLatin1String(""));

    //Inserting mainToolbar's items
    for (int i = 0; i < mainToolBarItems.size(); ++i) {
        if (allActionMap.contains(mainToolBarItems.at(i)) || mainToolBarItems.at(i).contains("separator", Qt::CaseInsensitive)) {
            ui->mainToolBar->addAction(actionInstance(mainToolBarItems.at(i)));
        }
        else {
            mainToolBarItems[i] = "";
        }
    }

    //removing no more supported items
    mainToolBarItems.removeAll(QLatin1String(""));
    sApp->setMainToolBarItems(mainToolBarItems);

    addToolBar(Qt::TopToolBarArea, m_breadCrumbToolBar);
    if (ui->menuToolBar) {
        insertToolBar(m_breadCrumbToolBar, ui->menuToolBar);
    }

    addToolBar(Qt::TopToolBarArea, parentCatsToolBar);
    parentCatsToolBar->hide();

#ifdef MEDIA_PLAYER
    if (SaagharWidget::musicPlayer) {
        insertToolBar(SaagharWidget::musicPlayer, ui->searchToolBar);
    }
    else
#endif
    {
        addToolBar(Qt::BottomToolBarArea, ui->searchToolBar);
    }

    addToolBar(Qt::LeftToolBarArea, ui->mainToolBar);
}

QMenu* SaagharWindow::cornerMenu()
{
    if (!m_cornerMenu) {
        m_cornerMenu = new QMenu(0);

        m_cornerMenu->addAction(actionInstance("actionNewTab"));
        m_cornerMenu->addAction(actionInstance("actionNewWindow"));
        m_cornerMenu->addSeparator();
        m_cornerMenu->addMenu(menuOpenedTabs);
        m_cornerMenu->addMenu(menuClosedTabs);
        m_cornerMenu->addSeparator();
        m_cornerMenu->addMenu(menuView);
        m_cornerMenu->addMenu(menuTools);
        m_cornerMenu->addSeparator();
        m_cornerMenu->addAction(actionInstance("actionFaal"));
        m_cornerMenu->addAction(actionInstance("actionExportAsPDF"));
        m_cornerMenu->addAction(actionInstance("actionPrint"));
        m_cornerMenu->addSeparator();
        m_cornerMenu->addAction(actionInstance("actionSettings"));
        m_cornerMenu->addAction(actionInstance("DownloadRepositories"));
        m_cornerMenu->addAction(actionInstance("actionCheckUpdates"));
#if 0
        m_cornerMenu->addAction(actionInstance("Registeration"));
#endif
        m_cornerMenu->addAction(actionInstance("actionAboutSaaghar"));
        m_cornerMenu->addSeparator();
        m_cornerMenu->addAction(actionInstance("actionExit"));
    }

    return m_cornerMenu;
}

void SaagharWindow::loadAudioForCurrentTab(SaagharWidget* old_saagharWidget)
{
#ifdef MEDIA_PLAYER
    if (!SaagharWidget::musicPlayer) {
        return;
    }
    QString path;
    QString title;
    QString album;
    if (old_saagharWidget) {
        SaagharWidget::musicPlayer->getFromAlbum(old_saagharWidget->currentPoem, &path, &title, 0, &album);
        SaagharWidget::musicPlayer->insertToAlbum(old_saagharWidget->currentPoem, path, title, SaagharWidget::musicPlayer->currentTime(), album);
    }
    path = "";
    title = "";
    qint64 time = 0;
    SaagharWidget::musicPlayer->getFromAlbum(saagharWidget->currentPoem, &path, &title, &time);
    if (SaagharWidget::musicPlayer->source() != path) {
        SaagharWidget::musicPlayer->setSource(path, saagharWidget->currentLocationList.join(">") + ">" + saagharWidget->currentPoemTitle, saagharWidget->currentPoem);
        SaagharWidget::musicPlayer->setCurrentTime(time);
    }

    bool isEnabled = (saagharWidget->pageMetaInfo.type == SaagharWidget::PoemViewerPage);
    SaagharWidget::musicPlayer->setEnabled(isEnabled);
    if (!isEnabled) {
        SaagharWidget::musicPlayer->stop();
    }
#endif // MEDIA_PLAYER
}

void SaagharWindow::showSearchOptionMenu()
{
    if (!searchOptionMenu || !SaagharWidget::lineEditSearchText || !SaagharWidget::lineEditSearchText->optionsButton()) {
        return;
    }
    searchOptionMenu->exec(SaagharWidget::lineEditSearchText->optionsButton()->mapToGlobal(QPoint(0, SaagharWidget::lineEditSearchText->optionsButton()->height()))/*QCursor::pos()*/);
}

void SaagharWindow::showSearchTips()
{
    QString searchTips;
    searchTips = tr("<b>Tip1:</b> Search operators and commands:"
                    "%5"
                    "<TR><TD%3 id=\"and-operator\"><b>%1+%2</b></TD>%8<TD%4>Mesras containing both <b>%1</b> and <b>%2</b>, <i>at any order</i>.</TD></TR>"
                    "<TR><TD%3><b>%1**%2</b></TD>%8<TD%4>Mesras containing both <b>%1</b> and <b>%2</b>, <i>at this order</i>.</TD></TR>"
                    "<TR><TD%3><b>%1 %2</b></TD>%8<TD%4>Same as <b><a href=\"#and-operator\">%1+%2</a></b>.</TD></TR>"
                    "<TR><TD%3><b>%1 | %2</b></TD>%8<TD%4>Mesras containing <b>%1</b> or <b>%2</b>, or both.</TD></TR>"
                    "<TR><TD%3><b>%1 -%2</b></TD>%8<TD%4>Mesras containing <b>%1</b>, but not <b>%2</b>.</TD></TR>"
                    "<TR><TD%3><b>\"%1\"</b></TD>%8<TD%4>Mesras containing the whole word <b>%1</b>.</TD></TR>"
                    "<TR><TD%3><b>\"%1 %2\"</b></TD>%8<TD%4>Mesras containing the whole mixed word <b>%1 %2</b>.</TD></TR>"
                    "<TR><TD%3><b>Sp*ng</b></TD>%8<TD%4>Mesras containing any phrase started with <b>Sp</b> and ended with <b>ng</b>; i.e: Mesras containing <b>Spring</b> or <b>Spying</b> or <b>Spoking</b> or...</TD></TR>"
                    "<TR><TD%3><b>=%1</b></TD>%8<TD%4>Mesras containing the word <b>%1</b> <i>as Rhyme</i>.</TD></TR>"
                    "<TR><TD%3><b>==%1</b></TD>%8<TD%4>Mesras containing the word <b>%1</b> <i>as Radif</i>.</TD></TR>"
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
    QTextBrowserDialog searchTipsDialog(this, tr("Search Tips..."), searchTips, QPixmap(ICON_PATH + "/search.png").scaledToHeight(64, Qt::SmoothTransformation));
    searchTipsDialog.exec();
}

void SaagharWindow::showStatusText(const QString &message, int newLevelsCount)
{
    if (!splashScreen(QExtendedSplashScreen) || splashScreen(QExtendedSplashScreen)->isFinished()) {
        return;
    }

    splashScreen(QExtendedSplashScreen)->addToMaximum(newLevelsCount, false);
    splashScreen(QExtendedSplashScreen)->showMessage(message);
}

void SaagharWindow::onDatabaseUpdate()
{
    outlineTree->refreshTree();
    selectSearchRange->clear();
    multiSelectInsertItems(selectSearchRange);
    setHomeAsDirty();
}

QAction* SaagharWindow::actionInstance(const QString &actionObjectName, QString iconPath, QString displayName)
{
    QAction* action = 0;

    if (actionObjectName.isEmpty() || actionObjectName.contains("separator", Qt::CaseInsensitive)) {
        action = new QAction(this);
        action->setSeparator(true);
        return action;
    }

    action = allActionMap.value(actionObjectName, 0);
    if (!allActionMap.contains(actionObjectName)) {
        if (displayName.isEmpty()) {
            displayName = actionObjectName;
            displayName.remove("action");
            displayName.replace("__", "&");
            displayName.replace("_", " ");
            displayName = tr(displayName.toUtf8().data());
        }

        action = new QAction(displayName, this);
        if (QFile::exists(iconPath)) {
            action->setIcon(QIcon(iconPath));
        }

        action->setObjectName(actionObjectName);
        allActionMap.insert(actionObjectName, action);
    }

    if (action) {
        connect(action, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));
    }

    return action;
}

void SaagharWindow::deleteActionInstance(const QString &actionObjectName)
{
    if (allActionMap.contains(actionObjectName)) {
        delete allActionMap.value(actionObjectName);
        allActionMap.insert(actionObjectName, 0);
    }
}

void SaagharWindow::createConnections()
{
    //File
    connect(actionInstance("actionNewTab")              ,   SIGNAL(triggered())     ,   this, SLOT(actionNewTabClicked()));
    connect(actionInstance("actionNewWindow")           ,   SIGNAL(triggered())     ,   this, SLOT(actionNewWindowClicked()));
    connect(actionInstance("actionCloseTab")            ,   SIGNAL(triggered())     ,   this, SLOT(closeCurrentTab()));
    connect(actionInstance("actionExportAsPDF")         ,   SIGNAL(triggered())     ,   this, SLOT(actionExportAsPDFClicked()));
    connect(actionInstance("actionExport")              ,   SIGNAL(triggered())     ,   this, SLOT(actionExportClicked()));
    connect(actionInstance("actionPrintPreview")        ,   SIGNAL(triggered())     ,   this, SLOT(actionPrintPreviewClicked()));
    connect(actionInstance("actionPrint")               ,   SIGNAL(triggered())     ,   this, SLOT(actionPrintClicked()));
    connect(actionInstance("actionExit")                ,   SIGNAL(triggered())     ,   this, SLOT(close()));

    //Navigation
    connect(globalRedoAction                            ,   SIGNAL(changed())       ,   this, SLOT(namedActionTriggered()));
    connect(globalUndoAction                            ,   SIGNAL(changed())       ,   this, SLOT(namedActionTriggered()));
    connect(actionInstance("actionHome")                ,   SIGNAL(triggered())     ,   this, SLOT(actionHomeClicked()));
    connect(actionInstance("actionPreviousPoem")        ,   SIGNAL(triggered())     ,   this, SLOT(actionPreviousPoemClicked()));
    connect(actionInstance("actionNextPoem")            ,   SIGNAL(triggered())     ,   this, SLOT(actionNextPoemClicked()));

    //View
    connect(actionInstance("actionFullScreen")          ,   SIGNAL(toggled(bool))   ,   this, SLOT(actFullScreenClicked(bool)));

    //Tools
    connect(actionInstance("actionViewInGanjoorSite")   ,   SIGNAL(triggered())     ,   this, SLOT(actionGanjoorSiteClicked()));
    connect(actionInstance("actionCopy")                ,   SIGNAL(triggered())     ,   this, SLOT(copySelectedItems()));
    connect(actionInstance("actionSettings")            ,   SIGNAL(triggered())     ,   this, SLOT(showSettingsDialog()));
    connect(actionInstance("actionRemovePoet")          ,   SIGNAL(triggered())     ,   this, SLOT(actionRemovePoet()));
    connect(actionInstance("actionImportNewSet")        ,   SIGNAL(triggered())     ,   this, SLOT(actionImportNewSet()));

    //Help
    connect(actionInstance("actionHelpContents")        ,   SIGNAL(triggered())     ,   this, SLOT(helpContents()));
    connect(actionInstance("actionCheckUpdates")        ,   SIGNAL(triggered())     ,   this, SLOT(checkForUpdates()));
    connect(actionInstance("actionAboutSaaghar")        ,   SIGNAL(triggered())     ,   this, SLOT(aboutSaaghar()));
    connect(actionInstance("actionAboutQt")             ,   SIGNAL(triggered())     ,   qApp, SLOT(aboutQt()));

    //searchToolBar connections
    connect(SaagharWidget::lineEditSearchText                           ,   SIGNAL(returnPressed()) ,   this, SLOT(searchStart()));
}

//Settings Dialog
void SaagharWindow::showSettingsDialog()
{
    m_settingsDialog = new Settings(this);

    QtWin::easyBlurUnBlur(m_settingsDialog, VARB("SaagharWindow/UseTransparecy"));

    connect(m_settingsDialog->ui->pushButtonRandomOptions, SIGNAL(clicked()), this, SLOT(customizeRandomDialog()));

    /************initializing saved state of Settings dialog object!************/
    m_settingsDialog->ui->uiLanguageComboBox->addItems(QStringList() << "fa" << "en");
    int langIndex = m_settingsDialog->ui->uiLanguageComboBox->findText(VARS("General/UILanguage"), Qt::MatchExactly);
    m_settingsDialog->ui->uiLanguageComboBox->setCurrentIndex(langIndex);

    m_settingsDialog->ui->pushButtonActionBottom->setIcon(QIcon(ICON_PATH + "/down.png"));
    m_settingsDialog->ui->pushButtonActionTop->setIcon(QIcon(ICON_PATH + "/up.png"));

    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        m_settingsDialog->ui->pushButtonActionAdd->setIcon(QIcon(ICON_PATH + "/left.png"));
        m_settingsDialog->ui->pushButtonActionRemove->setIcon(QIcon(ICON_PATH + "/right.png"));
    }
    else {
        m_settingsDialog->ui->pushButtonActionAdd->setIcon(QIcon(ICON_PATH + "/right.png"));
        m_settingsDialog->ui->pushButtonActionRemove->setIcon(QIcon(ICON_PATH + "/left.png"));
    }

    m_settingsDialog->ui->spinBoxPoetsPerGroup->setValue(SaagharWidget::maxPoetsPerGroup);

    m_settingsDialog->ui->checkBoxAutoUpdates->setChecked(VARB("General/AutoCheckUpdates"));

    //database
    m_settingsDialog->ui->lineEditDataBasePath->setText(sApp->defaultPath(SaagharApplication::DatabaseDirs));
    connect(m_settingsDialog->ui->pushButtonDataBasePath, SIGNAL(clicked()), m_settingsDialog, SLOT(browseForDataBasePath()));

    m_settingsDialog->ui->checkBoxBeytNumbers->setChecked(SaagharWidget::showBeytNumbers);

    m_settingsDialog->ui->checkBoxBackground->setChecked(SaagharWidget::backgroundImageState);
    m_settingsDialog->ui->lineEditBackground->setText(SaagharWidget::backgroundImagePath);
    m_settingsDialog->ui->lineEditBackground->setEnabled(SaagharWidget::backgroundImageState);
    m_settingsDialog->ui->pushButtonBackground->setEnabled(SaagharWidget::backgroundImageState);
    connect(m_settingsDialog->ui->pushButtonBackground, SIGNAL(clicked()), m_settingsDialog, SLOT(browseForBackground()));

    //splash screen
    m_settingsDialog->ui->checkBoxSplashScreen->setChecked(VARB("General/DisplaySplashScreen"));

    //initialize Action's Tables
    m_settingsDialog->initializeActionTables(allActionMap, sApp->mainToolBarItems());

    connect(m_settingsDialog->ui->pushButtonActionBottom, SIGNAL(clicked()), m_settingsDialog, SLOT(bottomAction()));
    connect(m_settingsDialog->ui->pushButtonActionTop, SIGNAL(clicked()), m_settingsDialog, SLOT(topAction()));
    connect(m_settingsDialog->ui->pushButtonActionAdd, SIGNAL(clicked()), m_settingsDialog, SLOT(addActionToToolbarTable()));
    connect(m_settingsDialog->ui->pushButtonActionRemove, SIGNAL(clicked()), m_settingsDialog, SLOT(removeActionFromToolbarTable()));

    m_settingsDialog->ui->checkBoxTransparent->setChecked(VARB("SaagharWindow/UseTransparecy"));

    m_compareSettings = m_settingsDialog->ui->uiLanguageComboBox->currentText() +
                        m_settingsDialog->ui->lineEditDataBasePath->text() +
                        m_settingsDialog->ui->lineEditIconTheme->text() +
                        (m_settingsDialog->ui->checkBoxIconTheme->isChecked() ? "TRUE" : "FALSE");
    /************end of initialization************/

    if (m_settingsDialog->exec()) {
        applySettings();
    }
    m_settingsDialog->deleteLater();
    m_settingsDialog = 0;
}

void SaagharWindow::applySettings()
{
    if (!m_settingsDialog) {
        return;
    }

    setUpdatesEnabled(false);

    VAR_DECL("General/UILanguage", m_settingsDialog->ui->uiLanguageComboBox->currentText());

    SaagharWidget::maxPoetsPerGroup = m_settingsDialog->ui->spinBoxPoetsPerGroup->value();

    VAR_DECL("General/AutoCheckUpdates", QVariant(m_settingsDialog->ui->checkBoxAutoUpdates->isChecked()));

    //database path
    sApp->setDefaultPath(SaagharApplication::DatabaseDirs, m_settingsDialog->ui->lineEditDataBasePath->text());

    SaagharWidget::showBeytNumbers = m_settingsDialog->ui->checkBoxBeytNumbers->isChecked();

    SaagharWidget::backgroundImageState = m_settingsDialog->ui->checkBoxBackground->isChecked();
    SaagharWidget::backgroundImagePath = m_settingsDialog->ui->lineEditBackground->text();

    //colors
    SaagharWidget::backgroundColor = m_settingsDialog->backgroundFontColor->color();
    SaagharWidget::matchedTextColor = m_settingsDialog->matchedFontColor->color();

    //splash screen
    VAR_DECL("General/DisplaySplashScreen", m_settingsDialog->ui->checkBoxSplashScreen->isChecked());

    //toolbar items
    QStringList mainToolBarItems;
    for (int i = 0; i < m_settingsDialog->ui->tableWidgetToolBarActions->rowCount(); ++i) {
        QTableWidgetItem* item = m_settingsDialog->ui->tableWidgetToolBarActions->item(i, 0);
        if (item) {
            mainToolBarItems.append(item->data(Qt::UserRole + 1).toString());
        }
    }

    sApp->setMainToolBarItems(mainToolBarItems);

    VAR_DECL("SaagharWidget/UseGlobalTextFormat", m_settingsDialog->ui->globalFontColorGroupBox->isChecked());
    //QFont fnt;
    VAR_DECL("SaagharWidget/Fonts/OutLine", m_settingsDialog->outlineFontColor->sampleFont());
    VAR_DECL("SaagharWidget/Fonts/Default", m_settingsDialog->globalTextFontColor->sampleFont());
    VAR_DECL("SaagharWidget/Fonts/PoemText", m_settingsDialog->poemTextFontColor->sampleFont());
    VAR_DECL("SaagharWidget/Fonts/ProseText", m_settingsDialog->proseTextFontColor->sampleFont());
    VAR_DECL("SaagharWidget/Fonts/SectionName", m_settingsDialog->sectionNameFontColor->sampleFont());
    VAR_DECL("SaagharWidget/Fonts/Titles", m_settingsDialog->titlesFontColor->sampleFont());
    VAR_DECL("SaagharWidget/Fonts/Numbers", m_settingsDialog->numbersFontColor->sampleFont());

    VAR_DECL("SaagharWidget/Colors/OutLine", m_settingsDialog->outlineFontColor->color());
    VAR_DECL("SaagharWidget/Colors/Default", m_settingsDialog->globalTextFontColor->color());
    VAR_DECL("SaagharWidget/Colors/PoemText", m_settingsDialog->poemTextFontColor->color());
    VAR_DECL("SaagharWidget/Colors/ProseText", m_settingsDialog->proseTextFontColor->color());
    VAR_DECL("SaagharWidget/Colors/SectionName", m_settingsDialog->sectionNameFontColor->color());
    VAR_DECL("SaagharWidget/Colors/Titles", m_settingsDialog->titlesFontColor->color());
    VAR_DECL("SaagharWidget/Colors/Numbers", m_settingsDialog->numbersFontColor->color());

    VAR_DECL("SaagharWindow/UseTransparecy", m_settingsDialog->ui->checkBoxTransparent->isChecked());
    QtWin::easyBlurUnBlur(this, VARB("SaagharWindow/UseTransparecy"));
    QtWin::easyBlurUnBlur(m_settingsDialog, VARB("SaagharWindow/UseTransparecy"));

    ui->mainToolBar->clear();
    //Inserting main toolbar Items
    for (int i = 0; i < mainToolBarItems.size(); ++i) {
        ui->mainToolBar->addAction(actionInstance(mainToolBarItems.at(i)));
    }

    if (saagharWidget) {
        saagharWidget->loadSettings();

        if (saagharWidget->tableViewWidget->columnCount() == 1 && saagharWidget->tableViewWidget->rowCount() > 0 && saagharWidget->currentCat != 0) {
//            QTableWidgetItem* item = saagharWidget->tableViewWidget->item(0, 0);
            //it seems after using QTextEdit this is not needed!
//              if (item)
//              {
//                  QString text = item->text();
//                  qDebug()<<"WWWWWWHERE IS HERE="<<text<<saagharWidget->currentCat<<saagharWidget->currentCaption;
//                  int textWidth = saagharWidget->tableViewWidget->fontMetrics().boundingRect(text).width();
//                  int totalWidth = saagharWidget->tableViewWidget->columnWidth(0);
//                  saagharWidget->tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(saagharWidget->tableViewWidget->fontMetrics(), textWidth, totalWidth));
//              }
        }
        else if (saagharWidget->currentCat == 0 && saagharWidget->currentPoem == 0) { //it's Home.
            saagharWidget->homeResizeColsRows();
        }
    }
#ifndef Q_OS_MAC //This doesn't work on MACX and moved to "SaagharWidget::loadSettings()"
    //apply new text and background color, and also background picture
    loadTabWidgetSettings();
#endif
    // save new settings to disk
    sApp->saveSettings();

    // all tab have to be refreshed
    setAllAsDirty();

    setUpdatesEnabled(true);

    QString optionsCompare = m_settingsDialog->ui->uiLanguageComboBox->currentText() +
                             m_settingsDialog->ui->lineEditDataBasePath->text() +
                             m_settingsDialog->ui->lineEditIconTheme->text() +
                             (m_settingsDialog->ui->checkBoxIconTheme->isChecked() ? "TRUE" : "FALSE");

    if (m_compareSettings != optionsCompare) {
        QMessageBox::information(this, tr("Need to Relaunch!"), tr("Some of changes are applied after relaunch!"));
        m_compareSettings = optionsCompare;
    }

    m_settingsDialog->applySettings();
    sApp->applySettings();
}

void SaagharWindow::loadTabWidgetSettings()
{
    QPalette p(mainTabWidget->palette());
    if (SaagharWidget::backgroundImageState && QFile::exists(SaagharWidget::backgroundImagePath)) {
        p.setBrush(QPalette::Base, QBrush(QPixmap(SaagharWidget::backgroundImagePath)));
    }
    else {
        p.setColor(QPalette::Base, SaagharWidget::backgroundColor);
    }
    p.setColor(QPalette::Text, SaagharWidget::resolvedColor(LS("SaagharWidget/Colors/PoemText")));
    mainTabWidget->setPalette(p);

    outlineTree->setTreeFont(VAR("SaagharWidget/Fonts/OutLine").value<QFont>());
    outlineTree->setTreeColor(VAR("SaagharWidget/Colors/OutLine").value<QColor>());
}

void SaagharWindow::saveSettings()
{
    if (SaagharWidget::bookmarks) {
        QFile bookmarkFile(sApp->defaultPath(SaagharApplication::BookmarksFile));
        if (!bookmarkFile.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Bookmarks"), tr("Can not write the bookmark file %1:\n%2.")
                                 .arg(bookmarkFile.fileName())
                                 .arg(bookmarkFile.errorString()));
        }
        else {
            SaagharWidget::bookmarks->write(&bookmarkFile);
        }
    }

#ifdef MEDIA_PLAYER
    if (SaagharWidget::musicPlayer) {
        SaagharWidget::musicPlayer->saveAllAlbums();
    }

    SaagharWidget::musicPlayer->savePlayerSettings();
#endif

    VAR_DECL("SaagharWidget/PoemViewStyle", SaagharWidget::CurrentViewStyle);

    VAR_DECL("SaagharWidget/MaxPoetsPerGroup", SaagharWidget::maxPoetsPerGroup);

    VAR_DECL("SaagharWidget/BackgroundState", SaagharWidget::backgroundImageState);
    VAR_DECL("SaagharWidget/BackgroundPath", SaagharWidget::backgroundImagePath);

    VAR_DECL("SaagharWidget/ShowBeytNumbers", SaagharWidget::showBeytNumbers);

    //colors
    VAR_DECL("SaagharWidget/Colors/MatchedText", SaagharWidget::matchedTextColor);
    VAR_DECL("SaagharWidget/Colors/Background", SaagharWidget::backgroundColor);

    ///////////////////////////////////////////////////
    /////////////////////save state////////////////////
    VAR_DECL("SaagharWindow/State0", saveState(0));
    VAR_DECL("SaagharWindow/Geometry", saveGeometry());

    QStringList openedTabs;
    for (int i = 0; i < mainTabWidget->count(); ++i) {
        SaagharWidget* tmp = getSaagharWidget(i);
        if (tmp) {
            QString tabViewType;
            if (tmp->currentPoem > 0) {
                tabViewType = "PoemID=" + QString::number(tmp->currentPoem);
            }
            else {
                tabViewType = "CatID=" + QString::number(tmp->currentCat);
            }

            openedTabs << tabViewType;
        }
    }
    VAR_DECL("SaagharWindow/LastSessionTabs", openedTabs);

    //database path
    VAR_DECL("DatabaseBrowser/DataBasePath", QDir::toNativeSeparators(sApp->defaultPath(SaagharApplication::DatabaseDirs)));

    //search options
    VAR_DECL("Search/MaxResultsPerPage", SearchResultWidget::maxItemPerPage);
    VAR_DECL("Search/NonPagedResults", SearchResultWidget::nonPagedSearch);
    VAR_DECL("Search/SkipVowelSigns", SearchResultWidget::skipVowelSigns);
    VAR_DECL("Search/SkipVowelLetters", SearchResultWidget::skipVowelLetters);

    QList<QListWidgetItem*> selectedItems = selectSearchRange->getSelectedItemList();

    QStringList selectedSearchRange;
    foreach (QListWidgetItem* item, selectedItems) {
        selectedSearchRange << item->data(Qt::UserRole).toString();
    }

    VAR_DECL("Search/SelectedRange", selectedSearchRange);
    VAR_DECL("SaagharWindow/RandomOpenNewTab", selectedSearchRange);

    VAR_DECL("DatabaseBrowser/RepositoriesList", DataBaseUpdater::repositories());
    VAR_DECL("DatabaseBrowser/KeepDownloadedFile", QVariant(DataBaseUpdater::keepDownloadedFiles));
    VAR_DECL("DatabaseBrowser/DownloadLocation", DataBaseUpdater::downloadLocation);
}

QString SaagharWindow::tableToString(QTableWidget* table, QString mesraSeparator, QString beytSeparator, int startRow, int startColumn, int endRow, int endColumn)
{
    QString tableAsString = "";
    for (int row = startRow; row < endRow ; ++row) {
        bool secondColumn = true;
        for (int col = startColumn; col < endColumn ; ++col) {
            if (col == 0) {
                continue;
            }
            QTableWidgetItem* currentItem = table->item(row , col);
            QString text = "";
            if (currentItem) {
                text = currentItem->text();
                if (text.isEmpty()) {
                    //maybe cell has QTextEdit as its widget!
                    QTextEdit* textEdit = qobject_cast<QTextEdit*>(table->cellWidget(row, col));
                    if (textEdit) {
                        text = textEdit->toPlainText();
                    }
                }
            }

            tableAsString.append(text);
            if (secondColumn) {
                secondColumn = false;
                tableAsString.append(mesraSeparator);
            }
        }
        tableAsString.append(beytSeparator);
    }
    return tableAsString;
}

void SaagharWindow::copySelectedItems()
{
    if (!saagharWidget || !saagharWidget->tableViewWidget) {
        return;
    }
    QString selectedText = "";
    QList<QTableWidgetSelectionRange> selectedRanges = saagharWidget->tableViewWidget->selectedRanges();

    int row = 0;

    foreach (QTableWidgetSelectionRange selectedRange, selectedRanges) {
        if (selectedRange.rowCount() == 0 || selectedRange.columnCount() == 0) {
            continue;    //return;
        }

        for (int i = row; i < selectedRange.topRow(); ++i) {
            QTextEdit* textEdit = qobject_cast<QTextEdit*>(saagharWidget->tableViewWidget->cellWidget(i, 1));
            if (textEdit) {
                QString sel = textEdit->textCursor().selectedText();
                if (!sel.isEmpty()) {
                    selectedText += sel + "\n";
                }
            }
        }
        row = selectedRange.topRow() + 1;
        selectedText += tableToString(saagharWidget->tableViewWidget, "          "/*ten blank spaces-Mesra separator*/, "\n"/*Beyt separator*/, selectedRange.topRow(), selectedRange.leftColumn(), selectedRange.bottomRow() + 1, selectedRange.rightColumn() + 1);
    }

    QString test = selectedText;
    test.remove(" ");
    test.remove("\n");
    test.remove("\t");
    if (test.isEmpty()) {
        selectedText = "";
        for (int i = 0; i < saagharWidget->tableViewWidget->rowCount(); ++i) {
            QTextEdit* textEdit = qobject_cast<QTextEdit*>(saagharWidget->tableViewWidget->cellWidget(i, 1));
            if (textEdit) {
                QString sel = textEdit->textCursor().selectedText();
                if (!sel.isEmpty()) {
                    selectedText += sel + "\n";
                }
            }
        }
    }

    if (!selectedText.isEmpty()) {
        QApplication::clipboard()->setText(selectedText);
    }
}

//Navigation
void SaagharWindow::actionHomeClicked()
{
    if (!saagharWidget) {
        return;
    }
    saagharWidget->showHome();
}

void SaagharWindow::actionPreviousPoemClicked()
{
    if (!saagharWidget) {
        return;
    }
    saagharWidget->previousPoem();
}

void SaagharWindow::actionNextPoemClicked()
{
    if (!saagharWidget) {
        return;
    }
    saagharWidget->nextPoem();
}

//Tools
void SaagharWindow::actionGanjoorSiteClicked()
{
    if (!saagharWidget) {
        return;
    }
    if (saagharWidget->currentPageGanjoorUrl().isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("There is no equivalent page there at ganjoor website."), QMessageBox::Ok, QMessageBox::Ok);
    }
    else {
        QDesktopServices::openUrl(QString("http://saaghar.sourceforge.net/redirect.php?sender=Saaghar&section=ganjoornet&url=%1").arg(saagharWidget->currentPageGanjoorUrl()));
    }
}

void SaagharWindow::closeEvent(QCloseEvent* event)
{
    QFontDatabase::removeAllApplicationFonts();
    if (SaagharWidget::lineEditSearchText) {
        SaagharWidget::lineEditSearchText->searchStop();
    }

    ConcurrentTaskManager::instance()->finish();

    event->accept();
    sApp->quitSaaghar();
}

#if QT_VERSION >= 0x050000
void SaagharWindow::paintEvent(QPaintEvent* event)
{
    if (VARB("SaagharWindow/UseTransparecy") && QtWin::isCompositionEnabled()) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(event->rect(), QColor(0, 0, 0, 0));
    }

    QMainWindow::paintEvent(event);
}
#endif

void SaagharWindow::tableItemMouseOver(QTableWidgetItem* item)
{
    if (!item) {
        return;
    }
    QTableWidget* senderTable = item->tableWidget();
    if (!saagharWidget || !senderTable) {
        return;
    }

    QVariant itemData = item->data(Qt::UserRole);

    if (itemData.isValid() && !itemData.isNull()) {
        QString dataString = itemData.toString();
        if (dataString.startsWith("CatID") || dataString.startsWith("PoemID")) {
            senderTable->setCursor(QCursor(Qt::PointingHandCursor));

            if (!(item->flags() & Qt::ItemIsSelectable)) {
                int pos = senderTable->verticalScrollBar()->value();
                senderTable->setCurrentItem(item);
                senderTable->verticalScrollBar()->setValue(pos);
                QImage image(ICON_PATH + "/select-mask.png");
                item->setBackground(QBrush(image.scaledToHeight(senderTable->rowHeight(item->row()))));
            }
            else {
                QImage image(ICON_PATH + "/select-mask.png");
                item->setBackground(QBrush(image.scaledToHeight(senderTable->rowHeight(item->row()))));
            }
        }
        else {
            senderTable->unsetCursor();
        }
    }
    else {
        senderTable->unsetCursor();
    }

    if (SaagharWidget::lastOveredItem && SaagharWidget::lastOveredItem != item /*&& (item->flags() & Qt::ItemIsSelectable)*/) {
        SaagharWidget::lastOveredItem->setBackground(QBrush(QImage()));//unset background
    }

    SaagharWidget::lastOveredItem = item;
}

void SaagharWindow::tableCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
    if (previous) {
        previous->setBackground(QBrush(QImage()));
    }
    if (current) { //maybe create a bug! check older codes!!
        QImage image(ICON_PATH + "/select-mask.png");
        current->setBackground(QBrush(image));
        //TODO: pageup and page down miss one row!! Qt-4.7.3
        if (saagharWidget && (saagharWidget->currentPoem > 0 || saagharWidget->currentCat > 0)) { //everywhere but home
            //Fixed BUG: sometimes saagharWidget->tableViewWidget != current->tableWidget()
            current->tableWidget()->scrollToItem(current);
        }
    }
}

void SaagharWindow::tableItemPress(QTableWidgetItem* item)
{
    Q_UNUSED(item)
    pressedMouseButton = QApplication::mouseButtons();
}

void SaagharWindow::tableItemClick(QTableWidgetItem* item)
{
    QTableWidget* senderTable = item->tableWidget();
    if (!senderTable) {
        return;
    }

    disconnect(senderTable, SIGNAL(itemClicked(QTableWidgetItem*)), 0, 0);
    QStringList itemData = item->data(Qt::UserRole).toString().split("=", QString::SkipEmptyParts);

    if (itemData.size() == 2 && itemData.at(0) == "VerseData") {
        QStringList verseData = itemData.at(1).split("|", QString::SkipEmptyParts);
        if (verseData.size() == 3) {
            emit verseSelected(verseData.at(1).toInt());
        }
    }

    if (itemData.size() != 2 || itemData.at(0) == "VerseData") {
        connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(tableItemClick(QTableWidgetItem*)));
        return;
    }

    //search data
    QStringList searchDataList = item->data(ITEM_SEARCH_DATA).toStringList();
    QString searchPhraseData;
    QString searchVerseData;
    if (searchDataList.size() == 2) {
        searchPhraseData = searchDataList.at(0);
        searchVerseData = searchDataList.at(1);
    }

    if (searchPhraseData.isEmpty()) {
        searchPhraseData = SaagharWidget::lineEditSearchText->text();
    }

    bool OK = false;
    int idData = itemData.at(1).toInt(&OK);
    bool noError = false;

    if (OK && sApp->databaseBrowser()) {
        noError = true;
    }

    //right click event
    if (pressedMouseButton == Qt::RightButton) {
        skipContextMenu = true;
        newTabForItem(idData, itemData.at(0), noError);

        saagharWidget->scrollToFirstItemContains(searchVerseData, false);
        SaagharItemDelegate* searchDelegate = new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), searchPhraseData);
        saagharWidget->tableViewWidget->setItemDelegate(searchDelegate);
        saagharWidget->scrollToFirstItemContains(searchVerseData, false);
        connect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), searchDelegate, SLOT(keywordChanged(QString)));

        connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(tableItemClick(QTableWidgetItem*)));
        return;
    }

    SaagharWidget::PageType pageType = SaagharWidget::CategoryViewerPage;
    if (itemData.at(0) == "PoemID") {
        pageType = SaagharWidget::PoemViewerPage;
    }

    if (!saagharWidget ||
            senderTable->objectName() != "searchTable" || //when clicked on searchTable's item we don't expect a refresh!
            saagharWidget->isDirty() ||
            saagharWidget->pageMetaInfo.id != idData ||
            saagharWidget->pageMetaInfo.type != pageType) {
        if (!saagharWidget) {
            insertNewTab(SaagharWindow::SaagharViewerTab, QString(), idData, itemData.at(0), noError);
        }
        else {
            saagharWidget->processClickedItem(itemData.at(0), idData, noError);
        }
    }

    saagharWidget->scrollToFirstItemContains(searchVerseData, false);
    SaagharItemDelegate* searchDelegate = new SaagharItemDelegate(saagharWidget->tableViewWidget, saagharWidget->tableViewWidget->style(), searchPhraseData);
    saagharWidget->tableViewWidget->setItemDelegate(searchDelegate);
    connect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), searchDelegate, SLOT(keywordChanged(QString)));

    // resolved by signal SaagharWidget::captionChanged()
    //updateTabsSubMenus();
    connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(tableItemClick(QTableWidgetItem*)));
}

void SaagharWindow::highlightTextOnPoem(int poemId, int vorder)
{
    if (saagharWidget->currentPoem != poemId) {
        return;
    }

    QString highlightedText = saagharWidget->highlightCell(vorder);
    emit highlightedTextChanged(highlightedText);
}

void SaagharWindow::openPath(const QString &path)
{
    QString SEPARATOR = QLatin1String("/");
    bool ok;
    QModelIndex ind = sApp->outlineModel()->index(path.split(SEPARATOR, QString::SkipEmptyParts), &ok);

    if (ok) {
        openPage(ind.data(OutlineModel::IDRole).toInt(), SaagharWidget::CategoryViewerPage);
    }
}

void SaagharWindow::showSearchOptionsDialog()
{
    SearchOptionsDialog searchDialog(this);
    connect(&searchDialog, SIGNAL(resultsRefreshRequired()), this, SIGNAL(maxItemPerPageChanged()));
    searchDialog.exec();
}

void SaagharWindow::toolBarContextMenu(const QPoint &/*pos*/)
{
    QMenu* contextMenu = new QMenu(0);
    toolBarViewActions(ui->mainToolBar, contextMenu, false);

    QAction* customizeRandom = 0;
    if (sApp->mainToolBarItems().contains("actionFaal", Qt::CaseInsensitive) || sApp->mainToolBarItems().contains("actionRandom", Qt::CaseInsensitive)) {
        contextMenu->addSeparator();
        customizeRandom = contextMenu->addAction(tr("Customize Faal && Random...")/*, customizeRandomButtons()*/);
    }

    if (contextMenu->exec(QCursor::pos()) == customizeRandom) {
        customizeRandomDialog();
    }

    delete contextMenu;
}

void SaagharWindow::customizeRandomDialog()
{
    bool openInNewTab = VARB("SaagharWindow/RandomOpenNewTab");

    CustomizeRandomDialog* randomSetting = new CustomizeRandomDialog(this, openInNewTab);

    QListWidgetItem* firstItem = randomSetting->selectRandomRange->insertRow(0, tr("Current tab's subsections"), true, "CURRENT_TAB_SUBSECTIONS", Qt::UserRole, true);
    QStringList selectRandomRange = VAR("SaagharWindow/SelectedRandomRange").toStringList();

    multiSelectObjectInitialize(randomSetting->selectRandomRange, selectRandomRange, 1);
    firstItem->setCheckState(selectRandomRange.contains("CURRENT_TAB_SUBSECTIONS") ? Qt::Checked : Qt::Unchecked);

    if (randomSetting->exec()) {
        randomSetting->acceptSettings(&openInNewTab);
        QList<QListWidgetItem*> selectedItems = randomSetting->selectRandomRange->getSelectedItemList();

        selectRandomRange.clear();
        foreach (QListWidgetItem* item, selectedItems) {
            selectRandomRange << item->data(Qt::UserRole).toString();
        }

        VAR_DECL("SaagharWindow/SelectedRandomRange", selectRandomRange);
        VAR_DECL("SaagharWindow/RandomOpenNewTab", openInNewTab);
    }

    delete randomSetting;
    randomSetting = 0;
}

void SaagharWindow::toolBarViewActions(QToolBar* toolBar, QMenu* menu, bool subMenu)
{
    QMenu* toolbarMenu = menu;
    if (subMenu) {
        toolbarMenu = new QMenu(toolBar->windowTitle() , menu);
        menu->addMenu(toolbarMenu);
    }

    toolbarMenu->addAction(actionInstance("actionToolBarSizeLargeIcon", "", QObject::tr("&Large Icon")));
    QActionGroup* toolBarIconSize = actionInstance("actionToolBarSizeLargeIcon")->actionGroup();
    if (!toolBarIconSize) {
        toolBarIconSize = new QActionGroup(this);
    }
    const int normalSize = toolBar->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    const int largeSize = toolBar->style()->pixelMetric(QStyle::PM_LargeIconSize);
    const int smallSize = toolBar->style()->pixelMetric(QStyle::PM_SmallIconSize);

    actionInstance("actionToolBarSizeLargeIcon")->setParent(toolBar);
    actionInstance("actionToolBarSizeLargeIcon")->setActionGroup(toolBarIconSize);
    actionInstance("actionToolBarSizeLargeIcon")->setCheckable(true);
    actionInstance("actionToolBarSizeLargeIcon")->setData(largeSize);

    toolbarMenu->addAction(actionInstance("actionToolBarSizeMediumIcon", "", QObject::tr("&Medium Icon")));
    actionInstance("actionToolBarSizeMediumIcon")->setParent(toolBar);
    actionInstance("actionToolBarSizeMediumIcon")->setActionGroup(toolBarIconSize);
    actionInstance("actionToolBarSizeMediumIcon")->setCheckable(true);
    actionInstance("actionToolBarSizeMediumIcon")->setData(normalSize);

    toolbarMenu->addAction(actionInstance("actionToolBarSizeSmallIcon", "", QObject::tr("&Small Icon")));
    actionInstance("actionToolBarSizeSmallIcon")->setParent(toolBar);
    actionInstance("actionToolBarSizeSmallIcon")->setActionGroup(toolBarIconSize);
    actionInstance("actionToolBarSizeSmallIcon")->setCheckable(true);
    actionInstance("actionToolBarSizeSmallIcon")->setData(smallSize);//qMax(normalSize-16, normalSize/2) );//qMin(normalSize, 16) == 16 ? normalSize-16 : normalSize/2 );//we sure we don't set it to zero!

    toolbarMenu->addSeparator();

    toolbarMenu->addAction(actionInstance("actionToolBarStyleOnlyIcon", "", QObject::tr("Only &Icon")));
    QActionGroup* toolBarButtonStyle = actionInstance("actionToolBarStyleOnlyIcon")->actionGroup();
    if (!toolBarButtonStyle) {
        toolBarButtonStyle = new QActionGroup(this);
    }
    actionInstance("actionToolBarStyleOnlyIcon")->setParent(toolBar);
    actionInstance("actionToolBarStyleOnlyIcon")->setActionGroup(toolBarButtonStyle);
    actionInstance("actionToolBarStyleOnlyIcon")->setCheckable(true);
    actionInstance("actionToolBarStyleOnlyIcon")->setData(Qt::ToolButtonIconOnly);

    toolbarMenu->addAction(actionInstance("actionToolBarStyleOnlyText", "", QObject::tr("Only &Text")));
    actionInstance("actionToolBarStyleOnlyText")->setParent(toolBar);
    actionInstance("actionToolBarStyleOnlyText")->setActionGroup(toolBarButtonStyle);
    actionInstance("actionToolBarStyleOnlyText")->setCheckable(true);
    actionInstance("actionToolBarStyleOnlyText")->setData(Qt::ToolButtonTextOnly);

    toolbarMenu->addAction(actionInstance("actionToolBarStyleTextIcon", "", QObject::tr("&Both Text && Icon")));
    actionInstance("actionToolBarStyleTextIcon")->setParent(toolBar);
    actionInstance("actionToolBarStyleTextIcon")->setActionGroup(toolBarButtonStyle);
    actionInstance("actionToolBarStyleTextIcon")->setCheckable(true);
    actionInstance("actionToolBarStyleTextIcon")->setData(Qt::ToolButtonTextUnderIcon);

    //checked actions
    actionInstance(VARS("SaagharWindow/MainToolBarStyle"))->setChecked(true);
    actionInstance(VARS("SaagharWindow/MainToolBarSize"))->setChecked(true);

    //create actiongroups connections
    connect(toolBarIconSize, SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));
    connect(toolBarButtonStyle, SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));
}

void SaagharWindow::toolbarViewChanges(QAction* action)
{
    if (!action) {
        return;
    }
    QToolBar* toolbar = qobject_cast<QToolBar*>(action->parent());
    if (!toolbar) {
        return;
    }

    bool ok = false;
    int data = action->data().toInt(&ok);
    if (!ok) {
        return;
    }

    disconnect(action->actionGroup(), SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));

    QString actionId = allActionMap.key(action);
    QString actionType = actionId.contains("Size") ? "SizeType" : "StyleType";

    if (actionType == "SizeType") {
        toolbar->setIconSize(QSize(data, data));
        VAR_DECL("SaagharWindow/MainToolBarSize", actionId);
    }
    else if (actionType == "StyleType") {
        toolbar->setToolButtonStyle((Qt::ToolButtonStyle)data);
        VAR_DECL("SaagharWindow/MainToolBarStyle", actionId);
    }

    connect(action->actionGroup(), SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));
}

bool SaagharWindow::eventFilter(QObject* receiver, QEvent* event)
{
    QEvent::Type eventType = QEvent::None;
    if (event) {
        eventType = event->type();
    }

    switch (eventType) {
    case QEvent::Resize :
    case QEvent::None : {
        if (saagharWidget &&
                saagharWidget->tableViewWidget &&
                receiver == saagharWidget->tableViewWidget->viewport()) {
            saagharWidget->resizeTable(saagharWidget->tableViewWidget);
        }
    }
    break;
    case QEvent::Move : {
        QMoveEvent* resEvent = static_cast<QMoveEvent*>(event);
        if (resEvent && receiver == ui->searchToolBar) {
            if (searchToolBarBoxLayout->direction() != QBoxLayout::LeftToRight && ui->searchToolBar->isFloating()) {
                searchToolBarBoxLayout->setDirection(QBoxLayout::LeftToRight);
                searchToolBarBoxLayout->setSpacing(4);
                searchToolBarBoxLayout->setContentsMargins(1, 1, 1, 1);
                ui->searchToolBar->adjustSize();
            }
            int height = comboBoxSearchRegion->height() + SaagharWidget::lineEditSearchText->height() + 1 + 1 + 2; //margins and spacing
            if (ui->searchToolBar->size().height() >= height && !ui->searchToolBar->isFloating()) {
                searchToolBarBoxLayout->setDirection(QBoxLayout::TopToBottom);
                searchToolBarBoxLayout->setSpacing(2);
                searchToolBarBoxLayout->setContentsMargins(1, 1, 1, 1);
            }
//              else
//              {
//                  searchToolBarBoxLayout->setDirection(QBoxLayout::RightToLeft);
//              }
        }
    }
    break;
    default:
        break;
    }
    skipSearchToolBarResize = false;
    return false;
}

void SaagharWindow::setupSearchToolBarUi()
{
    //initialize Search ToolBar
    QString clearIconPath = ICON_PATH + "/clear-left.png";
    if (layoutDirection() == Qt::RightToLeft) {
        clearIconPath = ICON_PATH + "/clear-right.png";
    }
    SaagharWidget::lineEditSearchText = new QSearchLineEdit(ui->searchToolBar, clearIconPath, ICON_PATH + "/search-options.png", ICON_PATH + "/cancel.png");
    SaagharWidget::lineEditSearchText->setObjectName(QString::fromUtf8("lineEditSearchText"));
    SaagharWidget::lineEditSearchText->setMaximumSize(QSize(170, 16777215));
    SaagharWidget::lineEditSearchText->setLayoutDirection(Qt::RightToLeft);

    selectSearchRange = new QMultiSelectWidget(ui->searchToolBar);
    comboBoxSearchRegion = selectSearchRange->getComboWidgetInstance();
    comboBoxSearchRegion->setObjectName(QString::fromUtf8("comboBoxSearchRegion"));
    comboBoxSearchRegion->setLayoutDirection(Qt::RightToLeft);
    comboBoxSearchRegion->setMaximumSize(QSize(170, 16777215));
    comboBoxSearchRegion->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    comboBoxSearchRegion->setEditable(true);
#if QT_VERSION >= 0x040700
    SaagharWidget::lineEditSearchText->setPlaceholderText(tr("Enter Search Phrase"));
    comboBoxSearchRegion->lineEdit()->setPlaceholderText(tr("Select Search Scope..."));
#else
    SaagharWidget::lineEditSearchText->setToolTip(tr("Enter Search Phrase"));
    comboBoxSearchRegion->lineEdit()->setToolTip(tr("Select Search Scope..."));
#endif

    //create layout and add widgets to it!
    searchToolBarBoxLayout = new QBoxLayout(QBoxLayout::LeftToRight);

    searchToolBarBoxLayout->setSpacing(4);
    searchToolBarBoxLayout->setContentsMargins(1, 1, 1, 1);

    searchToolBarBoxLayout->addWidget(SaagharWidget::lineEditSearchText);
    searchToolBarBoxLayout->addWidget(comboBoxSearchRegion);
    QSpacerItem* horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    searchToolBarBoxLayout->addItem(horizontalSpacer);

    QWidget* searchToolBarContent = new QWidget();

    QHBoxLayout* horizontalStretch = new QHBoxLayout;
    horizontalStretch->setSpacing(0);
    horizontalStretch->setContentsMargins(0, 0, 0, 0);
    horizontalStretch->addLayout(searchToolBarBoxLayout);
    horizontalStretch->addStretch(10000);
    searchToolBarContent->setLayout(horizontalStretch);

    searchToolBarContent->setFocusProxy(SaagharWidget::lineEditSearchText);
    ui->searchToolBar->addWidget(searchToolBarContent);

    searchOptionMenu = new QMenu(ui->searchToolBar);

    actionInstance("searchOptions", "", tr("Search &Options..."));
    connect(actionInstance("searchOptions"), SIGNAL(triggered()), this, SLOT(showSearchOptionsDialog()));

    actionInstance("actionSearchTips", "", tr("&Search Tips..."));
    connect(actionInstance("actionSearchTips"), SIGNAL(triggered()), this, SLOT(showSearchTips()));

    searchOptionMenu->addAction(actionInstance("searchOptions"));
    searchOptionMenu->addAction(actionInstance("separator"));
    searchOptionMenu->addAction(actionInstance("actionSearchTips"));

    connect(SaagharWidget::lineEditSearchText->optionsButton(), SIGNAL(clicked()), this, SLOT(showSearchOptionMenu()));
}

void SaagharWindow::namedActionTriggered(bool checked)
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }
    disconnect(action, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));

    QString actionName = action->objectName();
    QString text = action->text();
    text.remove("&");
    if (actionName == "SaagharWindow/ShowPhotoAtHome") {
        VAR_DECL("SaagharWindow/ShowPhotoAtHome", checked);
        //TODO: reload Home page!!!!!!!
        setHomeAsDirty();
    }
    else if (actionName == "menuOpenedTabsActions") {
        QObject* obj = qvariant_cast<QObject*>(action->data());
        QWidget* tabWidget = qobject_cast<QWidget*>(obj);
        if (tabWidget) {
            mainTabWidget->setCurrentWidget(tabWidget);
        }
        return;//action is deleted by 'updateTabsSubMenus()'
    }
    else if (actionName == "SaagharWindow/LockToolBars") {
        VAR_DECL("SaagharWindow/LockToolBars", checked);
        ui->mainToolBar->setMovable(!checked);
        if (ui->menuToolBar) {
            ui->menuToolBar->setMovable(!checked);
        }
        ui->searchToolBar->setMovable(!checked);
        parentCatsToolBar->setMovable(!checked);
        m_breadCrumbToolBar->setMovable(!checked);
#ifdef MEDIA_PLAYER
        SaagharWidget::musicPlayer->setMovable(!checked);
#endif
    }
    else if (actionName == "Ganjoor Verification") {
        QDesktopServices::openUrl(QString("http://saaghar.sourceforge.net/redirect.php?sender=Saaghar&section=ganjoor-vocr&url=%1").arg("http://v.ganjoor.net"));
    }
    else if (actionName == "fixedNameRedoAction" ||
             actionName == "fixedNameUndoAction" ||
             actionName == "globalRedoAction" ||
             actionName == "globalUndoAction") {
        if (actionName == "fixedNameRedoAction") {
            emit globalRedoAction->activate(QAction::Trigger);
        }
        if (actionName == "fixedNameUndoAction") {
            emit globalUndoAction->activate(QAction::Trigger);
        }

        actionInstance("fixedNameRedoAction")->setEnabled(globalRedoAction->isEnabled());
        actionInstance("fixedNameUndoAction")->setEnabled(globalUndoAction->isEnabled());
    }
    else if (actionName == "ImportGanjoorBookmarks") {
        if (SaagharWidget::bookmarks) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            SaagharWidget::bookmarks->insertBookmarkList(sApp->databaseBrowser()->importGanjoorBookmarks());
            QApplication::restoreOverrideCursor();
        }
    }
    else if (actionName.endsWith("PoemViewStyle")) {
        SaagharWidget::PoemViewStyle newPoemViewStyle = (SaagharWidget::PoemViewStyle)action->data().toInt();
        if (SaagharWidget::CurrentViewStyle != newPoemViewStyle) {
            SaagharWidget::CurrentViewStyle = newPoemViewStyle;

            for (int i = 0; i < mainTabWidget->count(); ++i) {
                SaagharWidget* tmp = getSaagharWidget(i);
                if (tmp && tmp->currentPoem != 0) {
                    tmp->setDirty();    //needs to be refreshed
                }
            }
            if (saagharWidget && saagharWidget->currentPoem != 0) {
                saagharWidget->refresh();
            }
        }
    }
    else if (actionName == "DownloadRepositories") {
        if (!DatabaseBrowser::dbUpdater) {
            DatabaseBrowser::dbUpdater = new DataBaseUpdater(this);
        }

        QtWin::easyBlurUnBlur(DatabaseBrowser::dbUpdater, VARB("SaagharWindow/UseTransparecy"));

        DatabaseBrowser::dbUpdater->exec();
    }
    else if (actionName == "actionFaal") {
        openRandomPoem(24, VARB("SaagharWindow/RandomOpenNewTab"));// '24' is for Hafez!
    }
    else if (actionName == "actionRandom") {
        int id = 0;
        const QStringList selectRandomRange = VAR("SaagharWindow/SelectedRandomRange").toStringList();

        if (selectRandomRange.contains("CURRENT_TAB_SUBSECTIONS")) {
            if (saagharWidget) {
                if (saagharWidget->currentPoem != 0) {
                    id = sApp->databaseBrowser()->getPoem(saagharWidget->currentPoem)._CatID;
                }
                else {
                    id = saagharWidget->currentCat;
                }
            }
            else {
                id = 0;
            }
        }
        else if (selectRandomRange.contains("0")) { //all
            id = 0;    //home
        }
        else {
            if (selectRandomRange.isEmpty()) {
                id = 0;    //all
            }
            else {
                int randIndex = Tools::getRandomNumber(0, selectRandomRange.size() - 1);
                id = sApp->databaseBrowser()->getPoet(selectRandomRange.at(randIndex).toInt())._CatID;
            }
        }

        openRandomPoem(id, VARB("SaagharWindow/RandomOpenNewTab"));
    }
    else if (actionName == "Registeration") {
#if 0
        QWidget* regForm = qvariant_cast<QWidget*>(actionInstance("Registeration")->data());
        if (!regForm) {
            checkRegistration(true);
        }
        else {
            mainTabWidget->setCurrentWidget(regForm);
        }
#endif
    }
    else if (actionName == "searchToolbarAction") {
        if (action->isChecked()) {
            SaagharWidget::lineEditSearchText->setFocus();
        }
    }
#ifdef MEDIA_PLAYER
    else if (actionName == "albumDockAction") {
        if (action->isChecked()) {
            SaagharWidget::musicPlayer->albumManagerDock()->show();
            SaagharWidget::musicPlayer->albumManagerDock()->raise();
        }
    }
#endif
    else if (actionName == "outlineDockAction") {
        if (action->isChecked()) {
            m_outlineDock->show();
            m_outlineDock->raise();
        }
    }
    else if (actionName == "bookmarkManagerDockAction" ||
             actionName == "copyOfBookmarkManagerDockAction") {
        m_bookmarkManagerDock->toggleViewAction()->setChecked(action->isChecked());
        allActionMap.value("bookmarkManagerDockAction")->setChecked(action->isChecked());
        if (action->isChecked()) {
            m_bookmarkManagerDock->show();
            m_bookmarkManagerDock->raise();
        }
        else {
            m_bookmarkManagerDock->hide();
        }
    }

    connect(action, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));
}

void SaagharWindow::updateTabsSubMenus()
{
    menuOpenedTabs->clear();
    //menuOpenedTabs->setTearOffEnabled(true);
    QAction* tabAction;
    int numOfTabs = mainTabWidget->count();
    for (int i = 0; i < numOfTabs; ++i) {
        tabAction = new QAction(mainTabWidget->tabText(i), menuOpenedTabs);

        if (i == mainTabWidget->currentIndex()) {
            tabAction->setIcon(QIcon(ICON_PATH + "/right.png"));
        }
        QObject* obj = mainTabWidget->widget(i);
        QVariant data = QVariant::fromValue(obj);
        tabAction->setData(data);

        tabAction->setObjectName("menuOpenedTabsActions");
        connect(tabAction, SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));
        menuOpenedTabs->addAction(tabAction);
    }
}

void SaagharWindow::actionClosedTabsClicked()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }
    //QString tabType = action->data().toString();
    QString type = action->data().toStringList().at(0);
    int id = action->data().toStringList().at(1).toInt();

    if ((type != "PoemID" && type != "CatID") || id < 0) {
        newTabForItem(id, type, false);
    }
    else {
        newTabForItem(id, type, true);
    }

    menuClosedTabs->removeAction(action);
}

void SaagharWindow::setHomeAsDirty()
{
    setAsDirty(0, -1);
}

void SaagharWindow::setAllAsDirty()
{
    setAsDirty(-1, -1);
}

void SaagharWindow::setAsDirty(int catId, int poemId)//-1 for skip it
{
    bool checkPoemId = false, checkCatId = false;
    if (catId != -1) {
        checkCatId = true;
    }
    if (poemId != -1) {
        checkPoemId = true;
    }

    for (int i = 0; i < mainTabWidget->count(); ++i) {
        SaagharWidget* tmp = getSaagharWidget(i);
        if (tmp &&
                (!checkPoemId || tmp->currentPoem == poemId)
                &&
                (!checkCatId || tmp->currentCat == catId)) {
            tmp->setDirty();//needs to be refreshed
        }
    }
    if (saagharWidget &&
            (!checkPoemId || saagharWidget->currentPoem == poemId)
            &&
            (!checkCatId || saagharWidget->currentCat == catId)) {
        saagharWidget->refresh();
    }
}

void SaagharWindow::setupBookmarkManagerUi()
{
    m_bookmarkManagerDock = new QDockWidget(tr("Bookmarks"), this);
    m_bookmarkManagerDock->setObjectName("bookMarkWidget");
    m_bookmarkManagerDock->setStyleSheet("QDockWidget::title { background: transparent; text-align: left; padding: 0 10 0 10;}"
                                         "QDockWidget::close-button, QDockWidget::float-button { background: transparent;}");
    m_bookmarkManagerDock->hide();

    QWidget* bookmarkContainer = new QWidget(m_bookmarkManagerDock);
    QVBoxLayout* bookmarkMainLayout = new QVBoxLayout;
    QHBoxLayout* bookmarkToolsLayout = new QHBoxLayout;

    SaagharWidget::bookmarks = new Bookmarks(this);

    connect(SaagharWidget::bookmarks, SIGNAL(showBookmarkedItem(QString,QString,QString,bool,bool)), this, SLOT(ensureVisibleBookmarkedItem(QString,QString,QString,bool,bool)));

    QString clearIconPath = ICON_PATH + "/clear-left.png";
    if (layoutDirection() == Qt::RightToLeft) {
        clearIconPath = ICON_PATH + "/clear-right.png";
    }

    QLabel* bookmarkFilterLabel = new QLabel(m_bookmarkManagerDock);
    bookmarkFilterLabel->setObjectName(QString::fromUtf8("bookmarkFilterLabel"));
    bookmarkFilterLabel->setText(tr("Filter:"));
    QSearchLineEdit* bookmarkFilter = new QSearchLineEdit(m_bookmarkManagerDock, clearIconPath, ICON_PATH + "/filter.png");
    bookmarkFilter->setObjectName("bookmarkFilter");
#if QT_VERSION >= 0x040700
    bookmarkFilter->setPlaceholderText(tr("Filter"));
#else
    bookmarkFilter->setToolTip(tr("Filter"));
#endif
    connect(bookmarkFilter, SIGNAL(textChanged(QString)), SaagharWidget::bookmarks, SLOT(filterItems(QString)));

    QToolButton* unBookmarkButton = new QToolButton(m_bookmarkManagerDock);
    unBookmarkButton->setObjectName("unBookmarkButton");
    unBookmarkButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

    unBookmarkButton->setIcon(QIcon(ICON_PATH + "/un-bookmark.png"));
    connect(unBookmarkButton, SIGNAL(clicked()), SaagharWidget::bookmarks, SLOT(unBookmarkItem()));

    QSpacerItem* filterHorizSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    bookmarkToolsLayout->addWidget(bookmarkFilterLabel, 0, Qt::AlignRight | Qt::AlignCenter);
    bookmarkToolsLayout->addWidget(bookmarkFilter);
    bookmarkToolsLayout->addItem(filterHorizSpacer);
    bookmarkToolsLayout->addWidget(unBookmarkButton);

    bookmarkMainLayout->addWidget(SaagharWidget::bookmarks);
    bookmarkMainLayout->addLayout(bookmarkToolsLayout);//move to bottom of layout! just we want looks similar other dock widgets!
    bookmarkContainer->setLayout(bookmarkMainLayout);

    QFile bookmarkFile(sApp->defaultPath(SaagharApplication::BookmarksFile));

    bool readBookmarkFile = true;

    if (!bookmarkFile.exists()) {
        //create an empty XBEL file.
        if (bookmarkFile.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream out(&bookmarkFile);
            out.setCodec("utf-8");
            out << "<?xml version='1.0' encoding='UTF-8'?>\n"
                <<  "<!DOCTYPE xbel>\n"
                <<  "<xbel version=\"1.0\">\n"
                <<  "</xbel>";
            bookmarkFile.close();
        }
    }

    if (!bookmarkFile.open(QFile::ReadOnly | QFile::Text) || !SaagharWidget::bookmarks->read(&bookmarkFile)) {
        readBookmarkFile = false;
    }

    if (readBookmarkFile) {
        m_bookmarkManagerDock->setWidget(bookmarkContainer /*SaagharWidget::bookmarks*/);
        addDockWidget(Qt::RightDockWidgetArea, m_bookmarkManagerDock);

        m_bookmarkManagerDock->toggleViewAction()->setIcon(QIcon(ICON_PATH + "/bookmark-folder.png"));
        m_bookmarkManagerDock->toggleViewAction()->setObjectName(QString::fromUtf8("copyOfBookmarkManagerDockAction"));
        connect(m_bookmarkManagerDock->toggleViewAction(), SIGNAL(triggered(bool)), this, SLOT(namedActionTriggered(bool)));

        // Fixed a bug (unity's global-menu): as a copy for toggleViewAction()
        //  that we'll add to panelsView submenu.
        actionInstance("bookmarkManagerDockAction")->setText(m_bookmarkManagerDock->toggleViewAction()->text());
        actionInstance("bookmarkManagerDockAction")->setCheckable(true);
        actionInstance("bookmarkManagerDockAction")->setChecked(m_bookmarkManagerDock->toggleViewAction()->isChecked());
        actionInstance("bookmarkManagerDockAction")->setIcon(QIcon(ICON_PATH + "/bookmark-folder.png"));
        actionInstance("bookmarkManagerDockAction")->setObjectName(QString::fromUtf8("bookmarkManagerDockAction"));
        connect(m_bookmarkManagerDock, SIGNAL(visibilityChanged(bool)), actionInstance("bookmarkManagerDockAction"), SLOT(setChecked(bool)));

        menuBookmarks = new QMenu(tr("&Bookmarks"), ui->menuBar);
        menuBookmarks->setObjectName(QString::fromUtf8("menuBookmarks"));
        menuBookmarks->addAction(m_bookmarkManagerDock->toggleViewAction());
        menuBookmarks->addSeparator();
        menuBookmarks->addAction(actionInstance("ImportGanjoorBookmarks", ICON_PATH + "/bookmarks-import.png", tr("&Import Ganjoor's Bookmarks")));
    }
    else {
        //bookmark not loaded!
        QStringList items = sApp->mainToolBarItems();
        items.removeAll("bookmarkManagerDockAction");
        items.removeAll("ImportGanjoorBookmarks");
        sApp->setMainToolBarItems(items);

        deleteActionInstance("bookmarkManagerDockAction");
        deleteActionInstance("ImportGanjoorBookmarks");
        allActionMap.insert("bookmarkManagerDockAction", 0);
        allActionMap.insert("ImportGanjoorBookmarks", 0);
        delete SaagharWidget::bookmarks;
        delete bookmarkContainer;
        bookmarkContainer = 0;
        SaagharWidget::bookmarks = 0;
        delete m_bookmarkManagerDock;
        m_bookmarkManagerDock = 0;
        menuBookmarks = 0;
        showStatusText("!QExtendedSplashScreenCommands:HIDE");
        QMessageBox warning(QMessageBox::Warning, tr("Warning!"), tr("Bookmarking system was disabled, something going wrong with "
                            "writing or reading from bookmarks file:\n%1")
                            .arg(sApp->defaultPath(SaagharApplication::BookmarksFile)), QMessageBox::Ok, this, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
        warning.exec();
        showStatusText("!QExtendedSplashScreenCommands:SHOW");
    }
}

void SaagharWindow::ensureVisibleBookmarkedItem(const QString &type, const QString &itemText, const QString &data, bool ensureVisible, bool unbookmark)
{
    if (type == "Verses" || type == tr("Verses")) {
        int poemID = data.split("|").at(0).toInt();
        //QString poemIdentifier = "PoemID="+data.split("|").at(0);
        for (int i = 0; i < mainTabWidget->count(); ++i) {
            SaagharWidget* tmp = getSaagharWidget(i);
            if (tmp) {
                if (tmp->identifier().at(0) == "PoemID" && tmp->identifier().at(1).toInt() == poemID) {
                    if (ensureVisible) {
                        mainTabWidget->setCurrentIndex(i);
                        saagharWidget->scrollToFirstItemContains(itemText, false, true);
                        return;
                    }

                    QTableWidgetItem* item = tmp->scrollToFirstItemContains(itemText, false, ensureVisible);
                    if (!ensureVisible) {
                        //un-bookmarking operation!!
                        if (item) {
                            QTableWidgetItem* numItem = tmp->tableViewWidget->item(item->row(), 0);
                            if (SaagharWidget::bookmarks && numItem) {
                                QPixmap star(ICON_PATH + "/bookmark-on.png");
                                QPixmap starOff(ICON_PATH + "/bookmark-off.png");
                                star = star.scaledToHeight(qMin(tmp->tableViewWidget->rowHeight(item->row()) - 1, 22), Qt::SmoothTransformation);
                                starOff = starOff.scaledToHeight(qMin(tmp->tableViewWidget->rowHeight(item->row()) - 1, 22), Qt::SmoothTransformation);
                                QIcon bookmarkIcon;
                                bookmarkIcon.addPixmap(star, QIcon::Active, QIcon::On);
                                bookmarkIcon.addPixmap(starOff, QIcon::Disabled, QIcon::Off);

                                if (!unbookmark) {
                                    bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Active, QIcon::On);
                                }
                                else {
                                    bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Disabled, QIcon::Off);
                                }

                                numItem->setIcon(bookmarkIcon);
                                const int ITEM_BOOKMARKED_STATE = Qt::UserRole + 20;
                                numItem->setData(ITEM_BOOKMARKED_STATE, !unbookmark);
                            }
                        }
                    }
                }
            }
        }

        if (ensureVisible) {
            //newTabForItem(poemID, "PoemID", true);
            saagharWidget->processClickedItem("PoemID", poemID, true);
            saagharWidget->scrollToFirstItemContains(itemText, false, true);
        }
    }
}

#ifdef MEDIA_PLAYER
void SaagharWindow::mediaInfoChanged(const QString &fileName, const QString &title, int id)
{
    if (!saagharWidget) {
        return;
    }

    QString mediaTitle = title;

    qint64 time = 0;
    if (SaagharWidget::musicPlayer) {
        time = SaagharWidget::musicPlayer->currentTime();
    }
    if (mediaTitle.isEmpty()) {
        mediaTitle = saagharWidget->currentLocationList.join(">");
        if (!saagharWidget->currentPoemTitle.isEmpty()) {
            mediaTitle += ">" + saagharWidget->currentPoemTitle;
        }
    }
    if (SaagharWidget::musicPlayer) {
        SaagharWidget::musicPlayer->insertToAlbum(id, fileName, mediaTitle, time);
    }
}
#endif

void SaagharWindow::createCustomContextMenu(const QPoint &pos)
{
    if (!saagharWidget || !saagharWidget->tableViewWidget) {
        return;
    }
    if (skipContextMenu) {
        skipContextMenu = false;
        return;
    }

    QTableWidgetItem* item = saagharWidget->tableViewWidget->itemAt(pos);

    QString cellText = "";
    if (item) {
        cellText = item->text();
        if (cellText.isEmpty()) {
            QTextEdit* textEdit = qobject_cast<QTextEdit*>(saagharWidget->tableViewWidget->cellWidget(item->row(), item->column()));
            if (textEdit) {
                cellText = textEdit->toPlainText();
            }
        }
    }

    QMenu* contextMenu = new QMenu;
    contextMenu->addAction(tr("Copy Selected Text"));
    contextMenu->addAction(tr("Copy Cell\'s Text"));
    contextMenu->addAction(tr("Copy All"));
    contextMenu->addSeparator();
    contextMenu->addAction(tr("New Tab"));
    contextMenu->addAction(tr("Duplicate Tab"));
    contextMenu->addAction(tr("Refresh"));

    QAction* action = contextMenu->exec(QCursor::pos());
    if (!action) {
        return;
    }

    QString text = action->text();
    text.remove("&");

    if (text == tr("Copy Selected Text")) {
        copySelectedItems();
    }
    else if (text == tr("Copy Cell\'s Text")) {
        if (!cellText.isEmpty()) {
            QApplication::clipboard()->setText(cellText);
        }
    }
    else if (text == tr("Copy All")) {
        QString tableText = tableToString(saagharWidget->tableViewWidget, "          "/*ten blank spaces-Mesra separator*/, "\n"/*Beyt separator*/, 0, 1, saagharWidget->tableViewWidget->rowCount(), saagharWidget->tableViewWidget->columnCount());
        QApplication::clipboard()->setText(tableText);
    }
    else if (text == tr("New Tab")) {
        insertNewTab();
    }
    else if (text == tr("Duplicate Tab")) {
        newTabForItem(saagharWidget->pageMetaInfo.id, saagharWidget->pageMetaInfo.type == SaagharWidget::PoemViewerPage ? "PoemID" : "CatID", true);
    }
    else if (text == tr("Refresh")) {
        saagharWidget->refresh();
    }
}

void SaagharWindow::onCurrentLocationChanged(const QStringList &locationList)
{
    const QString SEPARATOR = QLatin1String("/");

    m_breadCrumbBar->blockSignals(true);
    m_breadCrumbBar->setLocation(locationList.join(SEPARATOR));
    m_breadCrumbBar->blockSignals(false);
}

void SaagharWindow::openParentPage(int parentID, bool newPage)
{
    openPage(parentID, SaagharWidget::CategoryViewerPage, newPage);
}

void SaagharWindow::openChildPage(int childID, bool newPage)
{
    openPage(childID, SaagharWidget::PoemViewerPage, newPage);
}

void SaagharWindow::openPage(int id, SaagharWidget::PageType type, bool newPage)
{
    if (!newPage) {
        for (int j = 0; j < mainTabWidget->count(); ++j) {
            SaagharWidget* tmp = getSaagharWidget(j);

            if (tmp && tmp->pageMetaInfo.type == type && tmp->pageMetaInfo.id == id) {
                mainTabWidget->setCurrentWidget(tmp->parentWidget());
                loadAudioForCurrentTab();
                return;
            }
        }
    }

    QString typeSTR = (type == SaagharWidget::CategoryViewerPage ? "CatID" : "PoemID");
    if (!saagharWidget || newPage) {
        insertNewTab(SaagharWindow::SaagharViewerTab, QString(), id, typeSTR);
    }
    else {
        saagharWidget->processClickedItem(typeSTR, id, true);
    }
}

void SaagharWindow::checkRegistration(bool forceShow)
{
    Q_UNUSED(forceShow)
#if 0
    if (!forceShow && !RegisterationForm::showRegisterForm()) {
        return;
    }

    QWidget* formContainer = insertNewTab(SaagharWindow::WidgetTab, tr("Registeration"));
    if (!formContainer) {
        RegisterationForm* regForm = new RegisterationForm;
        regForm->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
        regForm->setAttribute(Qt::WA_DeleteOnClose, true);
        QtWin::easyBlurUnBlur(regForm, VARB("SaagharWindow/UseTransparecy"));
        regForm->show();
    }
    else {
        QGridLayout* gridLayout = new QGridLayout(formContainer);
        gridLayout->setObjectName(QString::fromUtf8("tabGridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        formContainer->setObjectName(QString::fromUtf8("WidgetTab-RegFormContainer"));

        RegisterationForm* regForm = new RegisterationForm(formContainer, mainTabWidget);
        regForm->setStyleSheet(regForm->styleSheet() + "QGroupBox {border: 1px solid lightgray; border-radius: 5px; margin-top: 7px; margin-bottom: 7px; padding: 0px;}QGroupBox::title {top: -7 ex;left: 10px; subcontrol-origin: border;}");
        regForm->setAutoFillBackground(true);
        QPalette p(regForm->palette());
        p.setColor(QPalette::Window, QColor(Qt::lightGray).lighter(130));
        p.setColor(QPalette::Base, QColor(Qt::white));
        regForm->setPalette(p);

        formContainer->setMinimumSize(regForm->sizeHint());
        gridLayout->addWidget(regForm, 1, 1, 1, 1, Qt::AlignCenter);

        QSpacerItem* leftSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        gridLayout->addItem(leftSpacer, 1, 0, 1, 1);
        QSpacerItem* bottomSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        gridLayout->addItem(bottomSpacer, 2, 1, 1, 1);
        QSpacerItem* rightSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        gridLayout->addItem(rightSpacer, 1, 2, 1, 1);
        QSpacerItem* topSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        gridLayout->addItem(topSpacer, 0, 1, 1, 1);

        formContainer->setLayout(gridLayout);
        formContainer->adjustSize();
        mainTabWidget->currentWidget()->setObjectName("WidgetTab-Registeration");
        actionInstance("Registeration")->setData(QVariant::fromValue(mainTabWidget->currentWidget()));
    }
#endif
}
