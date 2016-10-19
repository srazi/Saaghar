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

#include "saagharapplication.h"
#include "saagharwindow.h"
#include "tools.h"
#include "progressmanager_p.h"
#include "progressmanager.h"
#include "progressview.h"
#include "settings.h"
#include "databasebrowser.h"
#include "searchresultwidget.h"
#include "databaseupdater.h"
#include "settingsmanager.h"
#include "outlinemodel.h"
#include "selectionmanager.h"

#include <QExtendedSplashScreen>

#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include<QMessageBox>
#include <QPointer>
#include <QSettings>
#include <QThreadPool>
#include <QTranslator>

#ifdef Q_OS_WIN
#ifdef STATIC
#include <QtPlugin>
Q_IMPORT_PLUGIN(qsqlite)
#endif
#endif

namespace
{
QThread::Priority TASKS_PRIORITY = QThread::LowPriority;
static const int NORMAL_TASKS_THREADS = qBound(1, (QThread::idealThreadCount() > 2 ? (QThread::idealThreadCount() - 1) : QThread::idealThreadCount()), 4);

static const QString APPLICATION_NAME = QLatin1String("Saaghar");
static const QString ORGANIZATION_NAME = QLatin1String("Pozh");
static const QString ORGANIZATION_DOMAIN = QLatin1String("pozh.org");

static const QString DATABASE_FILE_NAME = QLatin1String("ganjoor.s3db");
static const QString SETTINGS_FILE_NAME = QLatin1String("settings.ini");
static const QString BOOKMARKS_FILE_NAME = QLatin1String("bookmarks.xbel");
static const QString ALBUM_FILE_NAME = QLatin1String("default.sal");
static const QString ALBUM_DIR_NAME = QLatin1String("audio");

#ifdef Q_OS_MAC
static const QString PORTABLE_SETTINGS_PATH = QLatin1String("/../Resources/settings.ini");
#else
static const QString PORTABLE_SETTINGS_PATH = QLatin1String("/settings.ini");
#endif
}

SaagharApplication::SaagharApplication(int &argc, char** argv)
    : QApplication(argc, argv, true),
      m_mainWindow(0),
      m_progressManager(0),
      m_tasksThreadPool(0),
      m_databaseBrowser(0),
      m_settingsManager(0),
      m_tasksThreads(NORMAL_TASKS_THREADS),
      m_displayFullNotification(true),
      m_notificationPosition(ProgressManager::DesktopBottomRight),
      m_isPortable(-1)
{
    setOrganizationName(ORGANIZATION_NAME);
    setApplicationName(APPLICATION_NAME);
    setOrganizationDomain(ORGANIZATION_DOMAIN);

    init();
}

SaagharApplication::~SaagharApplication()
{
    delete m_mainWindow;
    delete m_progressManager;
    delete m_databaseBrowser;
    delete m_splash;
}

SaagharApplication* SaagharApplication::instance()
{
    return static_cast<SaagharApplication*>(QCoreApplication::instance());
}

QString SaagharApplication::defaultPath(SaagharApplication::PathType type)
{
    if (!m_paths.contains(type)) {
        qWarning() << "A wrong path type is requested!" << type;
    }

    return m_paths.value(type);
}

void SaagharApplication::setDefaultPath(SaagharApplication::PathType type, const QString &path)
{
    m_paths.insert(type, path);
}

ProgressManager* SaagharApplication::progressManager()
{
    if (!m_progressManager) {
        m_progressManager = new ProgressManagerPrivate;

        m_progressManager->init();
        m_progressManager->progressView()->setReferenceWidget(m_mainWindow);
        m_progressManager->progressView()->setProgressWidgetVisible(m_displayFullNotification);
        m_progressManager->progressView()->setPosition(m_notificationPosition);
    }

    return m_progressManager;
}

