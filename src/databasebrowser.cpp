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

#include "databasebrowser.h"
#include "nodatabasedialog.h"
#include "searchresultwidget.h"
#include "tools.h"
#include "concurrenttasks.h"
#include "saagharapplication.h"
#include "settingsmanager.h"
#include "saagharwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QSqlQuery>
#include <QFileInfo>
#include <QFileDialog>
#include <QTime>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QThread>

DatabaseBrowser* DatabaseBrowser::s_instance = 0;
QMultiHash<QThread*, QString> DatabaseBrowser::s_threadConnections;

DataBaseUpdater* DatabaseBrowser::dbUpdater = 0;
QString DatabaseBrowser::s_defaultConnectionId;
QString DatabaseBrowser::s_defaultDatabaseFileName;
bool DatabaseBrowser::s_isDefaultDatabaseSet = false;

const int minNewPoetID = 1001;
const int minNewCatID = 10001;
const int minNewPoemID = 100001;

const int DatabaseVersion = 1;

#ifdef EMBEDDED_SQLITE
QSQLiteDriver* DatabaseBrowser::sqlDriver = 0;
#else
const QString sqlDriver = "QSQLITE";
#endif

static QString threadToString(QThread* thread = 0)
{
    return QString::number((quintptr)(thread ? thread : QThread::currentThread()));
}

DatabaseBrowser::DatabaseBrowser(const QString &sqliteDbCompletePath)
{
    Q_ASSERT(s_instance == 0);

    s_instance = this;

    qRegisterMetaType<SearchResults>("SearchResults");

    setObjectName(QLatin1String("DatabaseBrowser"));

    QFileInfo dBFile(sqliteDbCompletePath);
    QString pathOfDatabase = dBFile.absolutePath();

#ifdef EMBEDDED_SQLITE
    sqlite3* connection;
    sqlite3_open(sqliteDbCompletePath.toUtf8().data(), &connection);
    sqlDriver = new QSQLiteDriver(connection);
#endif

    QString defaultConnectionId = getIdForDataBase(sqliteDbCompletePath);

    while (!dBFile.exists() || !database(defaultConnectionId).open() || !isValid(defaultConnectionId)) {
        if (splashScreen(QWidget)) {
            splashScreen(QWidget)->close();
            Tools::setSplashScreen(0);
        }

        QString errorString = database(defaultConnectionId).lastError().text();

        removeDatabase(defaultConnectionId);

        NoDataBaseDialog noDataBaseDialog(0, Qt::WindowStaysOnTopHint);
        noDataBaseDialog.ui->pathLabel->setText(tr("Data Base Path:") + " " + sqliteDbCompletePath);
        noDataBaseDialog.ui->errorLabel->setText(tr("Error:") + " " + (errorString.isEmpty() ? tr("No Error!") : errorString));

        QtWin::easyBlurUnBlur(&noDataBaseDialog, VARB("SaagharWindow/UseTransparecy"));

        noDataBaseDialog.exec();

        if (noDataBaseDialog.clickedButton() != noDataBaseDialog.ui->exitPushButton) {
            if (noDataBaseDialog.clickedButton() == noDataBaseDialog.ui->selectDataBase) {
                QString dir = QFileDialog::getExistingDirectory(0, tr("Add Path For Data Base"), dBFile.absoluteDir().path(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
                if (!dir.isEmpty()) {
                    dir.replace(QString("\\"), QString("/"));
                    if (!dir.endsWith('/')) {
                        dir += "/";
                    }
                    dBFile.setFile(dir + "/ganjoor.s3db");

                    //don't open it here!
                    defaultConnectionId = getIdForDataBase(dir + "/ganjoor.s3db");

                    pathOfDatabase = dir;
                }
                else {
                    QMessageBox::information(0, tr("Exit"), tr("Saaghar could not find database file and will be closed."));
                    exit(0);
                }
            }
            else if (noDataBaseDialog.clickedButton() == noDataBaseDialog.ui->createDataBaseFromLocal
                     ||
                     noDataBaseDialog.clickedButton() == noDataBaseDialog.ui->createDataBaseFromRemote) {
                //do download job!!
                QFileDialog getDir(0, tr("Select Save Location"), dBFile.absoluteDir().path());
                getDir.setFileMode(QFileDialog::Directory);
                getDir.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks /*| QFileDialog::DontUseNativeDialog*/);
                getDir.setAcceptMode(QFileDialog::AcceptOpen);

                if (getDir.exec() != QFileDialog::Accepted) {
                    QMessageBox::information(0, tr("Exit"), tr("Saaghar could not find database file and will be closed."));
                    exit(0);
                }

                QString dir = "";
                if (!getDir.selectedFiles().isEmpty()) {
                    dir = getDir.selectedFiles().at(0);
                }

                while (dir.isEmpty() || QFile::exists(dir + "/ganjoor.s3db")) {
                    getDir.exec();
                    if (!getDir.selectedFiles().isEmpty()) {
                        dir = getDir.selectedFiles().at(0);
                    }
//                  dir = QFileDialog::getExistingDirectory(0,tr("Select Save Location"), dir.isEmpty() ? dBFile.absoluteDir().path() : dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
                }
                // now dir is not empty and it doesn't contain 'ganjoor.s3db'
                dir.replace(QString("\\"), QString("/"));
                if (!dir.endsWith('/')) {
                    dir += "/";
                }
                dBFile.setFile(dir + "/ganjoor.s3db");

                defaultConnectionId = getIdForDataBase(dir + "/ganjoor.s3db");

                if (!database().open()) {
                    QMessageBox::information(0, tr("Cannot open Database File!"), tr("Cannot open database file, please check if you have write permisson.\nError: %1\nDataBase Path=%2").arg(errorString).arg(dir));
                    exit(1);
                }
                //insert main tables
                bool tablesInserted = createEmptyDataBase(defaultConnectionId);
                qDebug() << "insert main tables: " << tablesInserted;
                if (!tablesInserted) {
                    QFile::remove(dir + "/ganjoor.s3db");
                    exit(1);
                }
                m_addRemoteDataSet = noDataBaseDialog.clickedButton() == noDataBaseDialog.ui->createDataBaseFromRemote;
                QTimer::singleShot(0, this, SLOT(addDataSets()));

                pathOfDatabase = dir;
            }
        }
        else {
            exit(1);
        }
    }

    s_defaultConnectionId = defaultConnectionId;

    // Saaghar just uses its first search path
    sApp->setDefaultPath(SaagharApplication::DatabaseDirs, QDir::toNativeSeparators(pathOfDatabase));

    cachedMaxCatID = cachedMaxPoemID = 0;
}

DatabaseBrowser::~DatabaseBrowser()
{
}

QString DatabaseBrowser::databaseFileFromID(const QString &connectionID)
{
    return connectionID.left(connectionID.lastIndexOf(QLatin1String("/thread:")));
}

DatabaseBrowser* DatabaseBrowser::instance()
{
    if (!s_instance) {
        if (!s_isDefaultDatabaseSet) {
            qFatal("In first place you have to use DatabaseBrowser::setDefaultDatabasename() to set default database.");
            exit(1);
        }
        new DatabaseBrowser(s_defaultDatabaseFileName);
    }

    return s_instance;
}

QSqlDatabase DatabaseBrowser::database(const QString &connectionID, bool open)
{
    return QSqlDatabase::database(connectionID, open);
}

QString DatabaseBrowser::defaultDatabasename()
{
    return s_defaultDatabaseFileName;
}

void DatabaseBrowser::setDefaultDatabasename(const QString &databaseName)
{
    s_isDefaultDatabaseSet = true;
    s_defaultDatabaseFileName = databaseName;
}

bool DatabaseBrowser::isConnected(const QString &connectionID)
{
    return database(connectionID, true).isOpen();
}

bool DatabaseBrowser::isValid(QString connectionID)
{
    QSqlDatabase db = database(connectionID);

    if (db.open()) {
        return db.tables().contains("cat");
    }

    return false;
}

QList<GanjoorPoet*> DatabaseBrowser::getPoets(const QString &connectionID, bool sort)
{
    QList<GanjoorPoet*> poets;
    if (isConnected(connectionID)) {
        QSqlDatabase databaseObject = database(connectionID);

        QSqlQuery q(databaseObject);
        bool descriptionExists = false;
        if (q.exec("SELECT id, name, cat_id, description FROM poet")) {
            descriptionExists = true;
        }
        else {
            q.exec("SELECT id, name, cat_id FROM poet");
        }

        q.first();
        while (q.isValid() && q.isActive()) {
            GanjoorPoet* gPoet = new GanjoorPoet();
            QSqlRecord qrec = q.record();
            if (descriptionExists) {
                gPoet->init(qrec.value(0).toInt(), qrec.value(1).toString(), qrec.value(2).toInt(), qrec.value(3).toString());
            }
            else {
                gPoet->init(qrec.value(0).toInt(), qrec.value(1).toString(), qrec.value(2).toInt(), "");
            }
            poets.append(gPoet);
            if (!q.next()) {
                break;
            }
        }
        if (sort) {
            std::sort(poets.begin(), poets.end(), comparePoetsByName);
        }
    }

    return poets;
}

bool DatabaseBrowser::comparePoetsByName(GanjoorPoet* poet1, GanjoorPoet* poet2)
{
    static bool isEnglish = VARS("General/UILanguage") == LS("en");

    if (isEnglish) {
        return (QString::localeAwareCompare(poet1->_Name, poet2->_Name) < 0);
    }
    else {
        const bool poet1IsRtl = poet1->_Name.isRightToLeft();
        const bool poet2IsRtl = poet2->_Name.isRightToLeft();

        if ((poet1IsRtl && poet2IsRtl) || (!poet1IsRtl && !poet2IsRtl)) {
            return (QString::localeAwareCompare(poet1->_Name, poet2->_Name) < 0);
        }
        else {
            if (poet1IsRtl && !poet2IsRtl) {
                return true;
            }
            else {
                return false;
            }
        }
    }
}

bool DatabaseBrowser::compareCategoriesByName(GanjoorCat* cat1, GanjoorCat* cat2)
{
    static bool isEnglish = VARS("General/UILanguage") == LS("en");

    if (isEnglish) {
        return (QString::localeAwareCompare(cat1->_Text, cat2->_Text) < 0);
    }
    else {
        const bool cat1IsRtl = cat1->_Text.isRightToLeft();
        const bool cat2IsRtl = cat2->_Text.isRightToLeft();

        if ((cat1IsRtl && cat2IsRtl) || (!cat1IsRtl && !cat2IsRtl)) {
            return (QString::localeAwareCompare(cat1->_Text, cat2->_Text) < 0);
        }
        else {
            if (cat1IsRtl && !cat2IsRtl) {
                return true;
            }
            else {
                return false;
            }
        }
    }
}

GanjoorCat DatabaseBrowser::getCategory(int CatID, const QString &connectionID)
{
    GanjoorCat gCat;
    gCat.init();
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        q.exec("SELECT poet_id, text, parent_id, url FROM cat WHERE id = " + QString::number(CatID));
        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            gCat.init(
                CatID,
                (qrec.value(0)).toInt(),
                (qrec.value(1)).toString(),
                (qrec.value(2)).toInt(),
                (qrec.value(3)).toString());
            return gCat;
        }
    }
    return gCat;
}

