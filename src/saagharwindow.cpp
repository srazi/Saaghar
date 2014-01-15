/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2014 by S. Razi Alavizadeh                          *
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
#include "registerationform.h"
#include "searchoptionsdialog.h"

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
#include <QTranslator>

#ifndef NO_PHONON_LIB
#include "qmusicplayer.h"
#endif

//const int ITEM_SEARCH_DATA = Qt::UserRole+10;
const QString BUILD_TIME = __DATE__" "__TIME__;
const QString SAAGHAR_VERSION = "2.5.0";

QString SaagharWindow::resourcesPath;
QString SaagharWindow::userHomePath;
bool SaagharWindow::autoCheckForUpdatesState = true;
bool SaagharWindow::isPortable = false;

SaagharWindow::SaagharWindow(QWidget* parent, QExtendedSplashScreen* splashScreen)
    : QMainWindow(parent)
    , ui(new Ui::SaagharWindow)
    , m_cornerMenu(0)
    , menuBookmarks(0)
    , m_settingsDialog(0)
    , m_splash(splashScreen)
{
    setObjectName("SaagharMainWindow");

    QCoreApplication::setOrganizationName("Pozh");
    QCoreApplication::setApplicationName("Saaghar");
    QCoreApplication::setOrganizationDomain("Pozh.org");

    setWindowIcon(QIcon(":/resources/images/saaghar.png"));

#ifdef Q_OS_MAC
    QFileInfo portableSettings(QCoreApplication::applicationDirPath() + "/../Resources/settings.ini");
#else
    QFileInfo portableSettings(QCoreApplication::applicationDirPath() + "/settings.ini");
#endif

    if (portableSettings.exists() && portableSettings.isWritable()) {
        isPortable = true;
    }
    else {
        isPortable = false;
    }

    const QString tempDataBaseName = "/ganjoor.s3db";//temp
    QString dataBaseCompleteName = "/ganjoor.s3db";

    if (isPortable) {
#ifdef Q_OS_MAC
        dataBaseCompleteName = QCoreApplication::applicationDirPath() + "/../Resources/" + dataBaseCompleteName;
        resourcesPath = QCoreApplication::applicationDirPath() + "/../Resources/";
#else
        dataBaseCompleteName = QCoreApplication::applicationDirPath() + dataBaseCompleteName;
        resourcesPath = QCoreApplication::applicationDirPath();
#endif
        userHomePath = resourcesPath;
    }
    else {
#ifdef Q_OS_WIN
        //dataBaseCompleteName = QDir::homePath()+"/Pojh/Saaghar/"+dataBaseCompleteName;
        resourcesPath = QCoreApplication::applicationDirPath();
        userHomePath = QDir::homePath() + "/Pozh/Saaghar/";
#endif
#ifdef Q_OS_X11
        //dataBaseCompleteName = "/usr/share/saaghar/"+dataBaseCompleteName;
        resourcesPath = PREFIX"/share/saaghar/";
        userHomePath = QDir::homePath() + "/.Pozh/Saaghar/";
#endif
#ifdef Q_OS_MAC
        //dataBaseCompleteName = QDir::homePath()+"/Library/Saaghar/"+dataBaseCompleteName;
        resourcesPath = QCoreApplication::applicationDirPath() + "/../Resources/";
        userHomePath = QDir::homePath() + "/Library/Saaghar/";
#endif
        dataBaseCompleteName = resourcesPath + dataBaseCompleteName;
    }

    bool fresh = QCoreApplication::arguments().contains("-fresh", Qt::CaseInsensitive);

    QString uiLanguage = getSettingsObject()->value("UI Language", "fa").toString();

    QTranslator* appTranslator = new QTranslator();
    QTranslator* basicTranslator = new QTranslator();

    if (appTranslator->load(QString("saaghar_") + uiLanguage, resourcesPath)) {
        QCoreApplication::installTranslator(appTranslator);
        if (basicTranslator->load(QString("qt_") + uiLanguage, resourcesPath)) {
            QCoreApplication::installTranslator(basicTranslator);
        }
    }

    QDir saagharUserPath(userHomePath);
    if (!saagharUserPath.exists()) {
        saagharUserPath.mkpath(userHomePath);
    }

#ifndef NO_PHONON_LIB
    SaagharWidget::musicPlayer = new QMusicPlayer(this);
    SaagharWidget::musicPlayer->setWindowTitle(tr("Audio Player"));
    addDockWidget(Qt::RightDockWidgetArea, SaagharWidget::musicPlayer->albumManagerDock());
    SaagharWidget::musicPlayer->albumManagerDock()->hide();
    SaagharWidget::musicPlayer->hide();

    SaagharWidget::musicPlayer->readPlayerSettings(getSettingsObject());

    if (QMusicPlayer::albumsPathList.isEmpty()) {
        const QString &defaultFile = userHomePath + "/default.sal";
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

    loadGlobalSettings();

    if (m_splash && !fresh) {
        if (Settings::READ("Display Splash Screen", true).toBool()) {
            m_splash->show();
            showStatusText(QObject::tr("<i><b>Loading...</b></i>"));
        }
        else {
            m_splash->finish(this);
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

    SaagharWidget::poetsImagesDir = resourcesPath + "/poets_images/";

    //loading application fonts
    //QString applicationFontsPath = resourcesPath + "/fonts/";
    QDir fontsDir(resourcesPath + "/fonts/");
    QStringList fontsList = fontsDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (int i = 0; i < fontsList.size(); ++i) {
        QFontDatabase::addApplicationFont(resourcesPath + "/fonts/" + fontsList.at(i));
    }

    ui->setupUi(this);

    //create Parent Categories ToolBar
    parentCatsToolBar = new QToolBar(this);
    parentCatsToolBar->setObjectName(QString::fromUtf8("parentCatsToolBar"));
    parentCatsToolBar->setLayoutDirection(Qt::RightToLeft);
    parentCatsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    parentCatsToolBar->setWindowTitle(tr("Parent Categories"));

    //create Tab Widget
    mainTabWidget = ui->tabWidget;

    if (autoCheckForUpdatesState && !fresh) {
        showStatusText(tr("<i><b>Checking for update...</b></i>"));
        QCoreApplication::processEvents();
        checkForUpdates();
    }

    setupUi();

    //setup corner widget
    mainTabWidget->getTabBar()->addTabButton()->setIcon(QIcon(ICON_PATH + "/add-tab.png"));
    connect(mainTabWidget->getTabBar()->addTabButton(), SIGNAL(clicked()), this, SLOT(actionNewTabClicked()));

    QToolButton* cornerMenuButton = new QToolButton(mainTabWidget);
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

    //searching database-path for database-file
    //following lines are for support old default data-base pathes.
    QStringList dataBaseCompatibilityPath(QGanjoorDbBrowser::dataBasePath);
    dataBaseCompatibilityPath   << resourcesPath
                                << QDir::homePath() + "/Pojh/Saaghar/"
                                << QDir::homePath() + "/Library/Saaghar/";
    for (int i = 0; i < dataBaseCompatibilityPath.size(); ++i) {
        if (QFile::exists(dataBaseCompatibilityPath.at(i) + tempDataBaseName)) {
            dataBaseCompleteName = dataBaseCompatibilityPath.at(i) + tempDataBaseName;
            break;
        }
    }
    //create ganjoor DataBase browser
    SaagharWidget::ganjoorDataBase = new QGanjoorDbBrowser(dataBaseCompleteName, splashScreen);
    qDebug() << "QGanjoorDbBrowser2222=" << SaagharWidget::ganjoorDataBase;
    SaagharWidget::ganjoorDataBase->setObjectName(QString::fromUtf8("ganjoorDataBaseBrowser"));

    loadTabWidgetSettings();

    //ui->gridLayout->setContentsMargins(0,0,0,0);

    outlineTree->setItems(SaagharWidget::ganjoorDataBase->loadOutlineFromDataBase(0));
    connect(outlineTree, SIGNAL(openParentRequested(int)), this, SLOT(openParentPage(int)));
    connect(outlineTree, SIGNAL(newParentRequested(int)), this, SLOT(newTabForItem(int)));
    connect(outlineTree, SIGNAL(openRandomRequested(int,bool)), this, SLOT(openRandomPoem(int,bool)));

//  splitter = new QSplitter(ui->centralWidget);
//  splitter->setObjectName("splitter");
//  //splitter->addWidget(dock);
//  splitter->addWidget(mainTabWidget);

    //ui->gridLayout->addWidget(mainTabWidget, 0, 0, 1, 1);

    if (!fresh) {
        QStringList openedTabs = Settings::READ("Opened tabs from last session").toStringList();
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

    QListWidgetItem* item = selectSearchRange->insertRow(0, tr("All Opened Tab"), true, "ALL_OPENED_TAB", Qt::UserRole, true);
    QListWidgetItem* titleSearchItem = selectSearchRange->insertRow(1, tr("Titles"), true, "ALL_TITLES", Qt::UserRole);
    multiSelectObjectInitialize(selectSearchRange, selectedSearchRange, 2);
    item->setCheckState(selectedSearchRange.contains("ALL_OPENED_TAB") ? Qt::Checked : Qt::Unchecked);
    titleSearchItem->setCheckState(selectedSearchRange.contains("ALL_TITLES") ? Qt::Checked : Qt::Unchecked);

    //seeding random function
    uint numOfSecs = QDateTime::currentDateTime().toTime_t();
    uint seed = QCursor::pos().x() + QCursor::pos().y() + numOfSecs + QDateTime::currentDateTime().time().msec();
    qsrand(seed);

    //install search pattern manager
    SearchPatternManager* searchPatternManager = new SearchPatternManager();
    searchPatternManager->setWildcardCharacter("%");

    restoreState(Settings::READ("MainWindowState").toByteArray(), 1);
    restoreGeometry(Settings::READ("Mainwindow Geometry").toByteArray());

//  if (!splitter->restoreState(Settings::READ("SplitterState").toByteArray()))
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
        QtWin::easyBlurUnBlur(this, Settings::READ("UseTransparecy").toBool());
    }

    showStatusText(tr("<i><b>Saaghar is starting...</b></i>"), -1);
}

SaagharWindow::~SaagharWindow()
{
    delete SaagharWidget::ganjoorDataBase;
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

    int numOfResults = 0;
    bool searchCanceled = false;
    bool slowSearch = false;//more calls of 'processEvents'
    if (selectList.size() > 1) {
        slowSearch = true;
    }

    for (int i = 0; i < selectList.size(); ++i) {
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
            //phrase = QGanjoorDbBrowser::cleanString(SaagharWidget::lineEditSearchText->text(), SaagharWidget::newSearchSkipNonAlphabet);
            //int poetID = comboBoxSearchRegion->itemData(comboBoxSearchRegion->currentIndex(), Qt::UserRole).toInt();
            SearchPatternManager::setInputPhrase(phrase);
            SearchPatternManager::init();
            QVector<QStringList> phraseVectorList = SearchPatternManager::outputPhrases();
            QVector<QStringList> excludedVectorList = SearchPatternManager::outputExcludedLlist();
            int vectorSize = phraseVectorList.size();
//          qDebug() << "SearchPatternManager::finished!";
//          qDebug() << "phraseVectorList=" <<phraseVectorList;
//          qDebug() << "excludedVectorList="<<excludedVectorList;
            qDebug() << "=======================" << currentItemData;

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
                //SaagharWidget::ganjoorDataBase->getPoet(poetID)._Name;
            }

            QWidget* searchResultContents = new QWidget(this);
            searchResultContents->setObjectName(QString::fromUtf8("searchResultContents"));
            searchResultContents->setAttribute(Qt::WA_DeleteOnClose, true);
            //searchResultContents->setLayoutDirection(Qt::RightToLeft);

            phrase = QGanjoorDbBrowser::cleanString(phrase);
            SearchResultWidget* searchResultWidget = new SearchResultWidget(searchResultContents, phrase, poetName);

            /////////////////////////////////////
            QMap<int, QString> finalResult;
//          QProgressDialog searchProgress(tr("Searching Data Base..."),  tr("Cancel"), 0, 0, this);
//          connect( &searchProgress, SIGNAL(canceled()), &searchProgress, SLOT(hide()) );
//          searchProgress.setWindowModality(Qt::WindowModal);
//          searchProgress.setFixedSize(searchProgress.size());
//          searchProgress.setMinimumDuration(0);
//          //searchProgress.setValue(0);
//          //searchProgress.show();

            SaagharWidget::lineEditSearchText->searchStart(&searchCanceled);
            connect(SaagharWidget::ganjoorDataBase, SIGNAL(searchStatusChanged(QString)), SaagharWidget::lineEditSearchText, SLOT(setSearchProgressText(QString)));
            //emit SaagharWidget::ganjoorDataBase->searchStatusChanged(tr("Searching Data Base..."));
            SaagharWidget::lineEditSearchText->setSearchProgressText(tr("Searching Data Base(subset= %1)...").arg(poetName));
            QApplication::processEvents();
            int resultCount = 0;
            for (int j = 0; j < vectorSize; ++j) {
                QStringList phrases = phraseVectorList.at(j);
//              int phraseCount = phrases.size();
                QStringList excluded = excludedVectorList.at(j);
                int start = QDateTime::currentDateTime().toTime_t() * 1000 + QDateTime::currentDateTime().time().msec();
                QMap<int, QString> mapResult = SaagharWidget::ganjoorDataBase->getPoemIDsByPhrase(poetID, phrases, excluded, &searchCanceled, resultCount, slowSearch);
                int end = QDateTime::currentDateTime().toTime_t() * 1000 + QDateTime::currentDateTime().time().msec();
                int miliSec = end - start;
                qDebug() << "\n------------------------------------------------\n"
                         << phrases << "\tsearch-duration=" << miliSec
                         << "\n------------------------------------------------\n";
                QMap<int, QString>::const_iterator it = mapResult.constBegin();

//              qDebug() << "mapResult-size" << mapResult.size();
                while (it != mapResult.constEnd()) {
                    finalResult.insertMulti(it.key(), it.value());//insertMulti for more than one result from a poem
                    ++it;
                }
                resultCount = finalResult.size();
//              qDebug() << "resultCount=" << resultCount;

                QApplication::processEvents();

                qDebug() << "finalResult-size" << finalResult.size();
                if (searchCanceled) {
                    break;
                }
            }

            numOfResults += resultCount;

            if (i == selectList.size() - 1) {
                SaagharWidget::lineEditSearchText->searchStop();
            }
            ///////////////////////////////////////

            searchResultWidget->setResultList(finalResult);
            if (!searchResultWidget->init(this, ICON_PATH)) {
                if (numOfResults == 0 && (i == selectList.size() - 1)) {
                    //QMessageBox::information(this, tr("Search"), tr("Current Scope: %1\nNo match found.").arg(poetName));
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

            //resize table on create and destroy
//          emitReSizeEvent();
//          connect(searchResultWidget, SIGNAL(destroyed()), this, SLOT(emitReSizeEvent()));
        }
        //we should create conection first and then break!
        if (searchCanceled) {
            break;    //search is canceled
        }
    }
}

void SaagharWindow::multiSelectObjectInitialize(QMultiSelectWidget* multiSelectWidget, const QStringList &selectedData, int insertIndex)
{
    qDebug() << "multiSelectObjectInitialize1";
    QListWidgetItem* rootItem = multiSelectWidget->insertRow(insertIndex, tr("All"), true, "0", Qt::UserRole);
    qDebug() << "multiSelectObjectInitialize22";
    QList<GanjoorPoet*> poets = SaagharWidget::ganjoorDataBase->getPoets();
    qDebug() << "multiSelectObjectInitialize333";
    //int insertIndex = count();
    for (int i = 0; i < poets.size(); ++i) {
        qDebug() << "for1-multiSelectObjectInitialize" << i + 4 << "i=" << i;
        multiSelectWidget->insertRow(i + 1 + insertIndex, poets.at(i)->_Name, true,
                                     QString::number(poets.at(i)->_ID), Qt::UserRole, false, rootItem)->setCheckState(selectedData.contains(QString::number(poets.at(i)->_ID)) ? Qt::Checked : Qt::Unchecked);
        qDebug() << "for2-multiSelectObjectInitialize" << i + 4 << "i=" << i;
    }
    qDebug() << "multiSelectObjectInitialize55555";
    if (selectedData.contains("0")) {
        rootItem->setCheckState(Qt::Checked);
    }
    qDebug() << "multiSelectObjectInitialize666666";
    multiSelectWidget->updateSelectedLists();
    qDebug() << "multiSelectObjectInitialize7777777";
}

void SaagharWindow::actionRemovePoet()
{
    bool ok = false;
    QStringList items;
    items << tr("Select a name...");
    QList<GanjoorPoet*> poets = SaagharWidget::ganjoorDataBase->getPoets();
    for (int i = 0; i < poets.size(); ++i) {
        items << poets.at(i)->_Name + "(" + tr("poet's code=") + QString::number(poets.at(i)->_ID) + ")";
    }
    QString item = QInputDialog::getItem(this, tr("Remove Poet"), tr("Select a poet name and click on 'OK' button, for remove it from database."), items, 0, false, &ok);
    if (ok && !item.isEmpty() && item != tr("Select a name...")) {
        qDebug() << "item=" << item;
        qDebug() << "int=" << item.toInt();
        qDebug() << "Int-itemleft=" << item.mid(item.indexOf("(" + tr("poet's code=")) + tr("poet's code=").size() + 1).remove(")").toInt();

        //int poetID = SaagharWidget::ganjoorDataBase->getPoet(item.left(item.indexOf("(")))._ID;
        int poetID = item.mid(item.indexOf("(" + tr("poet's code=")) + tr("poet's code=").size() + 1).remove(")").toInt();
        if (poetID > 0) {
            QMessageBox warnAboutDelete(this);
            warnAboutDelete.setWindowTitle(tr("Please Notice!"));
            warnAboutDelete.setIcon(QMessageBox::Warning);
            warnAboutDelete.setText(tr("Are you sure for removing \"%1\", from database?").arg(SaagharWidget::ganjoorDataBase->getPoet(poetID)._Name/*item*/));
            warnAboutDelete.addButton(tr("Continue"), QMessageBox::AcceptRole);
            warnAboutDelete.setStandardButtons(QMessageBox::Cancel);
            warnAboutDelete.setEscapeButton(QMessageBox::Cancel);
            warnAboutDelete.setDefaultButton(QMessageBox::Cancel);
            int ret = warnAboutDelete.exec();
            if (ret == QMessageBox::Cancel) {
                return;
            }

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            qDebug() << "Before remove";
            SaagharWidget::ganjoorDataBase->removePoetFromDataBase(poetID);
            qDebug() << "after remove";
            outlineTree->setItems(SaagharWidget::ganjoorDataBase->loadOutlineFromDataBase(0));

            //comboBoxSearchRegion->clear();
            selectSearchRange->clear();
            QListWidgetItem* item = selectSearchRange->insertRow(0, tr("All Opened Tab"), true, "ALL_OPENED_TAB", Qt::UserRole, true);
            QListWidgetItem* titleSearchItem = selectSearchRange->insertRow(1, tr("Titles"), true, "ALL_TITLES", Qt::UserRole);
            multiSelectObjectInitialize(selectSearchRange, selectedSearchRange, 2);
            item->setCheckState(selectedSearchRange.contains("ALL_OPENED_TAB") ? Qt::Checked : Qt::Unchecked);
            titleSearchItem->setCheckState(selectedSearchRange.contains("ALL_TITLES") ? Qt::Checked : Qt::Unchecked);
            //update visible region of searchToolBar
//          bool tmpFlag = labelMaxResultAction->isVisible();
//          labelMaxResultAction->setVisible(!tmpFlag);
//          ui->searchToolBar->update();
//          QApplication::processEvents();
//          labelMaxResultAction->setVisible(tmpFlag);
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

//      if (old_saagharWidget && old_saagharWidget->splitter && saagharWidget->splitter)
//      {
//      qDebug()<<"currentTabChanged222="<<old_saagharWidget<<"saagharWidget="<<saagharWidget;
//          QList<int> sizes = old_saagharWidget->splitter->sizes();
//          saagharWidget->splitter->insertWidget(old_saagharWidget->splitter->indexOf(outlineTree), outlineTree);
//          saagharWidget->splitter->setSizes(sizes);
//      }

        saagharWidget->loadSettings();

        if (saagharWidget->isDirty()) {
            saagharWidget->refresh();
        }
        saagharWidget->showParentCategory(SaagharWidget::ganjoorDataBase->getCategory(saagharWidget->currentCat));//just update parentCatsToolbar
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
                    //int textWidth = QFontMetrics(Settings::getFromFonts(Settings::ProseTextFontColor)).boundingRect(text).width();
                    int verticalScrollBarWidth = 0;
                    if (saagharWidget->tableViewWidget->verticalScrollBar()->isVisible()) {
                        verticalScrollBarWidth = saagharWidget->tableViewWidget->verticalScrollBar()->width();
                    }
                    int totalWidth = saagharWidget->tableViewWidget->columnWidth(0) - verticalScrollBarWidth - 82;
                    //int totalWidth = saagharWidget->tableViewWidget->viewport()->width()-verticalScrollBarWidth-82;
                    totalWidth = qMax(82 + verticalScrollBarWidth, totalWidth);
                    saagharWidget->resizeTable(saagharWidget->tableViewWidget);
                    //saagharWidget->tableViewWidget->setRowHeight(0, qMax(100, SaagharWidget::computeRowHeight(saagharWidget->tableViewWidget->fontMetrics(), textWidth, totalWidth)) );
                    ////saagharWidget->tableViewWidget->setRowHeight(0, 2*saagharWidget->tableViewWidget->rowHeight(0)+(saagharWidget->tableViewWidget->fontMetrics().height()*(numOfRow/*+1*/)));
                }
            }
            else if (saagharWidget->tableViewWidget->columnCount() > 1) {
                QTableWidgetItem* item = saagharWidget->tableViewWidget->item(0, 1);
                if (item) {
                    QString text = item->text();
                    //int textWidth = saagharWidget->tableViewWidget->fontMetrics().boundingRect(text).width();
                    int verticalScrollBarWidth = 0;
                    if (saagharWidget->tableViewWidget->verticalScrollBar()->isVisible()) {
                        verticalScrollBarWidth = saagharWidget->tableViewWidget->verticalScrollBar()->width();
                    }
                    int totalWidth = saagharWidget->tableViewWidget->viewport()->width();
                    //saagharWidget->tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(saagharWidget->tableViewWidget->fontMetrics(), textWidth, totalWidth) );
                    //new!!
                    saagharWidget->resizeTable(saagharWidget->tableViewWidget);
                }
            }
        }
        else if (SaagharWidget::maxPoetsPerGroup != 0 &&
                 saagharWidget->currentCat == 0 && saagharWidget->currentPoem == 0) {
            QList<GanjoorPoet*> poets = SaagharWidget::ganjoorDataBase->getPoets();
            int numOfPoets = poets.size();
            if (numOfPoets > SaagharWidget::maxPoetsPerGroup) {
                if (SaagharWidget::maxPoetsPerGroup != 1) {
                    saagharWidget->tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::TitlesFontColor)), -1, -1));
                }
            }
        }

        undoGroup->setActiveStack(saagharWidget->undoStack);
        updateCaption();
        updateTabsSubMenus();

        actionInstance("actionPreviousPoem")->setEnabled(!SaagharWidget::ganjoorDataBase->getPreviousPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());
        actionInstance("actionNextPoem")->setEnabled(!SaagharWidget::ganjoorDataBase->getNextPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());

        loadAudioForCurrentTab(old_saagharWidget);

        parentCatsToolBar->setEnabled(true);
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

        globalRedoAction->setEnabled(false);
        globalUndoAction->setEnabled(false);
        actionInstance("fixedNameRedoAction")->setEnabled(false);
        actionInstance("fixedNameUndoAction")->setEnabled(false);
