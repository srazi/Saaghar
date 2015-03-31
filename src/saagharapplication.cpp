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

namespace {
    QThread::Priority TASKS_PRIORITY = QThread::LowPriority;
    static const int NORMAL_TASKS_THREADS = QThread::idealThreadCount();

    static const QString APPLICATION_NAME = QLatin1String("Saaghar");
    static const QString ORGANIZATION_NAME = QLatin1String("Pozh");
    static const QString ORGANIZATION_DOMAIN = QLatin1String("pozh.org");

    static const QString DATABASE_FILE_NAME = QLatin1String("ganjoor.s3db");
    static const QString SETTINGS_FILE_NAME = QLatin1String("settings.ini");
    static const QString BOOKMARKS_FILE_NAME = QLatin1String("bookmarks.xbel");
    static const QString ALBUM_FILE_NAME = QLatin1String("default.sal");

#ifdef Q_OS_MAC
    static const QString PORTABLE_SETTINGS_PATH = QLatin1String("/../Resources/settings.ini");
#else
    static const QString PORTABLE_SETTINGS_PATH = QLatin1String("/settings.ini");
#endif
}

SaagharApplication::SaagharApplication(int &argc, char **argv)
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
}

SaagharApplication *SaagharApplication::instance()
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

QThreadPool *SaagharApplication::tasksThreadPool()
{
    if (!m_tasksThreadPool) {
        m_tasksThreadPool = new QThreadPool(this);

        m_tasksThreadPool->setMaxThreadCount(m_tasksThreads);
    }

    return m_tasksThreadPool;
}

DatabaseBrowser *SaagharApplication::databaseBrowser()
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