void DatabaseBrowser::removeThreadsConnections(QObject* obj)
{
    QThread* thread = qobject_cast<QThread*>(obj);

    if (thread) {
        QStringList connections = s_threadConnections.values(thread);

        foreach (const QString &connection, connections) {
            QSqlDatabase::removeDatabase(connection);
        }

#ifdef SAAGHAR_DEBUG
        qDebug() << "thread destroyed:" << thread;
#endif
    }
}

QList<GanjoorCat*> DatabaseBrowser::getSubCategories(int CatID, const QString &connectionID)
{
    QList<GanjoorCat*> lst;
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        q.exec("SELECT poet_id, text, url, ID FROM cat WHERE parent_id = " + QString::number(CatID));
        q.first();
        while (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            GanjoorCat* gCat = new GanjoorCat();
            gCat->init((qrec.value(3)).toInt(), (qrec.value(0)).toInt(), (qrec.value(1)).toString(), CatID, (qrec.value(2)).toString());
            lst.append(gCat);
            if (!q.next()) {
                break;
            }
        }
        if (CatID == 0) {
            std::sort(lst.begin(), lst.end(), compareCategoriesByName);
        }
    }
    return lst;
}

QList<GanjoorCat> DatabaseBrowser::getParentCategories(GanjoorCat Cat, const QString &connectionID)
{
    QList<GanjoorCat> lst;
    if (isConnected(connectionID)) {
        while (!Cat.isNull() && Cat._ParentID != 0) {
            Cat = getCategory(Cat._ParentID, connectionID);
            lst.insert(0, Cat);
        }
        GanjoorCat gCat;
        gCat.init(0, 0, SaagharWidget::rootTitle(), 0, QString());
        lst.insert(0, gCat);
    }
    return lst;
}

QList<GanjoorPoem*> DatabaseBrowser::getPoems(int CatID, const QString &connectionID)
{
    QList<GanjoorPoem*> lst;
    if (isConnected(connectionID)) {
        QString selectQuery = QString("SELECT ID, title, url FROM poem WHERE cat_id = %1 ORDER BY ID").arg(CatID);
        QSqlQuery q(database(connectionID));
        q.exec(selectQuery);
        q.first();
        QSqlRecord qrec;
        while (q.isValid() && q.isActive()) {
            qrec = q.record();
            GanjoorPoem* gPoem = new GanjoorPoem();
            gPoem->init((qrec.value(0)).toInt(), CatID, (qrec.value(1)).toString(), (qrec.value(2)).toString(), false, "");
            lst.append(gPoem);
            if (!q.next()) {
                break;
            }
        }
    }
    return lst;
}

QList<GanjoorVerse*> DatabaseBrowser::getVerses(int PoemID, const QString &connectionID)
{
    return getVerses(PoemID, 0, connectionID);
}

QString DatabaseBrowser::getFirstMesra(int PoemID, const QString &connectionID) //just first Mesra
{
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        QString selectQuery = QString("SELECT vorder, text FROM verse WHERE poem_id = %1 order by vorder LIMIT 1").arg(PoemID);
        q.exec(selectQuery);
        q.first();
        while (q.isValid() && q.isActive()) {
            return Tools::snippedText(q.record().value(1).toString(), "", 0, 12, true);
        }
    }
    return QString();
}

QList<GanjoorVerse*> DatabaseBrowser::getVerses(int PoemID, int Count, const QString &connectionID)
{
    QList<GanjoorVerse*> lst;
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        QString selectQuery = QString("SELECT vorder, position, text FROM verse WHERE poem_id = %1 order by vorder" + (Count > 0 ? " LIMIT " + QString::number(Count) : "")).arg(PoemID);
        q.exec(selectQuery);
        q.first();
        while (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            GanjoorVerse* gVerse = new GanjoorVerse();
            gVerse->init(PoemID, (qrec.value(0)).toInt(), (VersePosition)((qrec.value(1)).toInt()), (qrec.value(2)).toString());
            lst.append(gVerse);
            if (!q.next()) {
                break;
            }
        }
    }
    return lst;
}

GanjoorPoem DatabaseBrowser::getPoem(int PoemID, const QString &connectionID)
{
    GanjoorPoem gPoem;
    gPoem.init();
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        q.exec("SELECT cat_id, title, url FROM poem WHERE ID = " + QString::number(PoemID));
        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            gPoem.init(PoemID, (qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toString(), false, QString(""));
            return gPoem;
        }
    }
    return gPoem;
}

GanjoorPoem DatabaseBrowser::getNextPoem(int PoemID, int CatID, const QString &connectionID)
{
    GanjoorPoem gPoem;
    gPoem.init();
    if (isConnected(connectionID) && PoemID != -1) { // PoemID==-1 when getNextPoem(GanjoorPoem poem) pass null poem
        QSqlQuery q(database(connectionID));
        q.exec(QString("SELECT ID FROM poem WHERE cat_id = %1 AND id>%2 LIMIT 1").arg(CatID).arg(PoemID));
        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            gPoem = getPoem(qrec.value(0).toInt(), connectionID);
            return gPoem;
        }
    }
    return gPoem;
}

GanjoorPoem DatabaseBrowser::getNextPoem(GanjoorPoem poem, const QString &connectionID)
{
    return getNextPoem(poem._ID, poem._CatID, connectionID);
}

GanjoorPoem DatabaseBrowser::getPreviousPoem(int PoemID, int CatID, const QString &connectionID)
{
    GanjoorPoem gPoem;
    gPoem.init();
    if (isConnected(connectionID) && PoemID != -1) { // PoemID==-1 when getPreviousPoem(GanjoorPoem poem) pass null poem
        QSqlQuery q(database(connectionID));
        q.exec(QString("SELECT ID FROM poem WHERE cat_id = %1 AND id<%2 ORDER BY ID DESC LIMIT 1").arg(CatID).arg(PoemID));
        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            gPoem = getPoem(qrec.value(0).toInt(), connectionID);
            return gPoem;
        }
    }
    return gPoem;
}