QThreadPool* SaagharApplication::tasksThreadPool()
{
    if (!m_tasksThreadPool) {
        m_tasksThreadPool = new QThreadPool(this);

        m_tasksThreadPool->setMaxThreadCount(m_tasksThreads);
    }

    return m_tasksThreadPool;
}

DatabaseBrowser* SaagharApplication::databaseBrowser()
{
    if (!m_databaseBrowser) {
        if (!m_paths.contains(DatabaseDirs)) {
            qWarning() << "Empty DatabaseDirs!";
            setupDatabasePaths();
        }

        m_databaseBrowser = DatabaseBrowser::instance();
    }

    return m_databaseBrowser;
}

SettingsManager* SaagharApplication::settingsManager()
{
    if (!m_settingsManager) {
        m_settingsManager = SettingsManager::instance();
    }

    return m_settingsManager;
}

OutlineModel* SaagharApplication::outlineModel(const QString &connectionID)
{
    // Maybe filename instead connectionID
    QString theConnectionID = connectionID;
    if (theConnectionID.isEmpty()) {
        theConnectionID = DatabaseBrowser::defaultConnectionId();
    }

    OutlineModel* model = m_outlineModels.value(theConnectionID, 0);
    if (!model) {
        model = new OutlineModel(theConnectionID, sApp);
        m_outlineModels.insert(theConnectionID, model);
    }

    return model;
}

void SaagharApplication::setPriority(QThread* thread)
{
    if (thread) {
        thread->setPriority(TASKS_PRIORITY);
    }
}

bool SaagharApplication::isPortable() const
{
#ifdef SAAGHAR_PORTABLE
    m_isPortable = 1;
    return true;
#endif

    if (m_isPortable ==  -1) {
        QFileInfo portableSettings(QCoreApplication::applicationDirPath() + PORTABLE_SETTINGS_PATH);

        if (portableSettings.exists() && portableSettings.isWritable()) {
            m_isPortable = 1;
        }
        else {
            m_isPortable = 0;
        }
    }

    return (m_isPortable == 1);
}

void SaagharApplication::setupPaths()
{
    if (!m_paths.isEmpty()) {
        qWarning() << "Duplicate call to SaagharApplication::setupPaths()!";
        return;
    }

    QString resourcesPath;
    QString dataPath;

    if (sApp->isPortable()) {
#ifdef Q_OS_MAC
        resourcesPath = QCoreApplication::applicationDirPath() + "/../Resources/";
#else
        resourcesPath = QCoreApplication::applicationDirPath();
#endif

        dataPath = resourcesPath;
    }
    else {
#ifdef Q_OS_WIN
        resourcesPath = QCoreApplication::applicationDirPath();
        dataPath = QDir::homePath() + "/Pozh/Saaghar/";
#endif
#ifdef Q_OS_X11
        resourcesPath = PREFIX"/share/saaghar/";
        dataPath = QDir::homePath() + "/.Pozh/Saaghar/";
#endif
#ifdef Q_OS_MAC
        resourcesPath = QCoreApplication::applicationDirPath() + "/../Resources/";
        dataPath = QDir::homePath() + "/Library/Pozh/Saaghar/";
#endif
    }

    // dirs
    m_paths.insert(ResourcesDir, resourcesPath);
    m_paths.insert(UserDataDir, dataPath);
    m_paths.insert(AlbumDir, (dataPath + "/" + ALBUM_DIR_NAME + "/"));

    // files
    m_paths.insert(SettingsFile, (dataPath + "/" + SETTINGS_FILE_NAME));
    m_paths.insert(BookmarksFile, (dataPath + "/" + BOOKMARKS_FILE_NAME));
    m_paths.insert(AlbumFile, (dataPath + "/" + ALBUM_FILE_NAME));

    QDir saagharUserData(dataPath);
    if (!saagharUserData.exists()) {
        saagharUserData.mkpath(dataPath);
    }
}