#ifndef NO_PHONON_LIB
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
    QAction* action = qobject_cast<QAction*>(sender());

    QEventLoop loop;

    QStringList updateInfoServers;
    updateInfoServers << "http://srazi.github.io/Saaghar/saaghar.version"
                      << "http://saaghar.sourceforge.net/saaghar.version"
                      << "http://en.saaghar.pozh.org/saaghar.version";

    QNetworkReply* reply;
    bool error = true;

    for (int i = 0; i < updateInfoServers.size(); ++i) {
        showStatusText("<i><b>" + tr("Checking for updates... (server number=%1)").arg(i + 1) + "</b></i>");
        QProgressDialog updateProgress(tr("Checking for updates... (server number=%1)").arg(i + 1),  tr("Cancel"), 0, 0, this);
        if (action) {
            updateProgress.setMinimumDuration(0);
        }
        else {
            updateProgress.setParent(0);
            updateProgress.setMinimumDuration(5000);
        }

        updateProgress.setWindowModality(Qt::WindowModal);
        updateProgress.setFixedSize(updateProgress.size());
        if (action || !Settings::READ("Display Splash Screen", true).toBool()) {
            updateProgress.show();
        }

        QNetworkRequest requestVersionInfo(QUrl(updateInfoServers.at(i)));
        QNetworkAccessManager* netManager = new QNetworkAccessManager();
        reply = netManager->get(requestVersionInfo);

        connect(&updateProgress, SIGNAL(canceled()), &loop, SLOT(quit()));
        connect(reply, SIGNAL(finished()), &updateProgress, SLOT(hide()));
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        if (updateProgress.wasCanceled()) {
            updateProgress.hide();
            loop.quit();
            return;
        }

        if (!reply->error()) {
            error = false;
            break;
        }
    }

    if (error) {
        showStatusText("!QExtendedSplashScreenCommands:HIDE");
        QMessageBox criticalError(QMessageBox::Critical, tr("Error"), tr("There is an error when checking for updates...\nCheck your internet connection and try again."), QMessageBox::Ok, this, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
        criticalError.exec();
        if (!action) {
            showStatusText("!QExtendedSplashScreenCommands:SHOW");
        }
        return;
    }
    QStringList data = QString::fromUtf8(reply->readAll()).split("|", QString::SkipEmptyParts);

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
    QString runningAppVersionStr = QString(VER_FILEVERSION_STR);
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
        if (action) {
            QMessageBox::information(this, tr("Saaghar is up to date"), tr("There is no new version available. Please check for updates later!"));
        }
    }
}