GanjoorPoem DatabaseBrowser::getPreviousPoem(GanjoorPoem poem, const QString &connectionID)
{
    return getPreviousPoem(poem._ID, poem._CatID, connectionID);
}

GanjoorPoet DatabaseBrowser::getPoetForPoem(int poemID, const QString &connectionID)
{
    const GanjoorPoem poem = getPoem(poemID, connectionID);

    if (poem.isNull()) {
        return GanjoorPoet();
    }

    return getPoetForCat(poem._CatID, connectionID);
}

GanjoorPoet DatabaseBrowser::getPoetForCat(int CatID, const QString &connectionID)
{
    GanjoorPoet gPoet;
    gPoet.init();
    if (CatID == 0) {
        gPoet.init(0, tr("All"), 0);
        return gPoet;
    }
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        q.exec("SELECT poet_id FROM cat WHERE id = " + QString::number(CatID));
        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            return getPoet(qrec.value(0).toInt(), connectionID);
        }
    }
    return gPoet;
}

GanjoorPoet DatabaseBrowser::getPoet(int PoetID, const QString &connectionID)
{
    GanjoorPoet gPoet;
    gPoet.init();
    if (PoetID == 0) {
        gPoet.init(0, tr("All"), 0);
        return gPoet;
    }
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));

        bool descriptionExists = false;
        if (q.exec("SELECT id, name, cat_id, description FROM poet WHERE id = " + QString::number(PoetID))) {
            descriptionExists = true;
        }
        else {
            q.exec("SELECT id, name, cat_id FROM poet WHERE id = " + QString::number(PoetID));
        }

        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            if (descriptionExists) {
                gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), (qrec.value(3)).toString());
            }
            else {
                gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), "");
            }

            return gPoet;
        }
    }
    return gPoet;
}

QString DatabaseBrowser::getPoetDescription(int PoetID, const QString &connectionID)
{
    if (PoetID <= 0) {
        return "";
    }
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        q.exec("SELECT description FROM poet WHERE id = " + QString::number(PoetID));
        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            return (qrec.value(0)).toString();
        }
    }
    return "";
}

QString DatabaseBrowser::getPoemMediaSource(int PoemID, const QString &connectionID)
{
    if (PoemID <= 0) {
        return "";
    }
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        q.exec("SELECT mediasource FROM poem WHERE id = " + QString::number(PoemID));
        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            return (qrec.value(0)).toString();
        }
    }
    return "";
}

void DatabaseBrowser::setPoemMediaSource(int PoemID, const QString &fileName, const QString &connectionID)
{
    if (PoemID <= 0) {
        return;
    }
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        QString strQuery = "";
        if (!database().record("poem").contains("mediasource")) {
            strQuery = QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg("poem").arg("mediasource").arg("NVARCHAR(255)");
            q.exec(strQuery);
            //q.finish();
        }

        strQuery = QString("UPDATE poem SET mediasource = \"%1\" WHERE id = %2 ;").arg(fileName).arg(PoemID);
        //q.exec(strQuery);
        //PRAGMA TABLE_INFO(poem);


        //QString("REPLACE INTO poem (id, cat_id, title, url, mediasource) VALUES (%1, %2, \"%3\", \"%4\", \"%5\");").arg(newPoem->_ID).arg(newPoem->_CatID).arg(newPoem->_Title).arg(newPoem->_Url).arg(fileName);
        //qDebug() << "setPoemMediaSource="<< q.exec(strQuery);
//      q.first();
//      if (q.isValid() && q.isActive())
//      {
//          QSqlRecord qrec = q.record();
//          return (qrec.value(0)).toString();
//      }
    }
    return;
}

GanjoorPoet DatabaseBrowser::getPoet(QString PoetName, const QString &connectionID)
{
    GanjoorPoet gPoet;
    gPoet.init();
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));

        bool descriptionExists = false;
        if (q.exec("SELECT id, name, cat_id, description FROM poet WHERE name = \'" + PoetName + "\'")) {
            descriptionExists = true;
        }
        else {
            q.exec("SELECT id, name, cat_id FROM poet WHERE name = \'" + PoetName + "\'");
        }

        q.first();
        if (q.isValid() && q.isActive()) {
            QSqlRecord qrec = q.record();
            if (descriptionExists) {
                gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), (qrec.value(3)).toString());
            }
            else {
                gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), "");
            }

            return gPoet;
        }
    }
    return gPoet;
}

int DatabaseBrowser::getRandomPoemID(int* CatID, const QString &connectionID)
{
    if (isConnected(connectionID)) {
        QList<GanjoorCat*> subCats = getSubCategories(*CatID, connectionID);
        int randIndex = Tools::getRandomNumber(0, subCats.size());
        if (randIndex >= 0 && randIndex != subCats.size()) { // '==' is when we want to continue with current 'CatID'. notice maybe 'randIndex == subCats.size() == 0'.
            *CatID = subCats.at(randIndex)->_ID;
            return getRandomPoemID(CatID, connectionID);
        }

        QString strQuery = "SELECT MIN(id), MAX(id) FROM poem";
        if (*CatID != 0) {
            strQuery += " WHERE cat_id=" + QString::number(*CatID);
        }
        QSqlQuery q(database(connectionID));
        q.exec(strQuery);
        if (q.next()) {
            QSqlRecord qrec = q.record();
            int minID = qrec.value(0).toInt();
            int maxID = qrec.value(1).toInt();

            if (maxID <= 0 || minID < 0) {
                QList<GanjoorCat*> subCats = getSubCategories(*CatID, connectionID);
                *CatID = subCats.at(Tools::getRandomNumber(0, subCats.size() - 1))->_ID;
                return getRandomPoemID(CatID, connectionID);
            }
            return Tools::getRandomNumber(minID, maxID);
        }
    }
    return -1;
}

QList<GanjoorPoet*> DatabaseBrowser::getDataBasePoets(const QString fileName)
{
    if (!QFile::exists(fileName)) {
        return QList<GanjoorPoet*>();
    }

    QString dataBaseID = getIdForDataBase(fileName);

    if (!database(dataBaseID).open()) {
        QSqlDatabase::removeDatabase(dataBaseID);
        return QList<GanjoorPoet*>();
    }

    return getPoets(dataBaseID, false);
}

QString DatabaseBrowser::getIdForDataBase(const QString &fileName, QThread* thread)
{
    if (thread == 0) {
        thread = QThread::currentThread();
    }

    bool clone = false;
    const QString &longName = Tools::getLongPathName(fileName);
    const QString &connectionID = longName + QLatin1String("/thread:") + threadToString(thread);

    if (!QSqlDatabase::contains(connectionID)) {
        const QStringList &allConnections = QSqlDatabase::connectionNames();

        QString id;
        foreach (id, allConnections) {
            if (id.startsWith(longName)) {
                clone = true;
                break;
            }
        }

        QSqlDatabase db = clone ?
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
                    QSqlDatabase::cloneDatabase(id, connectionID)
#else
                    QSqlDatabase::cloneDatabase(QSqlDatabase::database(id, false), connectionID)
#endif
                  : QSqlDatabase::addDatabase(sqlDriver, connectionID);

        db.setDatabaseName(fileName);
        db.open();

        s_threadConnections.insert(thread, connectionID);

        Q_ASSERT(s_instance != 0);

        connect(thread, SIGNAL(destroyed(QObject*)), s_instance, SLOT(removeThreadsConnections(QObject*)));
    }

    return connectionID;
}

void DatabaseBrowser::removeDatabase(const QString &fileName, QThread* thread)
{
    if (thread == 0) {
        thread = QThread::currentThread();
    }

    QString connectionID = Tools::getLongPathName(fileName) + QLatin1String("/") + threadToString(thread);

    if (QSqlDatabase::contains(connectionID)) {
        QSqlDatabase::removeDatabase(connectionID);

        s_threadConnections.remove(thread, connectionID);
    }
}

QList<GanjoorPoet*> DatabaseBrowser::getConflictingPoets(const QString fileName, const QString &toConnectionID)
{
    QList<GanjoorPoet*> conflictList;
    if (isConnected()) {
        QList<GanjoorPoet*> dataBasePoets = getDataBasePoets(fileName);
        QList<GanjoorPoet*> installedPoets = getPoets(toConnectionID);
        foreach (GanjoorPoet* poet, installedPoets) {
            foreach (GanjoorPoet* dbPoet, dataBasePoets) {
                if (dbPoet->_ID == poet->_ID) {
                    conflictList << poet;
                    break;
                }
            }
        }
    }

    return conflictList;
}