void SaagharApplication::setupDatabasePaths()
{
    QStringList settingsDatabaseDirs = VARS("DatabaseBrowser/DataBasePath").split(QLatin1String(";"), QString::SkipEmptyParts);

    //searching database-path for database-file
    //following lines are for support old default data-base pathes.
    QStringList databaseDirs(settingsDatabaseDirs);

    databaseDirs << defaultPath(ResourcesDir)
                 << QDir::homePath() + "/Pojh/Saaghar/"
                 << QDir::homePath() + "/Library/Saaghar/";

    for (int i = 0; i < databaseDirs.size(); ++i) {
        if (QFile::exists(databaseDirs.at(i) + "/" + DATABASE_FILE_NAME)) {
            m_paths.insert(DatabaseFile, (databaseDirs.at(i) + "/" + DATABASE_FILE_NAME));
            settingsDatabaseDirs.removeAll(databaseDirs.at(i));
            settingsDatabaseDirs.prepend(databaseDirs.at(i));
            break;
        }
    }

    m_paths.insert(DatabaseDirs, settingsDatabaseDirs.join(QLatin1String(";")));

    // Set database browser default path
    DatabaseBrowser::setDefaultDatabasename(m_paths.value(DatabaseFile));
}

void SaagharApplication::setupTranslators()
{
    QTranslator* appTranslator = new QTranslator();
    QTranslator* basicTranslator = new QTranslator();

    if (appTranslator->load(QString("saaghar_") + VARS("General/UILanguage"), sApp->defaultPath(SaagharApplication::ResourcesDir))) {
        installTranslator(appTranslator);
        if (basicTranslator->load(QString("qt_") + VARS("General/UILanguage"), sApp->defaultPath(SaagharApplication::ResourcesDir))) {
            installTranslator(basicTranslator);
        }
    }

    if (tr("LTR") == QLatin1String("RTL")) {
        setLayoutDirection(Qt::RightToLeft);
        qDebug() << "Set layout direction to RTL";
    }
}