SettingsManager *SaagharApplication::settingsManager()
{
    if (!m_settingsManager) {
        m_settingsManager = SettingsManager::instance();
    }

    return m_settingsManager;
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

QSettings* SaagharApplication::getSettingsObject()
{
    QString organization = ORGANIZATION_NAME;
    QString settingsName = APPLICATION_NAME;
    const QSettings::Format settingsFormat = QSettings::IniFormat;

    if (isPortable()) {
        QSettings::setPath(settingsFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
        organization = ".";
        settingsName = "settings";
    }
    else {
        QSettings::setPath(settingsFormat, QSettings::UserScope, QDir::toNativeSeparators(defaultPath(UserDataDir)));
        organization = ".";
        settingsName = "settings";
    }

    return new QSettings(settingsFormat, QSettings::UserScope, organization, settingsName);
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
    QStringList settingsDatabaseDirs = VARS("DataBase Path").split(QLatin1String(";"), QString::SkipEmptyParts);

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
    const QString uiLanguage = getSettingsObject()->value("UI Language", "fa").toString();

    QTranslator* appTranslator = new QTranslator();
    QTranslator* basicTranslator = new QTranslator();

    if (appTranslator->load(QString("saaghar_") + uiLanguage, sApp->defaultPath(SaagharApplication::ResourcesDir))) {
        installTranslator(appTranslator);
        if (basicTranslator->load(QString("qt_") + uiLanguage, sApp->defaultPath(SaagharApplication::ResourcesDir))) {
            installTranslator(basicTranslator);
        }
    }
}

void SaagharApplication::setupInitialValues()
{
    VAR_INIT("SaagharWidget/PoemViewStyle", SaagharWidget::SteppedHemistichLine);
    VAR_INIT("Background State", true);
    VAR_INIT("Background Path", defaultPath(SaagharApplication::ResourcesDir) + LS("/themes/backgrounds/saaghar-pattern_1.png"));
    VAR_INIT("Show Beyt Numbers", true);
    VAR_INIT("Matched Text Color", QColor(225, 0, 225));
    VAR_INIT("Background Color", QColor(0xFE, 0xFD, 0xF2));

    // font & color defaults
    const QString firstFamily = QFontDatabase().families().contains("XB Sols")
            ? LS("XB Sols") : LS("Droid Arabic Naskh (with DOT)");

    QFont appFont1(firstFamily, 18);
    appFont1.setBold(true);

    //The "Droid Arabic Naskh (with DOT)" is an application font
    QString secondFamily = LS("Droid Arabic Naskh (with DOT)");
#ifdef Q_OS_MAC
    secondFamily = firstFamily;
#endif

    QFont appFont2(secondFamily, 8);
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

    VAR_INIT("Fonts Hash", QVariant(defaultFonts));
    VAR_INIT("Colors Hash", QVariant(defaultColors));

    VAR_INIT("Repositories List", QVariant());
    VAR_INIT("Keep Downloaded File", false);
    VAR_INIT("Download Location", QVariant());

#ifdef Q_OS_WIN
    VAR_INIT("UseTransparecy", true);
#else
    VAR_INIT("UseTransparecy", false);
#endif

    VAR_INIT("Show Photo at Home", true);

    VAR_INIT("Icon Theme State", false);
    VAR_INIT("Icon Theme Path", defaultPath(SaagharApplication::ResourcesDir) + LS("/themes/iconsets/light-gray/"));

    VAR_INIT("Global Font", false);

    VAR_INIT("TaskManager", false);
    VAR_INIT("TaskManager/Mode", "NORMAL");
    VAR_INIT("TaskManager/Notification", ProgressManager::DesktopBottomRight);

    VAR_INIT("DataBase Path", QVariant());

        const QString sepStr = QLatin1String("Separator");
    QStringList defaultToolbarActions = QStringList()
            << "outlineDockAction" << sepStr << "actionPreviousPoem" << "actionNextPoem"
            << "fixedNameUndoAction" << sepStr << "actionFaal" << "actionRandom"
            << sepStr << "searchToolbarAction" << "bookmarkManagerDockAction"
#ifdef MEDIA_PLAYER
            << "albumDockAction" << "toggleMusicPlayer"
#endif
            << sepStr << "actionSettings";

    VAR_INIT("Main ToolBar Items", defaultToolbarActions);
    VAR_INIT("Display Splash Screen", true);
    VAR_INIT("Opened tabs from last session", QVariant());
    VAR_INIT("MainWindowState1", QVariant());
    VAR_INIT("Mainwindow Geometry", QVariant());
    VAR_INIT("Auto Check For Updates", true);
    VAR_INIT("Selected Search Range", (QStringList() << "0" << "ALL_TITLES"));
    VAR_INIT("Lock ToolBars", true);
    VAR_INIT("MainToolBar Style", "actionToolBarStyleOnlyIcon");
    VAR_INIT("MainToolBar Size", "actionToolBarSizeMediumIcon");
    VAR_INIT("UI Language", "fa");
    VAR_INIT("Random Open New Tab", false);
    VAR_INIT("Selected Random Range", QVariant());
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
QSettings* config = getSettingsObject();
    SaagharWidget::maxPoetsPerGroup = config->value("Max Poets Per Group", 12).toInt();

    //search options
    SearchResultWidget::maxItemPerPage  = config->value("Max Search Results Per Page", 100).toInt();
    SearchResultWidget::nonPagedSearch = config->value("SearchNonPagedResults", false).toBool();
    SearchResultWidget::skipVowelSigns = config->value("SearchSkipVowelSigns", false).toBool();
    SearchResultWidget::skipVowelLetters = config->value("SearchSkipVowelLetters", false).toBool();

    SaagharWidget::backgroundImageState = VARB("Background State");
    SaagharWidget::backgroundImagePath = VARS("Background Path");

    SaagharWidget::showBeytNumbers = config->value("Show Beyt Numbers", true).toBool();
    SaagharWidget::matchedTextColor = VAR("Matched Text Color").value<QColor>();
    SaagharWidget::backgroundColor = VAR("Background Color").value<QColor>();

    QString firstFamily = QFontDatabase().families().contains("XB Sols") ?
                          "XB Sols" : "Droid Arabic Naskh (with DOT)";
    QFont appFont1(firstFamily, 18);
    appFont1.setBold(true);
    //The "Droid Arabic Naskh (with DOT)" is an application font
    QString secondFamily = "Droid Arabic Naskh (with DOT)";
#ifdef Q_OS_MAC
    secondFamily = firstFamily;
#endif
    QFont appFont2(secondFamily, 8);
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

    Settings::hashFonts = VAR("Fonts Hash").toHash();
    Settings::hashColors = VAR("Colors Hash").toHash();

    DataBaseUpdater::setRepositories(VAR("Repositories List").toStringList());
    DataBaseUpdater::keepDownloadedFiles = VARB("Keep Downloaded File");
    DataBaseUpdater::downloadLocation = VARS("Download Location");

    // application apply settings
    m_notificationPosition = ProgressManager::Position(VARI("TaskManager/Notification"));
    const QString mode = VARS("TaskManager/Mode");

    if (mode == "SLOW") {
        m_tasksThreads = QThread::idealThreadCount() > 1 ? (QThread::idealThreadCount() - 1) : 1;
        TASKS_PRIORITY = QThread::LowPriority;
        m_displayFullNotification = true;
    }
    else if (mode == "FAST") {
        m_tasksThreads = QThread::idealThreadCount();
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
    // TODO: do things with VAR_DECL
    if (m_mainWindow) {
        m_mainWindow->saveSettings();
    }
//    VAR_DECL("MainWindow/State0", saveState());
//    VAR_DECL("MainWindow/Geometry", saveGeometry());

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
    QStringList items = VAR("Main ToolBar Items").toStringList();
    items.removeAll(QLatin1String(""));

    return items;
}

void SaagharApplication::setMainToolBarItems(const QStringList &items)
{
    VAR_DECL("Main ToolBar Items", items);
}

void SaagharApplication::quitSaaghar()
{
    saveSettings();

    quit();
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
    QExtendedSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.setMessageOptions(QRect(QPoint(120, 525), QSize(310, qMin(splash.fontMetrics().height() + 2, 18))),
                             Qt::AlignLeft | Qt::AlignVCenter, QColor(7, 12, 150));
    splash.setProgressBar(mask, 0, 10, Qt::Vertical);
    Tools::setSplashScreen(&splash);

    m_mainWindow = new SaagharWindow;
    m_mainWindow->show();

    splash.finish(m_mainWindow);
}