void DatabaseBrowser::removePoetFromDataBase(int PoetID, const QString &connectionID)
{
    if (isConnected(connectionID)) {
        QString strQuery;
        QSqlQuery q(database(connectionID));

        removeCatFromDataBase(getCategory(getPoet(PoetID, connectionID)._CatID, connectionID), connectionID);

        strQuery = "DELETE FROM poet WHERE id=" + QString::number(PoetID);
        q.exec(strQuery);
    }
}

void DatabaseBrowser::removeCatFromDataBase(const GanjoorCat &gCat, const QString &connectionID)
{
    if (gCat.isNull()) {
        return;
    }
    QList<GanjoorCat*> subCats = getSubCategories(gCat._ID, connectionID);
    foreach (GanjoorCat* cat, subCats) {
        removeCatFromDataBase(*cat, connectionID);
    }
    if (isConnected(connectionID)) {
        QString strQuery;
        QSqlQuery q(database(connectionID));
        strQuery = "DELETE FROM verse WHERE poem_id IN (SELECT id FROM poem WHERE cat_id=" + QString::number(gCat._ID) + ")";
        q.exec(strQuery);
        strQuery = "DELETE FROM poem WHERE cat_id=" + QString::number(gCat._ID);
        q.exec(strQuery);
        strQuery = "DELETE FROM cat WHERE id=" + QString::number(gCat._ID);
        q.exec(strQuery);
    }
}

QString DatabaseBrowser::defaultConnectionId()
{
    if (s_defaultConnectionId.isEmpty()) {
        if (!s_instance) {
            if (!s_defaultDatabaseFileName.isEmpty()) {
                s_instance = new DatabaseBrowser(s_defaultDatabaseFileName);
            }
        }

        if (s_defaultConnectionId.isEmpty()) {
            qFatal("There is no a default connection!");
            exit(1);
        }
    }

    return getIdForDataBase(databaseFileFromID(s_defaultConnectionId), QThread::currentThread());
}

int DatabaseBrowser::getNewPoetID(const QString &connectionID)
{
    int newPoetID = -1;

    if (isConnected(connectionID)) {
        QString strQuery = "SELECT MAX(id) FROM poet";
        QSqlQuery q(database(connectionID));
        q.exec(strQuery);

        if (q.first()) {
            bool ok = false;
            newPoetID = q.value(0).toInt(&ok) + 1;
            if (!ok) {
                newPoetID = minNewPoetID + 1;
            }
        }
    }

    if (newPoetID < minNewPoetID) {
        newPoetID = minNewPoetID + 1;
    }

    return newPoetID;
}
int DatabaseBrowser::getNewPoemID(const QString &connectionID)
{
    int newPoemID = -1;

    if (isConnected(connectionID)) {
        QString strQuery = "SELECT MAX(id) FROM poem";
        QSqlQuery q(database(connectionID));
        q.exec(strQuery);

        if (q.first()) {
            bool ok = false;
            newPoemID = q.value(0).toInt(&ok) + 1;
            if (!ok) {
                newPoemID = minNewPoemID + 1;
            }
        }
    }

    if (newPoemID < minNewPoemID) {
        newPoemID = minNewPoemID + 1;
    }

    return newPoemID;
}

int DatabaseBrowser::getNewCatID(const QString &connectionID)
{
    int newCatID = -1;

    if (isConnected(connectionID)) {
        QString strQuery = "SELECT MAX(id) FROM cat";
        QSqlQuery q(database(connectionID));
        q.exec(strQuery);

        if (q.first()) {
            bool ok = false;
            newCatID = q.value(0).toInt(&ok) + 1;
            if (!ok) {
                newCatID = minNewCatID + 1;
            }
        }
    }

    if (newCatID < minNewCatID) {
        newCatID = minNewCatID + 1;
    }

    return newCatID;
}

bool DatabaseBrowser::importDataBase(const QString &fromFileName, const QString &toConnectionID)
{
    if (!QFile::exists(fromFileName)) {
        return false;
    }

    QString connectionID = getIdForDataBase(fromFileName);

    if (!database(connectionID).open() || !isValid(connectionID)) {
        QSqlDatabase::removeDatabase(connectionID);
        return false;
    }

    {
        // start of block
        QList<GanjoorPoet*> poets = getPoets(connectionID, false);

        QString strQuery;
        QSqlQuery queryObject(database(connectionID));
        QMap<int, int> mapPoets;
        QMap<int, int> mapCats;

        foreach (GanjoorPoet* newPoet, poets) {
            bool insertNewPoet = true;

            //skip every null poet(poet without subcat)
            if (!poetHasSubCats(newPoet->_ID, connectionID)) {
                continue;
            }

            GanjoorPoet poet = getPoet(newPoet->_Name, toConnectionID);
            if (!poet.isNull()) { //conflict on Names
                if (poet._ID == newPoet->_ID) {
                    insertNewPoet = false;
                    mapPoets.insert(newPoet->_ID, newPoet->_ID);
                    GanjoorCat poetCat = getCategory(newPoet->_CatID, toConnectionID);
                    if (!poetCat.isNull()) {
                        if (poetCat._PoetID == newPoet->_ID) {
                            mapCats.insert(newPoet->_CatID, newPoet->_CatID);
                        }
                        else {
                            int aRealyNewCatID = getNewCatID(toConnectionID);
                            mapCats.insert(newPoet->_CatID, getNewCatID(toConnectionID));
                            newPoet->_CatID = aRealyNewCatID;
                        }
                    }
                }
                else {
                    mapPoets.insert(newPoet->_ID, poet._ID);
                    mapPoets.insert(newPoet->_CatID, poet._CatID);
                    newPoet->_CatID = poet._CatID;
                }
            }
            else {
                if (!getPoet(newPoet->_ID, toConnectionID).isNull()) { //conflict on IDs
                    int aRealyNewPoetID = getNewPoetID(toConnectionID);
                    mapPoets.insert(newPoet->_ID, aRealyNewPoetID);
                    newPoet->_ID = aRealyNewPoetID;

                    int aRealyNewCatID = getNewCatID(toConnectionID);
                    mapCats.insert(newPoet->_CatID, aRealyNewCatID);
                    newPoet->_CatID = aRealyNewCatID;
                }
                else { //no conflict, insertNew
                    mapPoets.insert(newPoet->_ID, newPoet->_ID);
                    GanjoorCat newPoetCat = getCategory(newPoet->_CatID, toConnectionID);
                    if (newPoetCat.isNull()) {
                        mapCats.insert(newPoet->_CatID, newPoet->_CatID);
                    }
                    else {
                        int aRealyNewCatID = getNewCatID();
                        mapCats.insert(newPoet->_CatID, getNewCatID());
                        newPoet->_CatID = aRealyNewCatID;
                    }
                }
            }

            if (insertNewPoet && isConnected(toConnectionID)) {
                strQuery = QString("INSERT INTO poet (id, name, cat_id, description) VALUES (%1, \"%2\", %3, \"%4\");").arg(newPoet->_ID).arg(newPoet->_Name).arg(newPoet->_CatID).arg(newPoet->_Description);
                QSqlQuery q(database(toConnectionID));
                q.exec(strQuery);
            }
        }

        strQuery = QString("SELECT id, poet_id, text, parent_id, url FROM cat");
        queryObject.exec(strQuery);

        QList<GanjoorCat*> gCatList;
        while (queryObject.next()) {
            GanjoorCat* gCat = new GanjoorCat();
            gCat->init(queryObject.value(0).toInt(), queryObject.value(1).toInt(), queryObject.value(2).toString(), queryObject.value(3).toInt(), queryObject.value(4).toString());
            gCatList.append(gCat);
        }

        foreach (GanjoorCat* newCat, gCatList) {
            newCat->_PoetID = mapPoets.value(newCat->_PoetID, newCat->_PoetID);
            newCat->_ParentID = mapCats.value(newCat->_ParentID, newCat->_ParentID);

            bool insertNewCategory = true;
            GanjoorCat gCat = getCategory(newCat->_ID, toConnectionID);
            if (!gCat.isNull()) {
                if (gCat._PoetID == newCat->_PoetID) {
                    insertNewCategory = false;

                    if (mapCats.value(newCat->_ID, -1) == -1) {
                        mapCats.insert(newCat->_ID, newCat->_ID);
                    }
                }
                else {
                    int aRealyNewCatID = getNewCatID();
                    mapCats.insert(newCat->_ID, aRealyNewCatID);
                    newCat->_ID = aRealyNewCatID;
                }
            }
            else {
                if (mapCats.value(newCat->_ID, -1) == -1) {
                    mapCats.insert(newCat->_ID, newCat->_ID);
                }
            }

            if (insertNewCategory && isConnected(toConnectionID)) {
                strQuery = QString("INSERT INTO cat (id, poet_id, text, parent_id, url) VALUES (%1, %2, \"%3\", %4, \"%5\");").arg(newCat->_ID).arg(newCat->_PoetID).arg(newCat->_Text).arg(newCat->_ParentID).arg(newCat->_Url);
                QSqlQuery q(database(toConnectionID));
                q.exec(strQuery);
            }

            /*if ( getPoet(newCat->_PoetID).isNull() )
            {
                //missing poet
                int poetCat;
                if (newCat->_ParentID == 0)
                    poetCat = newCat->_ID;
                else
                {
                    //this is not good:
                    poetCat = newCat->_ParentID;
                }
                qDebug() << "a new poet should be created";
                this.NewPoet("شاعر " + PoetID.ToString(), PoetID, poetCat);
            }*/
        }

        QMap<int, int> dicPoemID;
        strQuery = "SELECT id, cat_id, title, url FROM poem";
        queryObject.exec(strQuery);
        QList<GanjoorPoem*> poemList;

        while (queryObject.next()) {
            GanjoorPoem* gPoem = new GanjoorPoem();
            gPoem->init(queryObject.value(0).toInt(), queryObject.value(1).toInt(), queryObject.value(2).toString(), queryObject.value(3).toString());
            poemList.append(gPoem);
        }

        foreach (GanjoorPoem* newPoem, poemList) {
            int tmp = newPoem->_ID;
            if (!getPoem(newPoem->_ID, toConnectionID).isNull()) {
                newPoem->_ID = getNewPoemID();
            }
            newPoem->_CatID = mapCats.value(newPoem->_CatID, newPoem->_CatID);
            dicPoemID.insert(tmp, newPoem->_ID);

            if (isConnected(toConnectionID)) {
                strQuery = QString("INSERT INTO poem (id, cat_id, title, url) VALUES (%1, %2, \"%3\", \"%4\");").arg(newPoem->_ID).arg(newPoem->_CatID).arg(newPoem->_Title).arg(newPoem->_Url);
                QSqlQuery q(database(toConnectionID));
                q.exec(strQuery);
            }
        }

        strQuery = "SELECT poem_id, vorder, position, text FROM verse";
        queryObject.exec(strQuery);
        QList<GanjoorVerse*> verseList;

        while (queryObject.next()) {
            GanjoorVerse* gVerse = new GanjoorVerse();
            gVerse->init(queryObject.value(0).toInt(), queryObject.value(1).toInt(), (VersePosition)queryObject.value(2).toInt(), queryObject.value(3).toString());
            verseList.append(gVerse);
        }

        if (isConnected(toConnectionID)) {
            strQuery = "INSERT INTO verse (poem_id, vorder, position, text) VALUES (:poem_id,:vorder,:position,:text)";
            QSqlQuery q(database(toConnectionID));
            q.prepare(strQuery);

            foreach (GanjoorVerse* newVerse, verseList) {
                newVerse->_PoemID = dicPoemID.value(newVerse->_PoemID, newVerse->_PoemID);
                q.bindValue(":poem_id", newVerse->_PoemID);
                q.bindValue(":vorder", newVerse->_Order);
                q.bindValue(":position", newVerse->_Position);
                q.bindValue(":text", newVerse->_Text);
                q.exec();
            }
        }
    } // end of block

    QSqlDatabase::removeDatabase(connectionID);

    emit databaseUpdated(toConnectionID);

    return true;
}