void SaagharApplication::setupInitialValues()
{
    VAR_INIT("General/DisplaySplashScreen", true);
    VAR_INIT("General/UILanguage", LS("fa"));
    VAR_INIT("General/AutoCheckUpdates", true);

    VAR_INIT("DatabaseBrowser/RepositoriesList", QVariant());
    VAR_INIT("DatabaseBrowser/KeepDownloadedFile", false);
    VAR_INIT("DatabaseBrowser/DownloadLocation", QVariant());
    VAR_INIT("DatabaseBrowser/DataBasePath", QVariant());

    VAR_INIT("SaagharWidget/ShowBeytNumbers", true);
    VAR_INIT("SaagharWidget/MaxPoetsPerGroup", 12);
    VAR_INIT("SaagharWidget/PoemViewStyle", SaagharWidget::SteppedHemistichLine);
    VAR_INIT("SaagharWidget/BackgroundState", true);
    VAR_INIT("SaagharWidget/BackgroundPath", defaultPath(SaagharApplication::ResourcesDir) + LS("/themes/backgrounds/saaghar-pattern_1.png"));
    VAR_INIT("SaagharWidget/UseGlobalTextFormat", false);
    VAR_INIT("SaagharWidget/Colors/MatchedText", QColor(225, 0, 225));
    VAR_INIT("SaagharWidget/Colors/Background", QColor(0xFE, 0xFD, 0xF2));

    // font & color defaults
    const QString firstFamily = QFontDatabase().families().contains(LS("XB Sols"))
                                ? LS("XB Sols") : LS("Droid Arabic Naskh (with DOT)");

    QFont appFont1(firstFamily, 18);
    appFont1.setBold(true);
    appFont1.setStyleStrategy(QFont::PreferAntialias);

    //The "Droid Arabic Naskh (with DOT)" is an application font
    QString secondFamily = LS("Droid Arabic Naskh (with DOT)");
#ifdef Q_OS_MAC
    secondFamily = firstFamily;
#endif

    QFont appFont2(secondFamily, 8);
    appFont2.setBold(true);
    appFont2.setStyleStrategy(QFont::PreferAntialias);

    VAR_INIT("SaagharWidget/Fonts/Default", appFont1);
    VAR_INIT("SaagharWidget/Fonts/PoemText", appFont1);

    VAR_INIT("SaagharWidget/Fonts/OutLine", appFont2);
    appFont2.setPointSize(12);
    VAR_INIT("SaagharWidget/Fonts/Numbers", appFont2);
    appFont2.setPointSize(16);
    VAR_INIT("SaagharWidget/Fonts/ProseText", appFont2);
    appFont2.setPointSize(18);
    VAR_INIT("SaagharWidget/Fonts/SectionName", appFont2);
    appFont2.setPointSize(22);
    VAR_INIT("SaagharWidget/Fonts/Titles", appFont2);

    VAR_INIT("SaagharWidget/Colors/OutLine", QColor(22, 127, 175));
    VAR_INIT("SaagharWidget/Colors/Default", QColor(47, 144, 45));
    VAR_INIT("SaagharWidget/Colors/PoemText", QColor(57, 175, 175));
    VAR_INIT("SaagharWidget/Colors/ProseText", QColor(2, 118, 190));
    VAR_INIT("SaagharWidget/Colors/SectionName", QColor(48, 127, 105));
    VAR_INIT("SaagharWidget/Colors/Titles", QColor(143, 47, 47));
    VAR_INIT("SaagharWidget/Colors/Numbers", QColor(84, 81, 171));

#ifdef Q_OS_WIN
    VAR_INIT("SaagharWindow/UseTransparecy", true);
#else
    VAR_INIT("SaagharWindow/UseTransparecy", false);
#endif

    VAR_INIT("SaagharWindow/ShowPhotoAtHome", true);

    VAR_INIT("SaagharWindow/IconThemeState", false);
    VAR_INIT("SaagharWindow/IconThemePath", defaultPath(SaagharApplication::ResourcesDir) + LS("/themes/iconsets/light-gray/"));

    const QString sepStr = LS("Separator");
    QStringList defaultToolbarActions = QStringList()
                                        << LS("outlineDockAction") << sepStr << LS("actionPreviousPoem") << LS("actionNextPoem")
                                        << LS("fixedNameUndoAction") << sepStr << LS("actionFaal") << LS("actionRandom")
                                        << sepStr << LS("searchToolbarAction") << LS("bookmarkManagerDockAction")
#ifdef MEDIA_PLAYER
                                        << LS("albumDockAction") << LS("toggleMusicPlayer")
#endif
                                        << sepStr << LS("actionSettings");

    VAR_INIT("SaagharWindow/MainToolBarItems", defaultToolbarActions);
    VAR_INIT("SaagharWindow/LastSessionTabs", QVariant());
    VAR_INIT("SaagharWindow/State0", QVariant());
    VAR_INIT("SaagharWindow/Geometry", QVariant());
    VAR_INIT("SaagharWindow/LockToolBars", true);
    VAR_INIT("SaagharWindow/MainToolBarStyle", LS("actionToolBarStyleOnlyIcon"));
    VAR_INIT("SaagharWindow/MainToolBarSize", LS("actionToolBarSizeMediumIcon"));
    VAR_INIT("SaagharWindow/RandomOpenNewTab", false);
    VAR_INIT("SaagharWindow/SelectedRandomRange", QVariant());
    VAR_INIT("SaagharWindow/QuickAccessBookmarks", QVariant());

    VAR_INIT("Search/SkipVowelLetters", false);
    VAR_INIT("Search/SkipVowelSigns", false);
    VAR_INIT("Search/NonPagedResults", false);
    VAR_INIT("Search/SelectedRange", (QStringList() << LS("0") << LS("ALL_TITLES")));
    VAR_INIT("Search/MaxResultsPerPage", 100);

    VAR_INIT("TaskManager/UseAdvancedSettings", false);
    VAR_INIT("TaskManager/Mode", "NORMAL");
    VAR_INIT("TaskManager/Notification", ProgressManager::DesktopBottomRight);

    VAR_INIT("QMusicPlayer/ListOfAlbum", QVariant());
    VAR_INIT("QMusicPlayer/Muted", false);
    VAR_INIT("QMusicPlayer/Volume", 0.4);

    VAR_INIT("AudioRepoDownloader/DownloadAlbumPath", defaultPath(SaagharApplication::AlbumFile));
}