void SaagharWindow::tabCloser(int tabIndex)
{
    QWidget* closedTabContent = mainTabWidget->widget(tabIndex);
    if (closedTabContent) {
        qDebug() << "closedTabContent=" << closedTabContent->objectName();
    }
    else {
        qDebug() << "closedTabContent NUUUUUUL";
    }
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

//  if (tmp && tmp->splitter && saagharWidget->splitter)
//  {
//      qDebug()<<"tabCloser()111="<<tmp<<"saagharWidget="<<saagharWidget;
//      QList<int> sizes = tmp->splitter->sizes();
//      saagharWidget->splitter->insertWidget(tmp->splitter->indexOf(outlineTree), outlineTree);
//      saagharWidget->splitter->setSizes(sizes);
//  }

//  QAction *tabAction = new QAction(mainTabWidget->tabText(i), menuOpenedTabs);
//  tabAction->setData(mainTabWidget->widget(i) /*QString::number(i)*/);
//  menuOpenedTabs->addAction(tabAction);

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
        tabGridLayout->setContentsMargins(0, 0, 0, 0); //3,4,3,3);

        tabContent->setObjectName(QString::fromUtf8("SaagharViewerTab"));
        QTableWidget* tabTableWidget = new QTableWidget;//(tabContent);

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
        //temp
        connect(saagharWidget, SIGNAL(captionChanged()), this, SLOT(updateTabsSubMenus()));

        connect(saagharWidget->tableViewWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(tableItemClick(QTableWidgetItem*)));
        connect(saagharWidget->tableViewWidget, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(tableItemPress(QTableWidgetItem*)));
        connect(saagharWidget->tableViewWidget, SIGNAL(itemEntered(QTableWidgetItem*)), this, SLOT(tableItemMouseOver(QTableWidgetItem*)));
        connect(saagharWidget->tableViewWidget, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)), this, SLOT(tableCurrentItemChanged(QTableWidgetItem*,QTableWidgetItem*)));

        connect(saagharWidget->tableViewWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(createCustomContextMenu(QPoint)));
        connect(saagharWidget, SIGNAL(createContextMenuRequested(QPoint)), this, SLOT(createCustomContextMenu(QPoint)));

        //Enable/Disable navigation actions
        connect(saagharWidget, SIGNAL(navPreviousActionState(bool)),    actionInstance("actionPreviousPoem"), SLOT(setEnabled(bool)));
        connect(saagharWidget, SIGNAL(navNextActionState(bool)),    actionInstance("actionNextPoem"), SLOT(setEnabled(bool)));
        actionInstance("actionPreviousPoem")->setEnabled(!SaagharWidget::ganjoorDataBase->getPreviousPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());
        actionInstance("actionNextPoem")->setEnabled(!SaagharWidget::ganjoorDataBase->getNextPoem(saagharWidget->currentPoem, saagharWidget->currentCat).isNull());

        //Updating table on changing of selection
        connect(saagharWidget->tableViewWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectChanged()));
        //tabGridLayout->setContentsMargins(0,0,0,0);//3,4,3,3);
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

        QPixmap pixmap(resourcesPath + "/themes/backgrounds/saaghar-pattern_1.png");
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
//    saagharWidget->processClickedItem(type, id, noError, pushToStack);
    showStatusText(tr("<i><b>\"%1\" was loaded!</b></i>").arg(QGanjoorDbBrowser::snippedText(saagharWidget->currentCaption.mid(saagharWidget->currentCaption.lastIndexOf(":") + 1), "", 0, 6, false, Qt::ElideRight)));
    //qDebug() << "emit updateTabsSubMenus()--848";
    //resolved by signal SaagharWidget::captionChanged()
    //updateTabsSubMenus();
}