bool DatabaseBrowser::getPoemIDsByPhrase(ConcurrentTask* searchTask, const QString &currentSelectionPath, const QString &currentSelectionPathTitle, const QStringList &phraseList, const QStringList &excludedList,
        bool* Canceled, bool slowSearch, const QString &connectionID)
{
    if (phraseList.isEmpty()) {
        return false;
    }

    QString strQuery;
    QStringList excludeWhenCleaning;
    QString searchQueryPhrase;
//    bool findRhyme = false;

//    if (phraseList.contains("=", Qt::CaseInsensitive)) {
//        findRhyme = true;
//    }

    QString firstPhrase = phraseList.at(0);

    if (!firstPhrase.contains("=")) {
        excludeWhenCleaning << " ";
    }
    else {
        firstPhrase.remove("=");
    }

    QString joiner;
    if (SearchResultWidget::skipVowelSigns) {
        joiner = QLatin1String("%");
    }
    //replace characters that have some variants with anyWord replaceholder!
    //we have not to worry about this replacement, because phraseList.at(0) is tested again!
    bool variantPresent = false;
    if (SearchResultWidget::skipVowelLetters) {
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
        const QRegularExpression variantEXP("[" + Tools::Ve_Variant.join("") + Tools::AE_Variant.join("") + Tools::He_Variant.join("") + Tools::Ye_Variant.join("") + "]+");
#else
        const QRegExp variantEXP("[" + Tools::Ve_Variant.join("") + Tools::AE_Variant.join("") + Tools::He_Variant.join("") + Tools::Ye_Variant.join("") + "]+");
#endif

        if (firstPhrase.contains(variantEXP)) {
            firstPhrase.replace(variantEXP, "%");
            variantPresent = true;
        }
    }

    QStringList anyWordedList = firstPhrase.split("%%", SKIP_EMPTY_PARTS);
    for (int i = 0; i < anyWordedList.size(); ++i) {
        QString subPhrase = anyWordedList.at(i);
        if (SearchResultWidget::skipVowelSigns) {
            subPhrase.remove("%");
        }
        //TODO: remove skipNonAlphabet and replace it with NEAR operator
        //search for firstPhrase then go for other ones
        subPhrase = subPhrase.simplified();
        if (!subPhrase.contains(" ") || variantPresent) {
            subPhrase = Tools::cleanString(subPhrase, excludeWhenCleaning).split("", SKIP_EMPTY_PARTS).join(joiner);
        }
        subPhrase.replace("% %", "%");
        anyWordedList[i] = subPhrase;
    }

    searchQueryPhrase = anyWordedList.join("%");

    QString taskTitle = currentSelectionPathTitle;

    if (currentSelectionPath == "ALL") {
        strQuery = QString("SELECT poem_id, text, vorder FROM verse WHERE text LIKE \'%" + searchQueryPhrase + "%\' ORDER BY poem_id");
    }
    else if (currentSelectionPath == "ALL_TITLES") {
        strQuery = QString("SELECT id, title FROM poem WHERE title LIKE \'%" + searchQueryPhrase + "%\' ORDER BY id");
    }
    else {
        strQuery = QString("SELECT verse.poem_id,verse.text, verse.vorder FROM verse WHERE verse.text LIKE \'%%1%\' AND verse.poem_id IN (SELECT poem.id FROM poem WHERE poem.cat_id IN (%2) ORDER BY poem.id)").arg(searchQueryPhrase).arg(currentSelectionPath);
    }

    taskTitle.prepend(tr("Search: "));

    QVariantHash arguments;
    // TODO: Add local database support to parallel search
    VAR_ADD(arguments, connectionID);
    VAR_ADD(arguments, strQuery);
    VAR_ADD(arguments, currentSelectionPath);
    VAR_ADD(arguments, phraseList);
    VAR_ADD(arguments, excludedList);
    VAR_ADD(arguments, excludeWhenCleaning);
    VAR_ADD(arguments, Canceled);
    VAR_ADD(arguments, slowSearch);
    VAR_ADD(arguments, taskTitle);

    searchTask->start("SEARCH", arguments, true);

    return true;
}