void SaagharApplication::loadSettings()
{
    // first setup paths
    setupPaths();

    QFile file(defaultPath(SettingsFile));

    if (file.exists() && (!file.open(QFile::ReadOnly)
                          || !settingsManager()->loadVariable(&file))) {
        QMessageBox::information(m_mainWindow, tr("Warning!"),
                                 tr("Settings could not be loaded!\nFile: %1\nError: %2")
                                 .arg(QFileInfo(file).absoluteFilePath())
                                 .arg(file.errorString()));
    }

    // before using VAR* macros we have to initialize defaults
    setupInitialValues();

    // ready to setup database path
    setupDatabasePaths();
}

void SaagharApplication::applySettings()
{
    // TODO: do things with VAR()
    SaagharWidget::CurrentViewStyle = (SaagharWidget::PoemViewStyle)VARI("SaagharWidget/PoemViewStyle");

    if (SaagharWidget::CurrentViewStyle != SaagharWidget::OneHemistichLine &&
            SaagharWidget::CurrentViewStyle != SaagharWidget::TwoHemistichLine &&
            SaagharWidget::CurrentViewStyle != SaagharWidget::SteppedHemistichLine) {
        SaagharWidget::CurrentViewStyle = SaagharWidget::SteppedHemistichLine;
    }

    SaagharWidget::maxPoetsPerGroup = VARI("SaagharWidget/MaxPoetsPerGroup");

    //search options
    SearchResultWidget::maxItemPerPage  = VARI("Search/MaxResultsPerPage");
    SearchResultWidget::nonPagedSearch = VARB("Search/NonPagedResults");
    SearchResultWidget::skipVowelSigns = VARB("Search/SkipVowelSigns");
    SearchResultWidget::skipVowelLetters = VARB("Search/SkipVowelLetters");

    SaagharWidget::backgroundImageState = VARB("SaagharWidget/BackgroundState");
    SaagharWidget::backgroundImagePath = VARS("SaagharWidget/BackgroundPath");

    SaagharWidget::showBeytNumbers = VARB("SaagharWidget/ShowBeytNumbers");
    SaagharWidget::matchedTextColor = VAR("SaagharWidget/Colors/MatchedText").value<QColor>();
    SaagharWidget::backgroundColor = VAR("SaagharWidget/Colors/Background").value<QColor>();

    DataBaseUpdater::setRepositories(VAR("DatabaseBrowser/RepositoriesList").toStringList());
    DataBaseUpdater::keepDownloadedFiles = VARB("DatabaseBrowser/KeepDownloadedFile");
    DataBaseUpdater::downloadLocation = VARS("DatabaseBrowser/DownloadLocation");

    // application apply settings
    m_notificationPosition = ProgressManager::Position(VARI("TaskManager/Notification"));
    const QString mode = VARS("TaskManager/Mode");

    if (mode == "SLOW") {
        m_tasksThreads = qBound(1, QThread::idealThreadCount() / 2, 2);
        TASKS_PRIORITY = QThread::LowPriority;
        m_displayFullNotification = true;
    }
    else if (mode == "FAST") {
        m_tasksThreads = qBound(1, (QThread::idealThreadCount() > 4 ? (QThread::idealThreadCount() - 1) : QThread::idealThreadCount()), 8);
        TASKS_PRIORITY = QThread::NormalPriority;
        m_displayFullNotification = false;
    }
    else { // fallback to "NORMAL"
        m_tasksThreads = NORMAL_TASKS_THREADS;
        TASKS_PRIORITY = QThread::LowPriority;
        m_displayFullNotification = true;
    }

    if (m_tasksThreadPool) {
        m_tasksThreadPool->setMaxThreadCount(m_tasksThreads);
    }

    if (m_progressManager) {
        if (sApp->notificationPosition() == ProgressManager::Disabled) {
            delete m_progressManager;
            m_progressManager = 0;
        }
        else {
            m_progressManager->progressView()->setProgressWidgetVisible(m_displayFullNotification);
            m_progressManager->progressView()->setPosition(m_notificationPosition);
        }
    }
}