void SaagharWindow::updateCaption()
{
    if (mainTabWidget->currentIndex() == -1 || !getSaagharWidget(mainTabWidget->currentIndex())) {
        return;
    }
    SaagharWidget* sw = getSaagharWidget(mainTabWidget->currentIndex());

    QString newTabCaption = QGanjoorDbBrowser::snippedText(sw->currentCaption, "", 0, 6, true, Qt::ElideRight) + QString(QChar(0x200F));
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

    const int extendedWidth = Settings::getFromFonts(Settings::PoemTextFontColor).pointSize() * 4;
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
                QFont fnt(Settings::getFromFonts(Settings::PoemTextFontColor));
                QColor color(Settings::getFromColors(Settings::PoemTextFontColor));
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
                        int textWidth;
                        QFontMetrics fontMetric(fnt);
                        if (!item->text().isEmpty()) {
                            textWidth = fontMetric.boundingRect(item->text()).width();
                        }
                        else {
                            textEdit = qobject_cast<QTextEdit*>(saagharWidget->tableViewWidget->cellWidget(row, col));
                            if (textEdit) {
                                fnt = Settings::getFromFonts(Settings::ProseTextFontColor);
                                color = Settings::getFromColors(Settings::ProseTextFontColor);
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
    QtWin::easyBlurUnBlur(previewDialog, Settings::READ("UseTransparecy").toBool());

    connect(previewDialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
    previewDialog->exec();
}

QString SaagharWindow::convertToHtml(SaagharWidget* saagharObject)
{
    GanjoorPoem curPoem = SaagharWidget::ganjoorDataBase->getPoem(saagharObject->currentPoem);
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

                //QString align = "RIGHT";
                //if (SaagharWidget::centeredView)
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
            .arg(Settings::getFromFonts(Settings::PoemTextFontColor).family())
            .arg(Settings::getFromColors(Settings::PoemTextFontColor).name())
            .arg(Settings::getFromFonts(Settings::PoemTextFontColor).pointSize())
            .arg(Settings::getFromFonts(Settings::PoemTextFontColor).bold() ? "FONT-WEIGHT: bold" : "")
            .arg(columnGroupFormat).arg(tableBody);

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

QString SaagharWindow::convertToTeX(SaagharWidget* saagharObject)
{
    GanjoorPoem curPoem = SaagharWidget::ganjoorDataBase->getPoem(saagharObject->currentPoem);
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
                if (row == 0 /*&& SaagharWidget::centeredView*/) { //title is centered by using each PoemViewStyle
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
                         .arg(VER_FILEVERSION_STR).arg(Settings::getFromFonts(Settings::PoemTextFontColor).family()).arg(curPoem._Title).arg(poemType).arg(tableBody).arg(endOfEnvironment);
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

        QtWin::easyBlurUnBlur(this, Settings::READ("UseTransparecy").toBool());
    }
    setUpdatesEnabled(true);
}

void SaagharWindow::openRandomPoem(int parentID, bool newPage)
{
//  QAction *triggeredAction = qobject_cast<QAction *>(sender());
//  int actionData = -1;//Faal
//  if (triggeredAction && triggeredAction->data().isValid())
//      actionData = triggeredAction->data().toInt();

//  if (actionData == -1)
//      actionData = 24;
//  else
//  {
//      if (selectedRandomRange.contains("CURRENT_TAB_SUBSECTIONS"))
//      {
//          if (saagharWidget->currentPoem != 0)
//              actionData = SaagharWidget::ganjoorDataBase->getPoem(saagharWidget->currentPoem)._CatID;
//          else
//              actionData = saagharWidget->currentCat;
//      }
//      else if (selectedRandomRange.contains("0")) //all
//          actionData = 0;//home
//      else
//      {
//          if (selectedRandomRange.isEmpty())
//              actionData = 0;//all
//          else
//          {
//              int randIndex = QGanjoorDbBrowser::getRandomNumber(0, selectedRandomRange.size()-1);
//              actionData = SaagharWidget::ganjoorDataBase->getPoet(selectedRandomRange.at(randIndex).toInt())._CatID;
//              qDebug() << "actionData=" << actionData << "randIndex=" << randIndex << SaagharWidget::ganjoorDataBase->getPoet(selectedRandomRange.at(randIndex))._Name;
//          }
//      }
//  }

    int actionData = parentID;

    int PoemID = SaagharWidget::ganjoorDataBase->getRandomPoemID(&actionData);
    GanjoorPoem poem = SaagharWidget::ganjoorDataBase->getPoem(PoemID);
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

    QtWin::easyBlurUnBlur(&about, Settings::READ("UseTransparecy").toBool());

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
    if (!QDesktopServices::openUrl("file:///" + resourcesPath + "/Saaghar-Manual.pdf")) {
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
//  QMessageBox warnAboutBackup(this);
//  warnAboutBackup.setWindowTitle(tr("Please Notice!"));
//  warnAboutBackup.setIcon(QMessageBox::Information);
//  warnAboutBackup.setText(tr("This feature is in beta state, before continue you should create a backup from your database."));
//  warnAboutBackup.addButton(tr("Continue"), QMessageBox::AcceptRole);
//  warnAboutBackup.setStandardButtons(QMessageBox::Cancel);
//  warnAboutBackup.setEscapeButton(QMessageBox::Cancel);
//  warnAboutBackup.setDefaultButton(QMessageBox::Cancel);
//  int ret = warnAboutBackup.exec();
//  if ( ret == QMessageBox::Cancel)
//      return;

    QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Browse for a new set"), QDir::homePath(), "Supported Files (*.gdb *.s3db *.zip);;Ganjoor DataBase (*.gdb *.s3db);;Compressed Data Sets (*.zip);;All Files (*.*)");
    if (!fileList.isEmpty()) {
        if (!QGanjoorDbBrowser::dbUpdater) {
            QGanjoorDbBrowser::dbUpdater = new DataBaseUpdater(this);
        }

        foreach (const QString &file, fileList) {
            //connect(QGanjoorDbBrowser::dbUpdater, SIGNAL(installRequest(QString,bool*)), this, SLOT(importDataBase(QString,bool*)));
            QGanjoorDbBrowser::dbUpdater->installItemToDB(file);
            QApplication::processEvents();
        }
    }
}

void SaagharWindow::setupUi()
{
#ifndef NO_PHONON_LIB
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

//    if (Settings::READ("MainWindowState").isNull()) {
//        insertToolBarBreak(ui->mainToolBar);
//    }

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
        qDebug() << "It seems that you are running Unity Desktop with global menubar!";
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
    actionInstance("Show Photo at Home", ICON_PATH + "/show-photo-home.png", tr("&Show Photo at Home"))->setCheckable(true);
    actionInstance("Show Photo at Home")->setChecked(Settings::READ("Show Photo at Home", true).toBool());

    actionInstance("Lock ToolBars", ICON_PATH + "/lock-toolbars.png", tr("&Lock ToolBars"))->setCheckable(true);
    actionInstance("Lock ToolBars")->setChecked(Settings::READ("Lock ToolBars", true).toBool());
    ui->mainToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());
    if (ui->menuToolBar) {
        ui->menuToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());
    }
    ui->searchToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());
    parentCatsToolBar->setMovable(!Settings::READ("Lock ToolBars").toBool());

#ifndef NO_PHONON_LIB
    SaagharWidget::musicPlayer->setMovable(!Settings::READ("Lock ToolBars").toBool());
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

//  poemViewStylesMenu->addAction(actionInstance("LastBeytCenteredPoemViewStyle", ICON_PATH+"/poem-view-last-beyt-centered.png",QObject::tr("&Last Beyt Centered")) );
//  actionInstance("LastBeytCenteredPoemViewStyle")->setParent(poemViewStylesMenu);
//  actionInstance("LastBeytCenteredPoemViewStyle")->setActionGroup(poemViewStylesGroup);
//  actionInstance("LastBeytCenteredPoemViewStyle")->setCheckable(true);
//  actionInstance("LastBeytCenteredPoemViewStyle")->setData(SaagharWidget::LastBeytCentered);

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

//  poemViewStylesMenu->addAction(actionInstance("MesraPerLineGroupedBeytPoemViewStyle", ICON_PATH+"/poem-view-mesra-per-line-grouped-beyt.png",QObject::tr("Mesra Per Line &Grouped Beyt")) );
//  actionInstance("MesraPerLineGroupedBeytPoemViewStyle")->setParent(poemViewStylesMenu);
//  actionInstance("MesraPerLineGroupedBeytPoemViewStyle")->setActionGroup(poemViewStylesGroup);
//  actionInstance("MesraPerLineGroupedBeytPoemViewStyle")->setCheckable(true);
//  actionInstance("MesraPerLineGroupedBeytPoemViewStyle")->setData(SaagharWidget::MesraPerLineGroupedBeyt);

    outlineTree = new OutLineTree;
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
//  case SaagharWidget::LastBeytCentered:
//      actionInstance("LastBeytCenteredPoemViewStyle")->setChecked(true);
//      break;
    case SaagharWidget::OneHemistichLine:
        actionInstance("OneHemistichPoemViewStyle")->setChecked(true);
        break;
    case SaagharWidget::SteppedHemistichLine:
        actionInstance("SteppedHemistichPoemViewStyle")->setChecked(true);
        break;
//  case SaagharWidget::MesraPerLineGroupedBeyt:
//      actionInstance("MesraPerLineGroupedBeytPoemViewStyle")->setChecked(true);
//      break;

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
//  QString debug = QString("PM_MenuBarHMargin = %1\nPM_MenuBarPanelWidth= %2\nPM_MenuBarItemSpacing= %3\nStyle= %4\nstyleWidth= %5")
//          .arg(ui->menuBar->style()->pixelMetric(QStyle::PM_MenuBarHMargin)).arg(ui->menuBar->style()->pixelMetric(QStyle::PM_MenuBarPanelWidth)).arg(ui->menuBar->style()->pixelMetric(QStyle::PM_MenuBarItemSpacing)).arg(QApplication::style()->objectName()).arg(styleWidth);
//  QMessageBox::information(0,"pixel",debug);
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

#ifndef NO_PHONON_LIB
    toolbarsView->addAction(actionInstance("toggleMusicPlayer"));
#endif

    toolbarsView->addSeparator();
    toolbarsView->addAction(actionInstance("Lock ToolBars"));
    menuView->addMenu(toolbarsView);

    QMenu* panelsView = new QMenu(tr("Panels"), menuView);
    panelsView->addAction(actionInstance("outlineDockAction"));
#ifndef NO_PHONON_LIB
    panelsView->addAction(actionInstance("albumDockAction"));
#endif
    panelsView->addAction(actionInstance("bookmarkManagerDockAction"));
    menuView->addMenu(panelsView);

    menuView->addSeparator();
    toolBarViewActions(ui->mainToolBar, menuView, true);
    //checked actions, must be after above line
    toolbarViewChanges(actionInstance(Settings::READ("MainToolBar Style").toString()));
    toolbarViewChanges(actionInstance(Settings::READ("MainToolBar Size").toString()));

    menuView->addMenu(poemViewStylesMenu);
    menuView->addAction(actionInstance("Show Photo at Home"));
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
    QString temp = mainToolBarItems.join("|");
    mainToolBarItems = temp.split("|", QString::SkipEmptyParts);

    addToolBar(Qt::TopToolBarArea, parentCatsToolBar);
    if (ui->menuToolBar) {
        insertToolBar(parentCatsToolBar, ui->menuToolBar);
    }
    if (SaagharWidget::musicPlayer) {
        insertToolBar(SaagharWidget::musicPlayer, ui->searchToolBar);
    }
    else {
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

void SaagharWindow::loadAudioForCurrentTab(SaagharWidget *old_saagharWidget)
{
#ifndef NO_PHONON_LIB
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
#endif //NO_PHONON_LIB
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
    if (!m_splash || m_splash->isFinished()) {
        return;
    }

    m_splash->addToMaximum(newLevelsCount, false);
    m_splash->showMessage(message);
}

//void MainWindow::newSearchNonAlphabetChanged(bool checked)
//{
//  SaagharWidget::newSearchSkipNonAlphabet = checked;
//}

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

        /*if (iconPath.isEmpty())
        {
            QString iconThemePath=":/resources/images/";
            if (!settingsIconThemePath.isEmpty() && settingsIconThemeState)
                iconThemePath = settingsIconThemePath;
            iconPath = iconThemePath+"/"+actionObjectName+".png";
        }*/

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
//  connect(actionInstance("actionFaal")                ,   SIGNAL(triggered())     ,   this, SLOT(actionFaalRandomClicked())   );
//  connect(actionInstance("actionRandom")              ,   SIGNAL(triggered())     ,   this, SLOT(actionFaalRandomClicked())   );

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
    //connect(pushButtonSearch                          ,   SIGNAL(clicked())       ,   this, SLOT(searchStart())               );
}

//Settings Dialog
void SaagharWindow::showSettingsDialog()
{
    m_settingsDialog = new Settings(this);
    //  m_settingsDialog->setAttribute(Qt::WA_DeleteOnClose, true);

    //if (Settings::READ("UseTransparecy").toBool())
    ////settingsDlg->setStyleSheet("QGroupBox{border: 1px solid lightgray;\nborder-radius: 5px;\n}");
    //settingsDlg->setStyleSheet("QGroupBox {\n     border: 1px solid lightgray;\n     border-radius: 5px;\n\n     margin-top: 1ex; /* leave space at the top for the title */\n }\n ");
    QtWin::easyBlurUnBlur(m_settingsDialog, Settings::READ("UseTransparecy").toBool());

    connect(m_settingsDialog->ui->pushButtonRandomOptions, SIGNAL(clicked()), this, SLOT(customizeRandomDialog()));

    /************initializing saved state of Settings dialog object!************/
    m_settingsDialog->ui->uiLanguageComboBox->addItems(QStringList() << "fa" << "en");
    int langIndex = m_settingsDialog->ui->uiLanguageComboBox->findText(Settings::READ("UI Language", "fa").toString(), Qt::MatchExactly);
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

    m_settingsDialog->ui->checkBoxAutoUpdates->setChecked(SaagharWindow::autoCheckForUpdatesState);

    //database
    m_settingsDialog->ui->lineEditDataBasePath->setText(QGanjoorDbBrowser::dataBasePath.join(";"));
    connect(m_settingsDialog->ui->pushButtonDataBasePath, SIGNAL(clicked()), m_settingsDialog, SLOT(browseForDataBasePath()));

    m_settingsDialog->ui->checkBoxBeytNumbers->setChecked(SaagharWidget::showBeytNumbers);

    m_settingsDialog->ui->checkBoxBackground->setChecked(SaagharWidget::backgroundImageState);
    m_settingsDialog->ui->lineEditBackground->setText(SaagharWidget::backgroundImagePath);
    m_settingsDialog->ui->lineEditBackground->setEnabled(SaagharWidget::backgroundImageState);
    m_settingsDialog->ui->pushButtonBackground->setEnabled(SaagharWidget::backgroundImageState);
    connect(m_settingsDialog->ui->pushButtonBackground, SIGNAL(clicked()), m_settingsDialog, SLOT(browseForBackground()));

    //splash screen
    m_settingsDialog->ui->checkBoxSplashScreen->setChecked(Settings::READ("Display Splash Screen", true).toBool());

    //initialize Action's Tables
    m_settingsDialog->initializeActionTables(allActionMap, mainToolBarItems);

    connect(m_settingsDialog->ui->pushButtonActionBottom, SIGNAL(clicked()), m_settingsDialog, SLOT(bottomAction()));
    connect(m_settingsDialog->ui->pushButtonActionTop, SIGNAL(clicked()), m_settingsDialog, SLOT(topAction()));
    connect(m_settingsDialog->ui->pushButtonActionAdd, SIGNAL(clicked()), m_settingsDialog, SLOT(addActionToToolbarTable()));
    connect(m_settingsDialog->ui->pushButtonActionRemove, SIGNAL(clicked()), m_settingsDialog, SLOT(removeActionFromToolbarTable()));

    m_settingsDialog->ui->checkBoxTransparent->setChecked(Settings::READ("UseTransparecy").toBool());

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

    Settings::WRITE("UI Language", m_settingsDialog->ui->uiLanguageComboBox->currentText());

    SaagharWidget::maxPoetsPerGroup = m_settingsDialog->ui->spinBoxPoetsPerGroup->value();

    SaagharWindow::autoCheckForUpdatesState = m_settingsDialog->ui->checkBoxAutoUpdates->isChecked();

    //database path
    QGanjoorDbBrowser::dataBasePath = m_settingsDialog->ui->lineEditDataBasePath->text().split(";", QString::SkipEmptyParts);

    SaagharWidget::showBeytNumbers = m_settingsDialog->ui->checkBoxBeytNumbers->isChecked();

    SaagharWidget::backgroundImageState = m_settingsDialog->ui->checkBoxBackground->isChecked();
    SaagharWidget::backgroundImagePath = m_settingsDialog->ui->lineEditBackground->text();

    //colors
    SaagharWidget::backgroundColor = m_settingsDialog->backgroundFontColor->color();
    SaagharWidget::matchedTextColor = m_settingsDialog->matchedFontColor->color();

    //splash screen
    Settings::WRITE("Display Splash Screen", m_settingsDialog->ui->checkBoxSplashScreen->isChecked());

    //toolbar items
    mainToolBarItems.clear();
    for (int i = 0; i < m_settingsDialog->ui->tableWidgetToolBarActions->rowCount(); ++i) {
        QTableWidgetItem* item = m_settingsDialog->ui->tableWidgetToolBarActions->item(i, 0);
        if (item) {
            mainToolBarItems.append(item->data(Qt::UserRole + 1).toString());
        }
    }

    Settings::WRITE("Main ToolBar Items", mainToolBarItems.join("|"));

    Settings::WRITE("Global Font", m_settingsDialog->ui->globalFontColorGroupBox->isChecked());
    //QFont fnt;
    Settings::insertToFontColorHash(&Settings::hashFonts, m_settingsDialog->outlineFontColor->sampleFont(), Settings::OutLineFontColor);
    Settings::insertToFontColorHash(&Settings::hashFonts, m_settingsDialog->globalTextFontColor->sampleFont(), Settings::DefaultFontColor);
    Settings::insertToFontColorHash(&Settings::hashFonts, m_settingsDialog->poemTextFontColor->sampleFont(), Settings::PoemTextFontColor);
    Settings::insertToFontColorHash(&Settings::hashFonts, m_settingsDialog->proseTextFontColor->sampleFont(), Settings::ProseTextFontColor);
    Settings::insertToFontColorHash(&Settings::hashFonts, m_settingsDialog->sectionNameFontColor->sampleFont(), Settings::SectionNameFontColor);
    Settings::insertToFontColorHash(&Settings::hashFonts, m_settingsDialog->titlesFontColor->sampleFont(), Settings::TitlesFontColor);
    Settings::insertToFontColorHash(&Settings::hashFonts, m_settingsDialog->numbersFontColor->sampleFont(), Settings::NumbersFontColor);

    Settings::insertToFontColorHash(&Settings::hashColors, m_settingsDialog->outlineFontColor->color(), Settings::OutLineFontColor);
    Settings::insertToFontColorHash(&Settings::hashColors, m_settingsDialog->globalTextFontColor->color(), Settings::DefaultFontColor);
    Settings::insertToFontColorHash(&Settings::hashColors, m_settingsDialog->poemTextFontColor->color(), Settings::PoemTextFontColor);
    Settings::insertToFontColorHash(&Settings::hashColors, m_settingsDialog->proseTextFontColor->color(), Settings::ProseTextFontColor);
    Settings::insertToFontColorHash(&Settings::hashColors, m_settingsDialog->sectionNameFontColor->color(), Settings::SectionNameFontColor);
    Settings::insertToFontColorHash(&Settings::hashColors, m_settingsDialog->titlesFontColor->color(), Settings::TitlesFontColor);
    Settings::insertToFontColorHash(&Settings::hashColors, m_settingsDialog->numbersFontColor->color(), Settings::NumbersFontColor);

    Settings::WRITE("UseTransparecy", m_settingsDialog->ui->checkBoxTransparent->isChecked());
    QtWin::easyBlurUnBlur(this, Settings::READ("UseTransparecy").toBool());
    QtWin::easyBlurUnBlur(m_settingsDialog, Settings::READ("UseTransparecy").toBool());

    ui->mainToolBar->clear();
    //Inserting main toolbar Items
    for (int i = 0; i < mainToolBarItems.size(); ++i) {
        ui->mainToolBar->addAction(actionInstance(mainToolBarItems.at(i)));
    }

    if (saagharWidget) {
        saagharWidget->loadSettings();

        if (saagharWidget->tableViewWidget->columnCount() == 1 && saagharWidget->tableViewWidget->rowCount() > 0 && saagharWidget->currentCat != 0) {
            QTableWidgetItem* item = saagharWidget->tableViewWidget->item(0, 0);
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
            //  saagharWidget->tableViewWidget->resizeRowsToContents();
        }
    }
#ifndef Q_OS_MAC //This doesn't work on MACX and moved to "SaagharWidget::loadSettings()"
    //apply new text and background color, and also background picture
    loadTabWidgetSettings();
#endif
    saveSettings();//save new settings to disk

    //////////////////////////////////////////////////////
    //set for refreshed all pages
    setAllAsDirty();
    //////////////////////////////////////////////////////

    setUpdatesEnabled(true);

    QString optionsCompare = m_settingsDialog->ui->uiLanguageComboBox->currentText() +
            m_settingsDialog->ui->lineEditDataBasePath->text() +
            m_settingsDialog->ui->lineEditIconTheme->text() +
            (m_settingsDialog->ui->checkBoxIconTheme->isChecked() ? "TRUE" : "FALSE");

    if (m_compareSettings != optionsCompare) {
        QMessageBox::information(this, tr("Need to Relaunch!"), tr("Some of changes are applied after relaunch!"));
        m_compareSettings = optionsCompare;
    }
}

/*static*/
QSettings* SaagharWindow::getSettingsObject()
{
    QString organization = "Pozh";
    QString settingsName = "Saaghar";
    QSettings::Format settingsFormat = QSettings::NativeFormat;

    if (isPortable) {
        settingsFormat = QSettings::IniFormat;
        QSettings::setPath(settingsFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
        organization = ".";
        settingsName = "settings";
    }
    return  new QSettings(settingsFormat, QSettings::UserScope, organization, settingsName);
}

void SaagharWindow::loadGlobalSettings()
{
    QSettings* config = getSettingsObject();

    Settings::LOAD_VARIABLES(config->value("VariableHash").toHash());

    //openedTabs = config->value("openedTabs", "").toStringList();

    SaagharWidget::CurrentViewStyle = (SaagharWidget::PoemViewStyle)Settings::READ("Poem Current View Style", SaagharWidget::SteppedHemistichLine).toInt();

    if (SaagharWidget::CurrentViewStyle != SaagharWidget::OneHemistichLine &&
            SaagharWidget::CurrentViewStyle != SaagharWidget::TwoHemistichLine &&
            SaagharWidget::CurrentViewStyle != SaagharWidget::SteppedHemistichLine) {
        SaagharWidget::CurrentViewStyle = SaagharWidget::SteppedHemistichLine;
    }

    SaagharWidget::maxPoetsPerGroup = config->value("Max Poets Per Group", 12).toInt();

    //search options
    //SaagharWidget::newSearchFlag = config->value("New Search",true).toBool();
    //SaagharWidget::newSearchSkipNonAlphabet  = config->value("New Search non-Alphabet",false).toBool();
    SearchResultWidget::maxItemPerPage  = config->value("Max Search Results Per Page", 100).toInt();
    SearchResultWidget::nonPagedSearch = config->value("SearchNonPagedResults", false).toBool();
    SearchResultWidget::skipVowelSigns = config->value("SearchSkipVowelSigns", false).toBool();
    SearchResultWidget::skipVowelLetters = config->value("SearchSkipVowelLetters", false).toBool();

    SaagharWidget::backgroundImageState = Settings::READ("Background State", true).toBool();
    SaagharWidget::backgroundImagePath = Settings::READ("Background Path", resourcesPath + "/themes/backgrounds/saaghar-pattern_1.png").toString();

    QGanjoorDbBrowser::dataBasePath = config->value("DataBase Path", "").toString().split(";", QString::SkipEmptyParts);

    //The "XB Sols" is embeded in executable.
//  QString fontFamily=config->value("Font Family", "XB Sols").toString();
//  int fontSize=config->value( "Font Size", 18).toInt();
//  QFont fnt(fontFamily,fontSize);
    //SaagharWidget::tableFont = fnt;
    SaagharWidget::showBeytNumbers = config->value("Show Beyt Numbers", true).toBool();
    //SaagharWidget::textColor = Settings::READ("Text Color",QColor(0x23,0x65, 0xFF)).value<QColor>();
    SaagharWidget::matchedTextColor = Settings::READ("Matched Text Color", QColor(225, 0, 225)).value<QColor>();
    SaagharWidget::backgroundColor = Settings::READ("Background Color", QColor(0xFE, 0xFD, 0xF2)).value<QColor>();

    QString firstFamily = QFontDatabase().families().contains("XB Sols") ?
                "XB Sols" : "Droid Arabic Naskh (with DOT)";
    QFont appFont1(firstFamily, 18);
    appFont1.setBold(true);
    //The "Droid Arabic Naskh (with DOT)" is an application font
    QFont appFont2("Droid Arabic Naskh (with DOT)", 8);
    appFont2.setBold(true);

    QHash<QString, QVariant> defaultFonts;

    Settings::insertToFontColorHash(&defaultFonts, appFont1, Settings::DefaultFontColor);
    Settings::insertToFontColorHash(&defaultFonts, appFont1, Settings::PoemTextFontColor);

    Settings::insertToFontColorHash(&defaultFonts, appFont2, Settings::OutLineFontColor);
    appFont2.setPointSize(12);
    Settings::insertToFontColorHash(&defaultFonts, appFont2, Settings::NumbersFontColor);
    appFont2.setPointSize(16);
    Settings::insertToFontColorHash(&defaultFonts, appFont2, Settings::ProseTextFontColor);
    appFont2.setPointSize(18);
    Settings::insertToFontColorHash(&defaultFonts, appFont2, Settings::SectionNameFontColor);
    appFont2.setPointSize(22);
    Settings::insertToFontColorHash(&defaultFonts, appFont2, Settings::TitlesFontColor);

    QHash<QString, QVariant> defaultColors;
    Settings::insertToFontColorHash(&defaultColors, QColor(22, 127, 175), Settings::OutLineFontColor);
    Settings::insertToFontColorHash(&defaultColors, QColor(47, 144, 45), Settings::DefaultFontColor);
    Settings::insertToFontColorHash(&defaultColors, QColor(57, 175, 175), Settings::PoemTextFontColor);
    Settings::insertToFontColorHash(&defaultColors, QColor(2, 118, 190), Settings::ProseTextFontColor);
    Settings::insertToFontColorHash(&defaultColors, QColor(48, 127, 105), Settings::SectionNameFontColor);
    Settings::insertToFontColorHash(&defaultColors, QColor(143, 47, 47), Settings::TitlesFontColor);
    Settings::insertToFontColorHash(&defaultColors, QColor(84, 81, 171), Settings::NumbersFontColor);

    Settings::hashFonts = Settings::READ("Fonts Hash", QVariant(defaultFonts)).toHash();
    Settings::hashColors = Settings::READ("Colors Hash", QVariant(defaultColors)).toHash();

    DataBaseUpdater::setRepositories(Settings::READ("Repositories List").toStringList());
    DataBaseUpdater::keepDownloadedFiles = Settings::READ("Keep Downloaded File", false).toBool();
    DataBaseUpdater::downloadLocation = Settings::READ("Download Location", "").toString();
    DataBaseUpdater::setSaagharWindow(this);

    autoCheckForUpdatesState = config->value("Auto Check For Updates", true).toBool();

    selectedRandomRange = config->value("Selected Random Range", "").toStringList();
    selectedSearchRange = config->value("Selected Search Range", (QStringList() << "0" << "ALL_TITLES")).toStringList();

    randomOpenInNewTab = config->value("Random Open New Tab", false).toBool();

    //initialize default value for "UseTransparecy"
#ifdef Q_OS_WIN
    Settings::READ("UseTransparecy", true);
#else
    Settings::READ("UseTransparecy", false);
#endif

    const QString sepStr = QLatin1String("Separator");
    QStringList defaultActions;
    defaultActions << "outlineDockAction" << sepStr << "actionPreviousPoem" << "actionNextPoem"
                   << "fixedNameUndoAction" << sepStr << "actionFaal" << "actionRandom"
                   << sepStr << "searchToolbarAction" << "bookmarkManagerDockAction"
#ifndef NO_PHONON_LIB
                   << "albumDockAction" << "toggleMusicPlayer"
#endif
                   << sepStr << "actionSettings";
    mainToolBarItems = Settings::READ("Main ToolBar Items", defaultActions.join("|")).toString().split("|", QString::SkipEmptyParts);
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
    p.setColor(QPalette::Text, Settings::getFromColors(Settings::PoemTextFontColor));
    mainTabWidget->setPalette(p);

    outlineTree->setTreeFont(Settings::getFromFonts(Settings::OutLineFontColor));
    outlineTree->setTreeColor(Settings::getFromColors(Settings::OutLineFontColor));
//  QPalette outlinePalette(outlineTree->palette());
//  outlinePalette.setColor(QPalette::Text, Settings::getFromColors(Settings::OutLineFontColor));
//  outlineTree->setPalette(outlinePalette);
}

void SaagharWindow::saveSettings()
{
#ifndef NO_PHONON_LIB
    if (SaagharWidget::musicPlayer) {
        SaagharWidget::musicPlayer->saveAllAlbums();
    }
#endif

    if (SaagharWidget::bookmarks) {
        QFile bookmarkFile(userHomePath + "/bookmarks.xbel");
        if (!bookmarkFile.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Bookmarks"), tr("Can not write the bookmark file %1:\n%2.")
                                 .arg(bookmarkFile.fileName())
                                 .arg(bookmarkFile.errorString()));
        }
        else {
            SaagharWidget::bookmarks->write(&bookmarkFile);
        }
    }

    QSettings* config = getSettingsObject();

    config->setValue("UI Language", Settings::READ("UI Language", "fa"));

#ifndef NO_PHONON_LIB
    SaagharWidget::musicPlayer->savePlayerSettings(config);
#endif

    //config->setValue("Main ToolBar Items", mainToolBarItems.join("|"));

    Settings::WRITE("Poem Current View Style", SaagharWidget::CurrentViewStyle);

    config->setValue("Max Poets Per Group", SaagharWidget::maxPoetsPerGroup);

    Settings::WRITE("Background State", SaagharWidget::backgroundImageState);
    Settings::WRITE("Background Path", SaagharWidget::backgroundImagePath);

//  //font
//  config->setValue("Font Family", SaagharWidget::tableFont.family());
//  config->setValue( "Font Size", SaagharWidget::tableFont.pointSize());

    config->setValue("Show Beyt Numbers", SaagharWidget::showBeytNumbers);

    //colors
    //Settings::WRITE("Text Color", SaagharWidget::textColor);
    Settings::WRITE("Matched Text Color", SaagharWidget::matchedTextColor);
    Settings::WRITE("Background Color", SaagharWidget::backgroundColor);

    config->setValue("Auto Check For Updates", autoCheckForUpdatesState);

    ///////////////////////////////////////////////////
    /////////////////////save state////////////////////
    Settings::WRITE("MainWindowState", saveState(1));
    Settings::WRITE("Mainwindow Geometry", saveGeometry());

    //Settings::WRITE("SplitterState", splitter->saveState());

    QStringList openedTabs;
    //openedTabs.clear();
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
    Settings::WRITE("Opened tabs from last session", openedTabs);
    //config->setValue("openedTabs", openedTabs);

    //database path
    config->setValue("DataBase Path", QDir::toNativeSeparators(QGanjoorDbBrowser::dataBasePath.join(";")));

    //search options
    //config->setValue("New Search", SaagharWidget::newSearchFlag);
    //config->setValue("New Search non-Alphabet", SaagharWidget::newSearchSkipNonAlphabet);
    config->setValue("Max Search Results Per Page", SearchResultWidget::maxItemPerPage);
    config->setValue("SearchNonPagedResults", SearchResultWidget::nonPagedSearch);
    config->setValue("SearchSkipVowelSigns", SearchResultWidget::skipVowelSigns);
    config->setValue("SearchSkipVowelLetters", SearchResultWidget::skipVowelLetters);

    //Search and Random Range
    config->setValue("Selected Random Range", selectedRandomRange);

    QList<QListWidgetItem*> selectedItems = selectSearchRange->getSelectedItemList();

    selectedSearchRange.clear();
    foreach (QListWidgetItem* item, selectedItems) {
        selectedSearchRange << item->data(Qt::UserRole).toString();
    }

    config->setValue("Selected Search Range", selectedSearchRange);

    config->setValue("Random Open New Tab", randomOpenInNewTab);

    Settings::WRITE("Fonts Hash", Settings::hashFonts);
    Settings::WRITE("Colors Hash", Settings::hashColors);

    Settings::WRITE("Repositories List", DataBaseUpdater::repositories());
    Settings::WRITE("Keep Downloaded File", QVariant(DataBaseUpdater::keepDownloadedFiles));
    Settings::WRITE("Download Location", DataBaseUpdater::downloadLocation);

//  QHash<int, QPair<QString, qint64> >::const_iterator it = SaagharWidget::mediaInfoCash.constBegin();

//  bool transectionStarted = false;

//  if (it != SaagharWidget::mediaInfoCash.constEnd())
//  {
//      transectionStarted = true;
//      SaagharWidget::ganjoorDataBase->dBConnection.transaction();
//  }

//  while (it != SaagharWidget::mediaInfoCash.constEnd())
//  {
//      SaagharWidget::ganjoorDataBase->setPoemMediaSource(it.key(), it.value().first);
//      ++it;
//  }

//  if ( transectionStarted )
//  {
//      SaagharWidget::ganjoorDataBase->dBConnection.commit();
//  }


    /////////////////////save state////////////////////
    ///////////////////////////////////////////////////
    //variable hash
    config->setValue("VariableHash", Settings::GET_VARIABLES_VARIANT());
//  QHash<QString, QVariant> vHash = Settings::GET_VARIABLES_VARIANT().toHash();
//  QHash<QString, QVariant>::const_iterator it = vHash.constBegin();
//  while (it != vHash.constEnd())
//  {
//      config->setValue(it.key(), it.value());
//      ++it;
//  }
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
//  if (selectedRanges.isEmpty())
//      return;

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

//void MainWindow::emitReSizeEvent()
//{
//  maybe a Qt BUG
//  before 'QMainWindow::show()' the computation of width of QMainWindow is not correct!
//  resizeEvent(0);
//  eventFilter(saagharWidget->tableViewWidget->viewport(), 0);
//}

//void MainWindow::resizeEvent( QResizeEvent * event )
//{
//  saagharWidget->resizeTable(saagharWidget->tableViewWidget);
//  if (event)
//      QMainWindow::resizeEvent(event);
//}

void SaagharWindow::closeEvent(QCloseEvent* event)
{
    QFontDatabase::removeAllApplicationFonts();
    if (SaagharWidget::lineEditSearchText) {
        SaagharWidget::lineEditSearchText->searchStop();
    }
    saveSettings();
    event->accept();
}

#if QT_VERSION >= 0x050000
void SaagharWindow::paintEvent(QPaintEvent *event)
{
    if (Settings::READ("UseTransparecy").toBool()) {
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
                senderTable->setCurrentItem(item);
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
            //saagharWidget->tableViewWidget->scrollToItem(current);
            //Fixed BUG: sometimes saagharWidget->tableViewWidget != current->tableWidget()
            current->tableWidget()->scrollToItem(current);
        }
    }
}

void SaagharWindow::tableItemPress(QTableWidgetItem* item)
{
    pressedMouseButton = QApplication::mouseButtons();
    if (item) {
        if (pressedMouseButton == Qt::RightButton) {
            qDebug() << "Press->RightClick";
        }

        if (item->isSelected()) {
            qDebug() << "Press->selected";
        }
    }
}

void SaagharWindow::tableItemClick(QTableWidgetItem* item)
{
    QTableWidget* senderTable = item->tableWidget();
    if (!senderTable) {
        return;
    }
    if (item) {
        if (pressedMouseButton == Qt::RightButton) {
            qDebug() << "Click->RightClick";
        }

        if (item->isSelected()) {
            qDebug() << "Click->selected";
        }
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
//  QStringList searchDataList = QStringList();
    QString searchPhraseData = "";//searchData.toString();
    QString searchVerseData = "";
    if (searchDataList.size() == 2) {
        searchPhraseData = searchDataList.at(0);
        searchVerseData = searchDataList.at(1);
    }

    if (searchPhraseData.isEmpty()) {
        searchPhraseData = SaagharWidget::lineEditSearchText->text();
    }
//  if (searchData.isValid() && !searchData.isNull())
//  {
//      searchDataList = searchData.toString().split("|", QString::SkipEmptyParts);

//      searchPhraseData = searchDataList.at(0);
//  }

    bool OK = false;
    int idData = itemData.at(1).toInt(&OK);
    bool noError = false;

    if (OK && SaagharWidget::ganjoorDataBase) {
        noError = true;
    }

    //right click event
    if (pressedMouseButton == Qt::RightButton) {
        //qDebug() << "tableItemClick-Qt::RightButton";
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

    //qDebug() << "emit updateTabsSubMenus()--2306";
    //resolved by signal SaagharWidget::captionChanged()
    //updateTabsSubMenus();
    connect(senderTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(tableItemClick(QTableWidgetItem*)));
}

void SaagharWindow::importDataBase(const QString &fileName, bool* ok)
{
    QSqlDatabase dataBaseObject = QSqlDatabase::database(QGanjoorDbBrowser::dBName/*SaagharWidget::ganjoorDataBase->dBName*/);
    QFileInfo dataBaseFile(/*QSqlDatabase::database("ganjoor.s3db")*/ dataBaseObject.databaseName());
    if (!dataBaseFile.isWritable()) {
        QMessageBox::warning(this, tr("Error!"), tr("You have not write permission to database file, the import procedure can not proceed.\nDataBase Path: %2").arg(dataBaseFile.fileName()));
        if (ok) {
            *ok = false;
        }
        return;
    }
    QList<GanjoorPoet*> poetsConflictList = SaagharWidget::ganjoorDataBase->getConflictingPoets(fileName);

    //SaagharWidget::ganjoorDataBase->dBConnection.transaction();
    dataBaseObject.transaction();

    if (!poetsConflictList.isEmpty()) {
        QMessageBox warnAboutConflict(this);
        warnAboutConflict.setWindowTitle(tr("Warning!"));
        warnAboutConflict.setIcon(QMessageBox::Warning);
        warnAboutConflict.setText(tr("There are some conflict with your installed database. If you continue, these poets will be removed!"));
        QString details = tr("These poets are present in installed database:\n");
        for (int i = 0; i < poetsConflictList.size(); ++i) {
            details += poetsConflictList.at(i)->_Name + "\n";
        }
        warnAboutConflict.setDetailedText(details);
        warnAboutConflict.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        warnAboutConflict.setEscapeButton(QMessageBox::Cancel);
        warnAboutConflict.setDefaultButton(QMessageBox::Cancel);
        int ret = warnAboutConflict.exec();
        if (ret == QMessageBox::Cancel) {
            if (ok) {
                *ok = false;
            }
            return;
        }

        foreach (GanjoorPoet* poet, poetsConflictList) {
            SaagharWidget::ganjoorDataBase->removePoetFromDataBase(poet->_ID);
        }
    }

    //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (SaagharWidget::ganjoorDataBase->importDataBase(fileName)) {
        //SaagharWidget::ganjoorDataBase->dBConnection.commit();
        qDebug() << "before commit";
        dataBaseObject.commit();
        qDebug() << "after commit1";
        if (ok) {
            *ok = true;
        }

        outlineTree->setItems(SaagharWidget::ganjoorDataBase->loadOutlineFromDataBase(0));
        qDebug() << "after commit22";
        //comboBoxSearchRegion->clear();
        selectSearchRange->clear();
        qDebug() << "after commit333";
        QListWidgetItem* item = selectSearchRange->insertRow(0, tr("All Opened Tab"), true, "ALL_OPENED_TAB", Qt::UserRole, true);
        qDebug() << "after commit4444";
        QListWidgetItem* titleSearchItem = selectSearchRange->insertRow(1, tr("Titles"), true, "ALL_TITLES", Qt::UserRole);
        qDebug() << "after commit55555";
        multiSelectObjectInitialize(selectSearchRange, selectedSearchRange, 2);
        qDebug() << "after commit666666";
        item->setCheckState(selectedSearchRange.contains("ALL_OPENED_TAB") ? Qt::Checked : Qt::Unchecked);
        qDebug() << "after commit7777777";
        titleSearchItem->setCheckState(selectedSearchRange.contains("ALL_TITLES") ? Qt::Checked : Qt::Unchecked);
        qDebug() << "after commit88888888";
        setHomeAsDirty();
        //update visible region of searchToolBar
//      bool tmpFlag = labelMaxResultAction->isVisible();
//      labelMaxResultAction->setVisible(!tmpFlag);
//      ui->searchToolBar->update();
//      QApplication::processEvents();
//      labelMaxResultAction->setVisible(tmpFlag);
    }
    else {
        if (ok) {
            *ok = false;
        }
        //SaagharWidget::ganjoorDataBase->dBConnection.rollback();
        dataBaseObject.rollback();
        QMessageBox warning(QMessageBox::Warning, tr("Error!"), tr("There are some errors, the import procedure was not completed"), QMessageBox::Ok
                            , QGanjoorDbBrowser::dbUpdater, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
        warning.exec();
    }
    qDebug() << "end of=" << Q_FUNC_INFO;
    //QApplication::restoreOverrideCursor();
}

void SaagharWindow::highlightTextOnPoem(int poemId, int vorder)
{
    if (saagharWidget->currentPoem != poemId) {
        return;
    }

    QString highlightedText = saagharWidget->highlightCell(vorder);
    emit highlightedTextChanged(highlightedText);
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

    //it has not a friendly view!
//  contextMenu->addSeparator();
//  QMenu *otherActions =   contextMenu->addMenu(tr("Other Actions"));
//  QMap<QString, QAction *>::const_iterator actIterator = allActionMap.constBegin();
//  while (actIterator != allActionMap.constEnd())
//  {
//      if (!mainToolBarItems.contains(actIterator.key()))
//      {
//          otherActions->addAction(actIterator.value());
//      }
//      ++actIterator;
//  }

    QAction* customizeRandom = 0;
    if (mainToolBarItems.contains("actionFaal", Qt::CaseInsensitive) || mainToolBarItems.contains("actionRandom", Qt::CaseInsensitive)) {
        contextMenu->addSeparator();
        customizeRandom = contextMenu->addAction(tr("Customize Faal && Random...")/*, customizeRandomButtons()*/);
    }
//  else
//  {
//      customizeRandom = otherActions->addAction(tr("Customize Faal && Random...")/*, customizeRandomButtons()*/);
//  }

    if (contextMenu->exec(QCursor::pos()) == customizeRandom) {
        customizeRandomDialog();
    }

    delete contextMenu;
}

void SaagharWindow::customizeRandomDialog()
{
    CustomizeRandomDialog* randomSetting = new CustomizeRandomDialog(this, randomOpenInNewTab);

    QListWidgetItem* firstItem = randomSetting->selectRandomRange->insertRow(0, tr("Current tab's subsections"), true, "CURRENT_TAB_SUBSECTIONS", Qt::UserRole, true);
    multiSelectObjectInitialize(randomSetting->selectRandomRange, selectedRandomRange, 1);
    firstItem->setCheckState(selectedRandomRange.contains("CURRENT_TAB_SUBSECTIONS") ? Qt::Checked : Qt::Unchecked);

    if (randomSetting->exec()) {
        randomSetting->acceptSettings(&randomOpenInNewTab);
        //randomSetting->selectRandomRange->updateSelectedLists();
        QList<QListWidgetItem*> selectedItems = randomSetting->selectRandomRange->getSelectedItemList();

        selectedRandomRange.clear();
        foreach (QListWidgetItem* item, selectedItems) {
            selectedRandomRange << item->data(Qt::UserRole).toString();
        }

        qDebug() << "selectedItems-size=" << selectedItems.size();
        qDebug() << "STR=" << randomSetting->selectRandomRange->getSelectedString();
        qDebug() << selectedRandomRange.join("|");
        qDebug() << "randomOpenInNewTab=" << randomOpenInNewTab;
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
        qDebug() << "toolBarIconSize CREATED";
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
        qDebug() << "toolBarButtonStyle CREATED";
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
    actionInstance(Settings::READ("MainToolBar Style", "actionToolBarStyleOnlyIcon").toString())->setChecked(true);
    actionInstance(Settings::READ("MainToolBar Size", "actionToolBarSizeMediumIcon").toString())->setChecked(true);

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
        qDebug() << "toolbar is not recived";
        return;
    }

    bool ok = false;
    int data = action->data().toInt(&ok);
    if (!ok) {
        qDebug() << "data is nor recived";
        return;
    }

    disconnect(action->actionGroup(), SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));

    QString actionId = allActionMap.key(action);
    QString actionType = actionId.contains("Size") ? "SizeType" : "StyleType";
    qDebug() << "actionType=" << actionType;
    if (actionType == "SizeType") {
        toolbar->setIconSize(QSize(data, data));
        Settings::WRITE("MainToolBar Size", actionId);
    }
    else if (actionType == "StyleType") {
        toolbar->setToolButtonStyle((Qt::ToolButtonStyle)data);
        Settings::WRITE("MainToolBar Style", actionId);
    }

    connect(action->actionGroup(), SIGNAL(triggered(QAction*)), this, SLOT(toolbarViewChanges(QAction*)));
}

bool SaagharWindow::eventFilter(QObject* receiver, QEvent* event)
{
//  if (skipSearchToolBarResize)
//  {
//      qDebug() << "skipSearchToolBarResize=TRUE";
//      skipSearchToolBarResize = false;
//      return false;
//  }
//  else
//      skipSearchToolBarResize = true;
    QEvent::Type eventType = QEvent::None;
    if (event) {
        eventType = event->type();
    }

    switch (eventType/*event->type()*/) {
    case QEvent::Resize :
    case QEvent::None : {
//          QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
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
                qDebug() << "isFloating()=" << ui->searchToolBar->isFloating();
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
    //qDebug() << "triggred=" << actionName << "checked=" << checked;
    QString text = action->text();
    text.remove("&");
    if (actionName == "Show Photo at Home") {
        Settings::WRITE(actionName, checked);
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
    else if (actionName == "Lock ToolBars") {
        Settings::WRITE(actionName, checked);
        ui->mainToolBar->setMovable(!checked);
        if (ui->menuToolBar) {
            ui->menuToolBar->setMovable(!checked);
        }
        ui->searchToolBar->setMovable(!checked);
        parentCatsToolBar->setMovable(!checked);
#ifndef NO_PHONON_LIB
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
            SaagharWidget::bookmarks->insertBookmarkList(SaagharWidget::ganjoorDataBase->importGanjoorBookmarks());
            QApplication::restoreOverrideCursor();
        }
    }
    else if (actionName.endsWith("PoemViewStyle")) {
        SaagharWidget::PoemViewStyle newPoemViewStyle = (SaagharWidget::PoemViewStyle)action->data().toInt();
        if (SaagharWidget::CurrentViewStyle != newPoemViewStyle) {
            SaagharWidget::CurrentViewStyle = newPoemViewStyle;
            //setAllAsDirty();
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
        if (!QGanjoorDbBrowser::dbUpdater) {
            QGanjoorDbBrowser::dbUpdater = new DataBaseUpdater(this);
        }

        QtWin::easyBlurUnBlur(QGanjoorDbBrowser::dbUpdater, Settings::READ("UseTransparecy").toBool());

        //connect(QGanjoorDbBrowser::dbUpdater, SIGNAL(installRequest(QString,bool*)), this, SLOT(importDataBase(QString,bool*)));
        QGanjoorDbBrowser::dbUpdater->exec();
    }
    else if (actionName == "actionFaal") {
        openRandomPoem(24, randomOpenInNewTab);// '24' is for Hafez!
    }
    else if (actionName == "actionRandom") {
        int id = 0;
        if (selectedRandomRange.contains("CURRENT_TAB_SUBSECTIONS")) {
            if (saagharWidget) {
                if (saagharWidget->currentPoem != 0) {
                    id = SaagharWidget::ganjoorDataBase->getPoem(saagharWidget->currentPoem)._CatID;
                }
                else {
                    id = saagharWidget->currentCat;
                }
            }
            else {
                id = 0;
            }
        }
        else if (selectedRandomRange.contains("0")) { //all
            id = 0;    //home
        }
        else {
            if (selectedRandomRange.isEmpty()) {
                id = 0;    //all
            }
            else {
                int randIndex = QGanjoorDbBrowser::getRandomNumber(0, selectedRandomRange.size() - 1);
                id = SaagharWidget::ganjoorDataBase->getPoet(selectedRandomRange.at(randIndex).toInt())._CatID;
                qDebug() << "actionData=" << id << "randIndex=" << randIndex << SaagharWidget::ganjoorDataBase->getPoet(selectedRandomRange.at(randIndex))._Name;
            }
        }
        qDebug() << "Random Parent ID=" << id;
        openRandomPoem(id, randomOpenInNewTab);
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
#ifndef NO_PHONON_LIB
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
    qDebug() << "end::updateTabsSubMenus";
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
    //QStringList tabViewData = action->data().toString().split("=", QString::SkipEmptyParts);
//  if (tabViewData.size() == 2 && (tabViewData.at(0) == "PoemID" || tabViewData.at(0) == "CatID") )
//  {
//      bool Ok = false;
//      int id = tabViewData.at(1).toInt(&Ok);
//      if (Ok)
//          newTabForItem(tabViewData.at(0), id, true);
//  }
    menuClosedTabs->removeAction(action);
}

//bool MainWindow::dataFromIdentifier(const QString &identifier, QString *type, int *id)
//{
//  QStringList tabViewData = identifier.split("=", QString::SkipEmptyParts);
//  if (tabViewData.size() == 2 && (tabViewData.at(0) == "PoemID" || tabViewData.at(0) == "CatID") )
//  {
//      bool Ok = false;
//      int ID = tabViewData.at(1).toInt(&Ok);
//      if (Ok)
//      {
//          if (type) *type = tabViewData.at(0);
//          if (id) *id = ID;
//          return true;
//      }
//  }
//  return false;
//}

void SaagharWindow::setHomeAsDirty()
{
    setAsDirty(0);
//  for (int i = 0; i < mainTabWidget->count(); ++i)
//  {
//      SaagharWidget *tmp = getSaagharWidget(i);
//      if (tmp && tmp->currentCat==0)
//      {
//          tmp->setDirty();//needs to be refreshed
//      }
//  }

//  if (saagharWidget && saagharWidget->isDirty())
//      saagharWidget->showHome();
}

void SaagharWindow::setAllAsDirty()
{
    setAsDirty();
//  //set for refreshed all pages
//  for (int i = 0; i < mainTabWidget->count(); ++i)
//  {
//      SaagharWidget *tmp = getSaagharWidget(i);
//      if (tmp)
//          tmp->setDirty();//needs to be refreshed
//  }
//  saagharWidget->refresh();
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
    //bookmarkMainLayout->addWidget(bookmarkFilter);

    bookmarkMainLayout->addWidget(SaagharWidget::bookmarks);
    bookmarkMainLayout->addLayout(bookmarkToolsLayout);//move to bottom of layout! just we want looks similar other dock widgets!
    bookmarkContainer->setLayout(bookmarkMainLayout);

    QString bookmarkFileName = userHomePath + "/bookmarks.xbel";
    QFile bookmarkFile(bookmarkFileName);

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
        mainToolBarItems.removeAll("bookmarkManagerDockAction");
        mainToolBarItems.removeAll("ImportGanjoorBookmarks");
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
                            .arg(bookmarkFileName), QMessageBox::Ok, this, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
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

#ifndef NO_PHONON_LIB
void SaagharWindow::mediaInfoChanged(const QString &fileName, const QString &title, int id)
{
    if (!saagharWidget) {
        return;
    }

    QString mediaTitle = title;
    if (saagharWidget->currentPoem == id) {
        qDebug() << "SetSource-mediaInfoChanged! saagharWidget->currentPoem == id";
    }
    else {
        qDebug() << "!!!!---SetSource-mediaInfoChanged!!!!!!!!!! saagharWidget->currentPoem != id" << title << id;
    }

    //if (saagharWidget->currentPoem == id && SaagharWidget::musicPlayer->source() == fileName)
    qDebug() << "mediaInfoChanged-saagharWidget->currentPoem=" << saagharWidget->currentPoem << "fileName=" << fileName;
    qint64 time = 0;
    if (SaagharWidget::musicPlayer) {
        time = SaagharWidget::musicPlayer->currentTime();
    }
    if (mediaTitle.isEmpty()) {
        qDebug() << "currentLocationList=" << saagharWidget->currentLocationList;
        mediaTitle = saagharWidget->currentLocationList.join(">");
        qDebug() << "mediaTitle11=" << mediaTitle;
        if (!saagharWidget->currentPoemTitle.isEmpty()) {
            mediaTitle += ">" + saagharWidget->currentPoemTitle;
            qDebug() << "mediaTitle22=" << mediaTitle;
        }
    }
    if (SaagharWidget::musicPlayer) {
        SaagharWidget::musicPlayer->insertToAlbum(id/*saagharWidget->currentPoem*/, fileName, mediaTitle, time);
    }
    //SaagharWidget::mediaInfoCash.insert(saagharWidget->currentPoem,QPair<QString, qint64>(fileName, time));
    //saagharWidget->pageMetaInfo.mediaFile = fileName;
}
#endif

void SaagharWindow::createCustomContextMenu(const QPoint &pos)
{
    if (!saagharWidget || !saagharWidget->tableViewWidget/*|| saagharWidget->currentPoem==0*/) {
        return;
    }
    if (skipContextMenu) {
        skipContextMenu = false;
        return;
    }
    qDebug() << "MainWindow--createCustomContextMenu-Pos=" << pos;
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
//  QAction *contextAction;
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
                qDebug() << "mainTabWidget->setCurrentWidget--Bef";
                mainTabWidget->setCurrentWidget(tmp->parentWidget());
                loadAudioForCurrentTab();
                qDebug() << "mainTabWidget->setCurrentWidget--Aft";
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
        QtWin::easyBlurUnBlur(regForm, Settings::READ("UseTransparecy").toBool());
        regForm->show();
    }
    else {
        qDebug() << "formContainer=" << formContainer->objectName();
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