bool DatabaseBrowser::isRadif(const QList<GanjoorVerse*> &verses, const QString &phrase, int verseOrder)
{
    //verseOrder starts from 1 to verses.size()
    if (verseOrder <= 0 || verseOrder > verses.size()) {
        return false;
    }

    //QList<GanjoorVerse *> verses = getVerses(PoemID);

    QString cleanedVerse = Tools::cleanStringFast(verses.at(verseOrder - 1)->_Text, QStringList(""));
    //cleanedVerse = " "+cleanedVerse+" ";//just needed for whole word
    QString cleanedPhrase = Tools::cleanStringFast(phrase, QStringList(""));

    if (!cleanedVerse.contains(cleanedPhrase)) {
        //verses.clear();
        return false;
    }

    QString secondMesra = "";
    QList<int> mesraOrdersForCompare;
    mesraOrdersForCompare  << -1 << -1 << -1;
    int secondMesraOrder = -1;

    switch (verses.at(verseOrder - 1)->_Position) {
    case Right:
    case CenteredVerse1:
        if (verseOrder == verses.size()) { //last single beyt! there is no 'CenteredVerse2' or a database error
            //verses.clear();
            return false;
        }
        if (verses.at(verseOrder)->_Position == Left || verses.at(verseOrder)->_Position == CenteredVerse2) {
            mesraOrdersForCompare[0] = verseOrder;
        }
        else {
            //verses.clear();
            return false;
        }//again database error
        break;
    case Left:
    case CenteredVerse2:
        if (verseOrder == 1) { //there is just one beyt!! more probably a database error
            //verses.clear();
            return false;
        }
        if (verses.at(verseOrder - 2)->_Position == Right || verses.at(verseOrder - 2)->_Position == CenteredVerse1) {
            mesraOrdersForCompare[0] = verseOrder - 2;
        }
        if (verseOrder - 3 >= 0 && (verses.at(verseOrder - 3)->_Position == Left || verses.at(verseOrder - 3)->_Position == CenteredVerse2)) {
            mesraOrdersForCompare[1] = verseOrder - 3;    //mesra above current mesra
        }
        if (verseOrder + 1 <= verses.size() - 1 && (verses.at(verseOrder + 1)->_Position == Left || verses.at(verseOrder + 1)->_Position == CenteredVerse2)) {
            mesraOrdersForCompare[2] = verseOrder + 1;    //mesra above current mesra
        }
        if (mesraOrdersForCompare.at(0) == -1 && mesraOrdersForCompare.at(1) == -1 && mesraOrdersForCompare.at(2) == -1) {
            //verses.clear();
            return false;
        }//again database error
        break;

    default:
        break;
    }

    for (int i = 0; i < 3; ++i) {
        secondMesraOrder = mesraOrdersForCompare.at(i);
        if (secondMesraOrder == -1) {
            continue;
        }
        secondMesra = Tools::cleanStringFast(verses.at(secondMesraOrder)->_Text, QStringList(""));

        if (!secondMesra.contains(cleanedPhrase)) {
            //verses.clear();
            //return false;
            continue;
        }

        QString firstEnding = cleanedVerse.mid(cleanedVerse.lastIndexOf(cleanedPhrase) + cleanedPhrase.size());
        QString secondEnding = secondMesra.mid(secondMesra.lastIndexOf(cleanedPhrase) + cleanedPhrase.size());

        if (firstEnding != "" || secondEnding != "") { //they're not empty or RADIF
            continue;    //return false;
        }

        int tmp1 = cleanedVerse.lastIndexOf(cleanedPhrase) - 1;
        int tmp2 = secondMesra.lastIndexOf(cleanedPhrase) - 1;
        if (tmp1 < 0 || tmp2 < 0) {
            continue;    //return false;
        }

        return true;
    }
    return false;
}

bool DatabaseBrowser::isRhyme(const QList<GanjoorVerse*> &verses, const QString &phrase, int verseOrder)
{
    //verseOrder starts from 1 to verses.size()
    if (verseOrder <= 0 || verseOrder > verses.size()) {
        return false;
    }

    QString cleanedVerse = Tools::cleanStringFast(verses.at(verseOrder - 1)->_Text, QStringList(""));
    QString cleanedPhrase = Tools::cleanStringFast(phrase, QStringList(""));

    if (!cleanedVerse.contains(cleanedPhrase)) {
        //verses.clear();
        return false;
    }

    QString secondMesra = "";
    QList<int> mesraOrdersForCompare;
    mesraOrdersForCompare  << -1 << -1 << -1;
    int secondMesraOrder = -1;

    switch (verses.at(verseOrder - 1)->_Position) {
    case Right:
    case CenteredVerse1:
        if (verseOrder == verses.size()) { //last single beyt! there is no 'CenteredVerse2' or a database error
            //verses.clear();
            return false;
        }
        if (verses.at(verseOrder)->_Position == Left || verses.at(verseOrder)->_Position == CenteredVerse2) {
            mesraOrdersForCompare[0] = verseOrder;
        }
        else {
            //verses.clear();
            return false;
        }//again database error
        break;
    case Left:
    case CenteredVerse2:
        if (verseOrder == 1) { //there is just one beyt!! more probably a database error
            //verses.clear();
            return false;
        }
        if (verses.at(verseOrder - 2)->_Position == Right || verses.at(verseOrder - 2)->_Position == CenteredVerse1) {
            mesraOrdersForCompare[0] = verseOrder - 2;
        }
        if (verseOrder - 3 >= 0 && (verses.at(verseOrder - 3)->_Position == Left || verses.at(verseOrder - 3)->_Position == CenteredVerse2)) {
            mesraOrdersForCompare[1] = verseOrder - 3;    //mesra above current mesra
        }
        if (verseOrder + 1 <= verses.size() - 1 && (verses.at(verseOrder + 1)->_Position == Left || verses.at(verseOrder + 1)->_Position == CenteredVerse2)) {
            mesraOrdersForCompare[2] = verseOrder + 1;    //mesra above current mesra
        }
        if (mesraOrdersForCompare.at(0) == -1 && mesraOrdersForCompare.at(1) == -1 && mesraOrdersForCompare.at(2) == -1) {
            //verses.clear();
            return false;
        }//again database error
        break;

    default:
        break;
    }

    for (int i = 0; i < 3; ++i) {
        secondMesraOrder = mesraOrdersForCompare.at(i);
        if (secondMesraOrder == -1) {
            continue;
        }
        secondMesra = Tools::cleanStringFast(verses.at(secondMesraOrder)->_Text, QStringList(""));

        int indexInSecondMesra = secondMesra.lastIndexOf(cleanedPhrase);
        int offset = cleanedPhrase.size();
        while (indexInSecondMesra < 0) {
            --offset;
            if (offset < 1) {
                break;
            }
            indexInSecondMesra = secondMesra.lastIndexOf(cleanedPhrase.right(offset));
        }
        if (indexInSecondMesra < 0) {
            continue;
        }

        QString firstEnding = cleanedVerse.mid(cleanedVerse.lastIndexOf(cleanedPhrase) + cleanedPhrase.size());
        QString secondEnding = secondMesra.mid(indexInSecondMesra + cleanedPhrase.right(offset).size()); //secondMesra.mid(secondMesra.lastIndexOf(cleanedPhrase)+cleanedPhrase.size());

        if (firstEnding != secondEnding) { //they're not empty or RADIF
            continue;    //return false;
        }

        int tmp1 = cleanedVerse.lastIndexOf(cleanedPhrase) - 1;
        int tmp2 = indexInSecondMesra - 1;
        if (tmp1 < 0 || tmp2 < 0) {
            continue;    //return false;
        }

        if (cleanedVerse.at(tmp1) == secondMesra.at(tmp2) &&
                firstEnding.isEmpty()) { // and so secondEnding
            //if last character before phrase are similar this is not a Rhyme maybe its RADIF
            //or a part!!! of Rhyme.
            //the following algorithm works good for Rhyme with similar spell and diffrent meanings.
            continue;//return false;
        }
        else {
            return true;
        }
    }
    return false;
}

QVariantList DatabaseBrowser::importGanjoorBookmarks(QString connectionID)
{
    QVariantList lst;
    if (isConnected(connectionID)) {
        QString selectQuery = QString("SELECT poem_id, verse_id FROM fav");
        QSqlQuery q(database(connectionID));
        q.exec(selectQuery);
        QSqlRecord qrec;
        while (q.next()) {
            qrec = q.record();
            int poemID = qrec.value(0).toInt();
            int verseID = qrec.value(1).toInt();
            QString comment = QObject::tr("From Desktop Ganjoor");
            GanjoorPoem poem = getPoem(poemID, connectionID);
            if (verseID == -1) {
                verseID = 1;
            }
            QString verseText = getBeyt(poemID, verseID, "[newline]", connectionID).simplified();
            verseText.replace("[newline]", "\n");
            verseText.prepend(poem._Title + "\n");
            QStringList data;
            data << QString::number(poemID) << QString::number(verseID) << verseText << poem._Url + "/#" + QString::number(verseID) << comment;
            lst << data;
        }
    }
    return lst;
}