void SaagharApplication::saveSettings()
{
    if (m_mainWindow) {
        m_mainWindow->saveSettings();
    }

    QFile file(defaultPath(SettingsFile));
    if (!file.open(QFile::WriteOnly)
            || !settingsManager()->writeVariable(&file)) {
        QMessageBox::information(m_mainWindow, tr("Warning!"),
                                 tr("Settings could not be saved!\nFile: %1\nError: %2")
                                 .arg(QFileInfo(file).absoluteFilePath())
                                 .arg(file.errorString()));
    }
}

QStringList SaagharApplication::mainToolBarItems()
{
    QStringList items = VAR("SaagharWindow/MainToolBarItems").toStringList();
    items.removeAll(QLatin1String(""));

    return items;
}

void SaagharApplication::setMainToolBarItems(const QStringList &items)
{
    VAR_DECL("SaagharWindow/MainToolBarItems", items);
}

void SaagharApplication::quitSaaghar()
{
    saveSettings();

    quit();
}

QStringList SaagharApplication::quickAccessBookmarks() const
{
    return VAR("SaagharWindow/QuickAccessBookmarks").toStringList();
}

QAction* SaagharApplication::quickAccessCustomizeAction()
{
    QAction* act = new QAction(tr("Customize this menu..."), 0);
    act->setObjectName("QuickAccessCustomizeAction");
    connect(act, SIGNAL(triggered()), this, SLOT(customizeQuickAccessBookmarks()));

    return act;
}

void SaagharApplication::customizeQuickAccessBookmarks()
{
    if (qobject_cast<QAction*>(sender())) {
        QTimer::singleShot(0, this, SLOT(customizeQuickAccessBookmarks()));
        return;
    }

    SelectionManager* selectionManager = new SelectionManager(0);
    selectionManager->setSelection(VAR("SaagharWindow/QuickAccessBookmarks").toStringList());

    if (selectionManager->exec() == QDialog::Accepted) {
        VAR_DECL("SaagharWindow/QuickAccessBookmarks", selectionManager->selectionPaths());
    }

    selectionManager->deleteLater();
}

void SaagharApplication::init()
{
    // loadSettings() setups paths and loads settings and then
    // setups initial values and finally setups database path
    loadSettings();

    // apply loaded settings
    applySettings();

    // install translators
    setupTranslators();

    ////////////////////
    // Initialize GUI //
    ////////////////////

    //'At Development Stage' message
    //QMessageBox::information(0, QObject::tr("At Development Stage"), QObject::tr("This is an experimental version! Don\'t release it!\nWWW: http://saaghar.pozh.org"));

    QPixmap pixmap(":/resources/images/saaghar-splash.png");
    QPixmap mask(":/resources/images/saaghar-splash-mask.png");
    m_splash = new QExtendedSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
    m_splash->setMessageOptions(QRect(QPoint(120, 525), QSize(310, qMin(m_splash->fontMetrics().height() + 2, 18))),
                                Qt::AlignLeft | Qt::AlignVCenter, QColor(7, 12, 150));
    m_splash->setProgressBar(mask, 0, 10, Qt::Vertical);
    Tools::setSplashScreen(m_splash);

    m_mainWindow = new SaagharWindow;
    m_mainWindow->show();

    if (m_splash) {
        m_splash->finish(m_mainWindow);
    }
}