QString DatabaseBrowser::getBeyt(int poemID, int firstMesraID, const QString &separator, QString connectionID)
{
    QStringList mesras;
    if (isConnected(connectionID)) {
        QSqlQuery q(database(connectionID));
        QString selectQuery = QString("SELECT vorder, position, text FROM verse WHERE poem_id = %1 and vorder >= %2 ORDER BY vorder LIMIT 2").arg(poemID).arg(firstMesraID);
        q.exec(selectQuery);
        QSqlRecord qrec;
        bool centeredVerse1 = false;
        while (q.next()) {
            qrec = q.record();
            VersePosition mesraPosition = VersePosition(qrec.value(1).toInt());
            if (!centeredVerse1) {
                mesras << qrec.value(2).toString();
            }
            bool breakLoop = false;

            switch (mesraPosition) {
            case Single:
            case Paragraph:
                breakLoop = true;
                break;
            case Right:
                break;
            case Left:
                breakLoop = true;
                break;
            case CenteredVerse1:
                centeredVerse1 = true;
                break;
            case CenteredVerse2:
                if (centeredVerse1) {
                    mesras << qrec.value(2).toString();
                }
                breakLoop = true;
                break;
            default:
                break;
            }
            if (breakLoop) {
                break;
            }
        }
    }
    if (!mesras.isEmpty()) {
        return mesras.join(separator);
    }
    return "";
}

bool DatabaseBrowser::poetHasSubCats(int poetID, const QString &connectionID)
{
    if (isConnected(connectionID)) {
        QSqlDatabase dataBaseObject = database(connectionID);

        QSqlQuery q(dataBaseObject);
        q.exec("SELECT id, text FROM cat WHERE poet_id = " + QString::number(poetID));

        q.first();
        if (q.isValid() && q.isActive()) {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

//SearchResults DatabaseBrowser::startSearch(const QString &strQuery, const QSqlDatabase &db,
//        int PoetID, const QStringList &phraseList,
//        const QStringList &excludedList, const QStringList &excludeWhenCleaning,
//        bool* Canceled, bool slowSearch)
//{
//    SearchResults searchResults;

//    if (!isConnected()) {
//        return searchResults;
//    }

//    int andedPhraseCount = phraseList.size();
//    int excludedCount = excludedList.size();
//    int numOfFounded = 0;

//    QSqlQuery q(db);
//#ifdef SAAGHAR_DEBUG
//    int start = QDateTime::currentDateTime().toMSecsSinceEpoch();
//#endif
//    q.exec(strQuery);
//#ifdef SAAGHAR_DEBUG
//    int end = QDateTime::currentDateTime().toMSecsSinceEpoch();
//    int miliSec = end - start;
//    qDebug() << "duration=" << miliSec;
//#endif
//    int numOfNearResult = 0, nextStep = 0, stepLenght = 300;
//    if (slowSearch) {
//        stepLenght = 30;
//    }

//    int lastPoemID = -1;
//    QList<GanjoorVerse*> verses;

//    while (q.next()) {
//        ++numOfNearResult;
//        if (numOfNearResult > nextStep) {
//            nextStep += stepLenght; //500
//            emit searchStatusChanged(DatabaseBrowser::tr("Search Result(s): %1").arg(numOfFounded));
//            QApplication::processEvents(QEventLoop::AllEvents);
//        }

//        QSqlRecord qrec = q.record();
//        int poemID = qrec.value(0).toInt();
////          if (idList.contains(poemID))
////              continue;//we need just first result

//        QString verseText = qrec.value(1).toString();
//        // assume title's order is zero!
//        int verseOrder = (PoetID == -1000 ? 0 : qrec.value(2).toInt());

//        QString foundedVerse = Tools::cleanStringFast(verseText, excludeWhenCleaning);
//        // for whole word option when word is in the start or end of verse
//        foundedVerse = " " + foundedVerse + " ";

//        //excluded list
//        bool excludeCurrentVerse = false;
//        for (int t = 0; t < excludedCount; ++t) {
//            if (foundedVerse.contains(excludedList.at(t))) {
//                excludeCurrentVerse = true;
//                break;
//            }
//        }

//        if (!excludeCurrentVerse) {
//            for (int t = 0; t < andedPhraseCount; ++t) {
//                QString tphrase = phraseList.at(t);
//                if (tphrase.contains("==")) {
//                    tphrase.remove("==");
//                    if (lastPoemID != poemID/* && findRhyme*/) {
//                        lastPoemID = poemID;
//                        int versesSize = verses.size();
//                        for (int j = 0; j < versesSize; ++j) {
//                            delete verses[j];
//                            verses[j] = 0;
//                        }
//                        verses = getVerses(poemID);
//                    }
//                    excludeCurrentVerse = !isRadif(verses, tphrase, verseOrder);
//                    break;
//                }
//                if (tphrase.contains("=")) {
//                    tphrase.remove("=");
//                    if (lastPoemID != poemID/* && findRhyme*/) {
//                        lastPoemID = poemID;
//                        int versesSize = verses.size();
//                        for (int j = 0; j < versesSize; ++j) {
//                            delete verses[j];
//                            verses[j] = 0;
//                        }
//                        verses = getVerses(poemID);
//                    }
//                    excludeCurrentVerse = !isRhyme(verses, tphrase, verseOrder);
//                    break;
//                }
//                if (!tphrase.contains("%")) {
//                    //QChar(71,6): Simple He
//                    //QChar(204,6): Persian Ye
//                    QString YeAsKasre = QString(QChar(71, 6)) + " ";
//                    if (tphrase.contains(YeAsKasre)) {
//                        tphrase.replace(YeAsKasre, QString(QChar(71, 6)) + "\\s*" + QString(QChar(204, 6)) +
//                                        "{0,2}\\s+");

//                        QRegExp anySearch(".*" + tphrase + ".*", Qt::CaseInsensitive);

//                        if (!anySearch.exactMatch(foundedVerse)) {
//                            excludeCurrentVerse = true;
//                            break;
//                        }
//                    }
//                    else {
//                        if (!foundedVerse.contains(tphrase))
//                            //the verse doesn't contain an ANDed phrase
//                            //maybe for ++ and +++ this should be removed
//                        {
//                            excludeCurrentVerse = true;
//                            break;
//                        }
//                    }
//                }
//                else {
//                    tphrase = tphrase.replace("%%", ".*");
//                    tphrase = tphrase.replace("%", "\\S*");
//                    QRegExp anySearch(".*" + tphrase + ".*", Qt::CaseInsensitive);
//                    if (!anySearch.exactMatch(foundedVerse)) {
//                        excludeCurrentVerse = true;
//                        break;
//                    }
//                }
//            }
//        }

//        if (Canceled && *Canceled) {
//            break;
//        }

//        if (excludeCurrentVerse) {
//#ifdef Q_OS_X11
//            QApplication::processEvents(QEventLoop::WaitForMoreEvents , 3);//max wait 3 miliseconds
//#endif
//            continue;
//        }

//        ++numOfFounded;
//        GanjoorPoem gPoem = getPoem(poemID);
//        searchResults.insertMulti(poemID, "verseText=" + verseText + "|poemTitle=" + gPoem._Title + "|poetName=" + getPoetForCat(gPoem._CatID)._Name);
//#ifdef Q_OS_X11
//        QApplication::processEvents(QEventLoop::WaitForMoreEvents , 3);//max wait 3 miliseconds
//#endif

//    }

//    //for the last result
//    emit searchStatusChanged(DatabaseBrowser::tr("Search Result(s): %1").arg(numOfFounded));

//    qDeleteAll(verses);

//    return searchResults;
//}

void DatabaseBrowser::addDataSets()
{
    if (!DatabaseBrowser::dbUpdater) {
        DatabaseBrowser::dbUpdater = new DataBaseUpdater(0, Qt::WindowStaysOnTopHint);
    }

    if (!m_addRemoteDataSet) {
        //select data sets existing
        QStringList fileList = QFileDialog::getOpenFileNames(0, tr("Select data sets to install"), QDir::homePath(), "Supported Files (*.gdb *.s3db *.zip);;Ganjoor DataBase (*.gdb *.s3db);;Compressed Data Sets (*.zip);;All Files (*.*)");
        if (!fileList.isEmpty()) {
            foreach (const QString &file, fileList) {
                DatabaseBrowser::dbUpdater->installItemToDB(file);
            }
        }
    }
    else if (m_addRemoteDataSet) {
        //download dialog
        QtWin::easyBlurUnBlur(DatabaseBrowser::dbUpdater, VARB("SaagharWindow/UseTransparecy"));
        DatabaseBrowser::dbUpdater->exec();
    }
}

bool DatabaseBrowser::createEmptyDataBase(const QString &connectionID)
{
    if (isConnected(connectionID)) {
        QSqlDatabase dataBaseObject = database(connectionID);

        qDebug() << "tabels==" << dataBaseObject.tables(QSql::Tables);
        if (!dataBaseObject.tables(QSql::Tables).isEmpty()) {
            qWarning() << "The Data Base is NOT empty: " << dataBaseObject.databaseName();
            return false;
        }
        dataBaseObject.transaction();
        QSqlQuery q(dataBaseObject);
        q.exec("CREATE TABLE [cat] ([id] INTEGER  PRIMARY KEY NOT NULL,[poet_id] INTEGER  NULL,[text] NVARCHAR(100)  NULL,[parent_id] INTEGER  NULL,[url] NVARCHAR(255)  NULL);");
        q.exec("CREATE TABLE poem (id INTEGER PRIMARY KEY, cat_id INTEGER, title NVARCHAR(255), url NVARCHAR(255));");
        q.exec("CREATE TABLE [poet] ([id] INTEGER  PRIMARY KEY NOT NULL,[name] NVARCHAR(20)  NULL,[cat_id] INTEGER  NULL  NULL, [description] TEXT);");
        q.exec("CREATE TABLE [verse] ([poem_id] INTEGER  NULL,[vorder] INTEGER  NULL,[position] INTEGER  NULL,[text] TEXT  NULL);");
        q.exec("CREATE INDEX cat_pid ON cat(parent_id ASC);");
        q.exec("CREATE INDEX poem_cid ON poem(cat_id ASC);");
        q.exec("CREATE INDEX verse_pid ON verse(poem_id ASC);");
        return dataBaseObject.commit();
    }
    return false;
}

QSqlDatabase DatabaseBrowser::databaseForThread(QThread* thread, const QString &baseConnectionID)
{
    return database(getIdForDataBase(databaseFileFromID(baseConnectionID), thread));
}

int DatabaseBrowser::createCatPathOnNeed(QList<GanjoorCat> &catPath, const QString &description, const QString &connectionID)
{
    int newPoetID = -1;
    int newCatID = -1;


    if (!isConnected(connectionID)) {
        return newCatID;
    }

    for (int i = 0; i < catPath.size(); ++i) {
        GanjoorCat &cat = catPath[i];
        if (cat._ID == -1) {
            if (i == 0) {
                // new poet
                if (newPoetID == -1) {
                    newPoetID = getNewPoetID(connectionID);
                }
                if (newCatID == -1) {
                    newCatID = getNewCatID(connectionID);
                }
                cat._ID = newCatID;
                cat._ParentID = 0;
                cat._PoetID = newPoetID;

                ++newCatID;
                ++newPoetID;

                QString strQuery = QString("INSERT INTO poet (id, name, cat_id, description) VALUES (%1, \"%2\", %3, \"%4\");")
                                   .arg(cat._PoetID).arg(cat._Text).arg(cat._ID).arg(description);
                QSqlQuery q(database(connectionID));
                q.exec(strQuery);

                strQuery = QString("INSERT INTO cat (id, poet_id, text, parent_id, url) VALUES (%1, %2, \"%3\", %4, \"%5\");")
                           .arg(cat._ID).arg(cat._PoetID).arg(cat._Text).arg(cat._ParentID).arg(cat._Url);

                q.exec(strQuery);
            }
            else {
                if (newCatID == -1) {
                    newCatID = getNewCatID(connectionID);
                }
                cat._ID = newCatID;
                cat._ParentID = catPath.at(i - 1)._ID;
                cat._PoetID = catPath.at(i - 1)._PoetID;

                ++newCatID;

                QString strQuery = QString("INSERT INTO cat (id, poet_id, text, parent_id) VALUES (%1, %2, \"%3\", %4);")
                                   .arg(cat._ID).arg(cat._PoetID).arg(cat._Text).arg(cat._ParentID);
                QSqlQuery q(database(connectionID));
                q.exec(strQuery);
            }
        }
    }

    return newCatID;
}

static void debugCatPath(const QList<GanjoorCat> &catPath, const QString &extra)
{
    qDebug() << "\n" << extra << "@@@@@@@ START @@@@@@@@@\n";
    for (int i = 0; i < catPath.size(); ++i) {
        GanjoorCat cat = catPath.at(i);
        qDebug() << (i + 1) << " - " << cat._Text << "\n"
                 << (i + 1) << " - " << cat._PoetID << "\n"
                 << (i + 1) << " - " << cat._ParentID << "\n"
                 << (i + 1) << " - " << cat._ID << "\n";
    }
    qDebug() << extra << "@@@@@@@@ END @@@@@@@@@@\n";
}

void DatabaseBrowser::storeAsDataset(const CatContents &importData, const QList<GanjoorCat> &catPath, bool storeAsGDB, const QString &toConnectionID)
{
    if (!isConnected(toConnectionID) || catPath.isEmpty() || importData.isNull()) {
        return;
    }

    if (storeAsGDB) {
        //
    }

    QSqlDatabase dataBaseObject = database(toConnectionID);
    bool inTransaction = dataBaseObject.transaction();

    QList<GanjoorCat> initCatPath = catPath;
    debugCatPath(initCatPath, "1-BEFORE");
    int newCatID = createCatPathOnNeed(initCatPath, importData.description, toConnectionID);
    debugCatPath(initCatPath, "22-AFTER");

//    struct CatContents {
//        QList<GanjoorPoem> poems;
//        QMap<int, QList<GanjoorVerse> > verses;

//        CatContents() {}
//        bool isNull() const { return poems.isEmpty() || verses.isEmpty(); }
//        void clear() { poems.clear(); verses.clear(); }
//    };
    QString strQuery;
    QSqlQuery qp(database(toConnectionID));

    QSqlQuery qv(database(toConnectionID));
    QString verseQuery = "INSERT INTO verse (poem_id, vorder, position, text) VALUES (:poem_id,:vorder,:position,:text)";
    qv.prepare(verseQuery);

    int newPoemId = getNewPoemID(toConnectionID);
    GanjoorCat topLevelCat = initCatPath.last();
    QHash<int, int> createdCatsIdMap;
    int catId = -1;

    foreach (const GanjoorPoem &poem, importData.poems) {
        QList<GanjoorVerse> verses = importData.verses.value(poem._ID);

        if (verses.isEmpty()) {
            continue;
        }

        QList<GanjoorCat> parentCats = importData.catParents(poem._CatID);

        if (!parentCats.isEmpty()) {
            foreach (const GanjoorCat &cat, parentCats) {
                if (!createdCatsIdMap.contains(cat._ID)) {
                    Q_ASSERT(cat._ParentID == -1 || createdCatsIdMap.contains(cat._ParentID));

                    int parentId = cat._ParentID == -1 ? topLevelCat._ID : createdCatsIdMap.value(cat._ParentID);

                    if (newCatID == -1) {
                        newCatID = getNewCatID(toConnectionID);
                    }

                    createdCatsIdMap.insert(cat._ID, newCatID);
                    catId = newCatID;

                    strQuery = QString("INSERT INTO cat (id, poet_id, text, parent_id) VALUES (%1, %2, \"%3\", %4);")
                               .arg(newCatID).arg(topLevelCat._PoetID).arg(cat._Text).arg(parentId);
                    QSqlQuery q(database(toConnectionID));
                    q.exec(strQuery);

                    ++newCatID;
                }
                else {
                    catId = createdCatsIdMap.value(cat._ID);
                }
            }
        }
        else {
            catId = topLevelCat._ID;
        }

        strQuery = QString("INSERT INTO poem (id, cat_id, title, url) VALUES (%1, %2, \"%3\", \"%4\");")
                   .arg(newPoemId).arg(catId).arg(poem._Title).arg(poem._Url);
        qp.exec(strQuery);

        foreach (const GanjoorVerse &verse, verses) {
            qv.bindValue(":poem_id", newPoemId);
            qv.bindValue(":vorder", verse._Order);
            qv.bindValue(":position", verse._Position);
            qv.bindValue(":text", verse._Text);
            qv.exec();
        }

        ++newPoemId;
    }

    if (!inTransaction || dataBaseObject.commit()) {
        emit databaseUpdated(toConnectionID);
    }
    else {
        dataBaseObject.rollback();
    }
}
