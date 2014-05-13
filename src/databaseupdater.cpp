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

#include <QProgressBar>
#include <QDate>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>

#include "unzip.h"
#include "downloader.h"
#include "databaseupdater.h"
#include "saagharwidget.h"
#include "saagharwindow.h"

QString DataBaseUpdater::downloadLocation;
bool DataBaseUpdater::keepDownloadedFiles = false;
QStringList DataBaseUpdater::repositoriesUrls = QStringList();
const QStringList DataBaseUpdater::defaultRepositories = QStringList()
        << "http://i.ganjoor.net/android/androidgdbs.xml";
//        << "http://ganjoor.sourceforge.net/newgdbs.xml"
//        << "http://ganjoor.sourceforge.net/programgdbs.xml"
//        << "http://ganjoor.sourceforge.net/sitegdbs.xml";

SaagharWindow* DataBaseUpdater::s_saagharWindow = 0;

int PoetID_DATA = Qt::UserRole + 1;
int CatID_DATA = Qt::UserRole + 2;
int FileExt_DATA = Qt::UserRole + 3;
int ImageUrl_DATA = Qt::UserRole + 4;
int LowestPoemID_DATA = Qt::UserRole + 5;
int DownloadUrl_DATA = Qt::UserRole + 6;
int BlogUrl_DATA = Qt::UserRole + 7;

DataBaseUpdater::DataBaseUpdater(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), ui(new Ui::DataBaseUpdater)
{
    downloadStarted = downloadAboutToStart = false;

    ui->setupUi(this);
    ui->refreshPushButton->setIcon(QIcon(ICON_PATH + "/refresh.png"));
    downloaderObject = new Downloader(this, ui->downloadProgressBar, ui->labelDownloadStatus);
    setupUi();

    connect(ui->refreshPushButton, SIGNAL(clicked()), this, SLOT(readRepository()));
    connect(ui->comboBoxRepoList, SIGNAL(currentIndexChanged(QString)), this, SLOT(readRepository(QString)));
    connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
    connect(ui->pushButtonBrowse, SIGNAL(clicked()), this, SLOT(getDownloadLocation()));
    connect(downloaderObject, SIGNAL(downloadStopped()), this, SLOT(forceStopDownload()));
}

bool DataBaseUpdater::read(QIODevice* device)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn)) {
        return false;
    }
    return parseDocument();
}

bool DataBaseUpdater::read(const QByteArray &data)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(data, true, &errorStr, &errorLine, &errorColumn)) {
        return false;
    }
    return parseDocument();
}

bool DataBaseUpdater::parseDocument()
{
    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "DesktopGanjoorGDBList") {
        return false;
    }

    insertedToList.clear();

    QDomElement child = root.firstChildElement("gdb");
    while (!child.isNull()) {
        parseElement(child);
        child = child.nextSiblingElement("gdb");
    }

    resizeColumnsToContents();

    return true;
}

void DataBaseUpdater::parseElement(const QDomElement &element)
{
//visible info
    QString CatName = element.firstChildElement("CatName").text();
    QString BlogUrl = element.firstChildElement("BlogUrl").text();
    QString DownloadUrl = element.firstChildElement("DownloadUrl").text();
    int fileSize = element.firstChildElement("FileSizeInByte").text().toInt();
    fileSize = fileSize / 1024;
    QString fileSizeKB = QString::number(fileSize) == "0" ? "" : QString::number(fileSize) + " " + tr("KB");
    QLocale persainLocale(QLocale::Persian, QLocale::Iran);
    QDate pubDate = persainLocale.toDate(element.firstChildElement("PubDate").text(), "yyyy-MM-dd");

    QStringList visibleInfo = QStringList() << CatName
                              << fileSizeKB
                              << pubDate.toString("yyyy-MM-dd");

//meta data
    QString PoetID = element.firstChildElement("PoetID").text();
    int CatID = element.firstChildElement("CatID").text().toInt();
    QString FileExt = element.firstChildElement("FileExt").text();
    QString ImageUrl = element.firstChildElement("ImageUrl").text();
    QString LowestPoemID = element.firstChildElement("LowestPoemID").text();

    bool isNew = SaagharWidget::ganjoorDataBase->getCategory(CatID).isNull();

    if (insertedToList.contains(CatID) && insertedToList.value(CatID) == DownloadUrl) {
        return;
    }
    else {
        insertedToList.insert(CatID, DownloadUrl);
    }

    QTreeWidgetItem* item = new QTreeWidgetItem(isNew ? newRootItem : oldRootItem , visibleInfo);
    item->setToolTip(0, CatName);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setCheckState(0, Qt::Unchecked);
    item->setData(0, PoetID_DATA, PoetID);
    item->setData(0, CatID_DATA, CatID);
    item->setData(0, FileExt_DATA, FileExt);
    item->setData(0, ImageUrl_DATA, ImageUrl);
    item->setData(0, LowestPoemID_DATA, LowestPoemID);
    item->setData(0, DownloadUrl_DATA, DownloadUrl);
    item->setData(0, BlogUrl_DATA, BlogUrl);

    if (!BlogUrl.isEmpty()) {
        QLabel* blogLink = new QLabel(ui->repoSelectTree);
        blogLink->setTextFormat(Qt::RichText);
        blogLink->setText(QString("<a href=\"%1\" title=\"%1\" >%2</a>").arg(BlogUrl).arg(tr("Go to Release Information")));
        //blogLink->setToolTip(BlogUrl);
        blogLink->setOpenExternalLinks(true);
        ui->repoSelectTree->setItemWidget(item, 3, blogLink);
    }
}

void DataBaseUpdater::fillRepositoryList()
{
    ui->comboBoxRepoList->clear();
    ui->comboBoxRepoList->addItems(QStringList()
                                   << tr("Select From Repositories List...")
                                   << repositoriesUrls
                                   << tr("Click To Add/Remove..."));
    ui->comboBoxRepoList->insertSeparator(1);
    ui->comboBoxRepoList->insertSeparator(ui->comboBoxRepoList->count() - 1);
}

void DataBaseUpdater::setRepositories(const QStringList &urls)
{
    repositoriesUrls.clear();
    repositoriesUrls << urls << defaultRepositories;
    repositoriesUrls.removeDuplicates();
}

QStringList DataBaseUpdater::repositories()
{
    return repositoriesUrls;
}

void DataBaseUpdater::setupUi()
{
#if QT_VERSION >= 0x040700
    ui->lineEditDownloadLocation->setPlaceholderText(tr("Please select download location..."));
#else
    ui->lineEditDownloadLocation->setToolTip(tr("Please select download location..."));
#endif

    fillRepositoryList();

    ui->refreshPushButton->setEnabled(false);

    ui->lineEditDownloadLocation->setText(DataBaseUpdater::downloadLocation);
    keepDownloadedFiles = keepDownloadedFiles && !downloadLocation.isEmpty();
    ui->groupBoxKeepDownload->setChecked(keepDownloadedFiles);
    ui->downloadProgressBar->hide();
    ui->labelDownloadStatus->hide();

    ui->repoSelectTree->setLayoutDirection(Qt::RightToLeft);
    ui->repoSelectTree->setTextElideMode(Qt::ElideMiddle);
}

void DataBaseUpdater::setupTreeRootItems()
{
    QFont font(ui->repoSelectTree->font());
    font.setBold(true);

    oldRootItem = new QTreeWidgetItem(ui->repoSelectTree);
    oldRootItem->setText(0, tr("Installed"));
    oldRootItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
    oldRootItem->setCheckState(0, Qt::Unchecked);
    oldRootItem->setFont(0, font);
    oldRootItem->setExpanded(false);
    oldRootItem->setDisabled(true);

    newRootItem = new QTreeWidgetItem(ui->repoSelectTree);
    newRootItem->setText(0, tr("Ready To Install"));
    newRootItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
    newRootItem->setCheckState(0, Qt::Unchecked);
    newRootItem->setFont(0, font);
    newRootItem->setExpanded(true);
    newRootItem->setDisabled(true);
}

void DataBaseUpdater::readRepository(const QString &url)
{
    disconnect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
    ui->pushButtonDownload->setEnabled(false);

    if (ui->comboBoxRepoList->currentIndex() == ui->comboBoxRepoList->count() - 1) {
        disconnect(ui->comboBoxRepoList, SIGNAL(currentIndexChanged(QString)), this, SLOT(readRepository(QString)));
        addRemoveRepository();
        connect(ui->comboBoxRepoList, SIGNAL(currentIndexChanged(QString)), this, SLOT(readRepository(QString)));
        ui->comboBoxRepoList->setCurrentIndex(-1);//maybe a bug in older Qt, set to '0' does not work!
        ui->comboBoxRepoList->setCurrentIndex(0);
        return;
    }

    ui->repoSelectTree->takeTopLevelItem(1);
    ui->repoSelectTree->takeTopLevelItem(0);

    QUrl repoUrl = QUrl::fromUserInput(ui->comboBoxRepoList->currentText());
    if (!repoUrl.isValid()) {
        qDebug() << "if-repoUrl==" << repoUrl.toString() << "url=" << url;
        ui->refreshPushButton->setEnabled(false);
        return;
    }

    qDebug() << "else-repoUrl==" << repoUrl.toString() << "url=" << url;
    ui->refreshPushButton->setEnabled(true);

    if (!url.isEmpty()) {
        QPair<QTreeWidgetItem*, QTreeWidgetItem*> cachedItems = itemsCache.value(ui->comboBoxRepoList->currentText(), QPair<QTreeWidgetItem*, QTreeWidgetItem*>());
        if (cachedItems.first && cachedItems.second) {
            ui->repoSelectTree->takeTopLevelItem(1);
            ui->repoSelectTree->takeTopLevelItem(0);
            newRootItem = cachedItems.first;
            oldRootItem = cachedItems.second;
            ui->repoSelectTree->addTopLevelItem(oldRootItem);
            ui->repoSelectTree->addTopLevelItem(newRootItem);
            resizeColumnsToContents();
            newRootItem->setExpanded(true);
            itemDataChanged(0, 0);
            connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
            return;
        }
    }
    else {
        itemsCache.insert(ui->comboBoxRepoList->currentText(), QPair<QTreeWidgetItem*, QTreeWidgetItem*>());
    }

    bool isRemote = false;
    QString urlStr = repoUrl.toString();
    qDebug() << "url====" << urlStr;
    setupTreeRootItems();
    if (!urlStr.contains("file:///")) {
        isRemote = true;
        QString tmpPath = getTempDir();
        QDir downDir(tmpPath);
        if (!downDir.exists() && !downDir.mkpath(tmpPath)) {
            QMessageBox::information(this, tr("Error!"), tr("Can not create temp path."));
            return;//continue;
        }

        QFileInfo urlInfo(urlStr);
        ui->comboBoxRepoList->setEnabled(false);
        ui->refreshPushButton->setEnabled(false);
        downloadAboutToStart = downloadStarted = true;
        downloaderObject->downloadFile(repoUrl, tmpPath, urlInfo.fileName());
        downloadAboutToStart = downloadStarted = false;
        ui->comboBoxRepoList->setEnabled(true);
        if (!downloaderObject->hasError()) {
            ui->refreshPushButton->setEnabled(true);
        }
        else {
            //maybe a bug in older Qt, set to '0' does not work!
            ui->comboBoxRepoList->setCurrentIndex(-1);
            ui->comboBoxRepoList->setCurrentIndex(0);
        }
        ui->labelDownloadStatus->hide();
        QString filepath = tmpPath + "/" + urlInfo.fileName();
        QFile file(filepath);
        read(&file);
        file.remove();
        downDir.rmdir(tmpPath);
    }
    else {
        urlStr.remove("file:///");
        QFile file(urlStr);
        read(&file);
    }

    newRootItem->setDisabled(newRootItem->childCount() == 0);
    oldRootItem->setDisabled(oldRootItem->childCount() == 0);
    if (newRootItem->childCount() == 0 && oldRootItem->childCount() == 0) {
        ui->repoSelectTree->clear();
        newRootItem = 0;
        oldRootItem = 0;
    }
    itemsCache.insert(ui->comboBoxRepoList->currentText(), QPair<QTreeWidgetItem*, QTreeWidgetItem*>(newRootItem, oldRootItem));
    connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
}

void DataBaseUpdater::itemDataChanged(QTreeWidgetItem* /*item*/, int /*column*/)
{
    bool itemsChecked = (newRootItem->checkState(0) != Qt::Unchecked) || (oldRootItem->checkState(0) != Qt::Unchecked);
    ui->pushButtonDownload->setEnabled(itemsChecked);
}

void DataBaseUpdater::getDownloadLocation()
{
    DataBaseUpdater::downloadLocation = QFileDialog::getExistingDirectory(this, tr("Select Download Location"), QDir::homePath());
    ui->lineEditDownloadLocation->setText(DataBaseUpdater::downloadLocation);
}

void DataBaseUpdater::initDownload()
{
    downloadAboutToStart = true;
    ui->pushButtonDownload->setText(tr("Stop"));
    disconnect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(doStopDownload()));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), downloaderObject->loop, SLOT(quit()));

    sessionDownloadFolder = DataBaseUpdater::downloadLocation;
    QDir downDir;
    if (ui->groupBoxKeepDownload->isChecked()) {
        if (sessionDownloadFolder.isEmpty()) {
            QMessageBox::information(this, tr("Warning!"), tr("Please select download location."));
            doStopDownload();
            return;
        }
    }
    else {
        randomFolder = sessionDownloadFolder = getTempDir();
    }

    downDir.setPath(sessionDownloadFolder);
    if (!downDir.exists() && !downDir.mkpath(sessionDownloadFolder)) {
        QMessageBox::information(this, tr("Error!"), tr("Can not create download path."));
        doStopDownload();
        return;
    }

    ui->repoSelectTree->setEnabled(false);
    ui->comboBoxRepoList->setEnabled(false);
    ui->refreshPushButton->setEnabled(false);
    ui->groupBoxKeepDownload->setEnabled(false);

    for (int i = 0; i < newRootItem->childCount(); ++i) {
        QTreeWidgetItem* child = newRootItem->child(i);
        if (!child || child->checkState(0) != Qt::Checked) {
            continue;
        }
        installCompleted = false;
        downloadItem(child, true);

        if (installCompleted) {
            //after install we change the parent
            newRootItem->removeChild(child);
            oldRootItem->addChild(child);
            child->setCheckState(0, Qt::Unchecked);
            child->setFlags(Qt::NoItemFlags);
            --i;
        }
    }

    for (int i = 0; i < oldRootItem->childCount(); ++i) {
        QTreeWidgetItem* child = oldRootItem->child(i);
        if (!child || child->checkState(0) != Qt::Checked) {
            continue;
        }
        installCompleted = false;
        downloadItem(child, true);

        if (installCompleted) {
            child->setCheckState(0, Qt::Unchecked);
            child->setFlags(Qt::NoItemFlags);
        }
    }

    if (!ui->groupBoxKeepDownload->isChecked()) {
        //TODO: Fix me!
        //QFile::remove(sessionDownloadFolder+"/"+fileName);
        QDir downDir(sessionDownloadFolder);
        downDir.rmdir(sessionDownloadFolder);
    }

    ui->labelDownloadStatus->setText(tr("Finished!"));

    //download ended
    downloadAboutToStart = downloadStarted = false;
    doStopDownload();

    ui->repoSelectTree->setEnabled(true);
    ui->comboBoxRepoList->setEnabled(true);
    ui->refreshPushButton->setEnabled(true);
    ui->groupBoxKeepDownload->setEnabled(true);
}

bool DataBaseUpdater::doStopDownload()
{
    if (!downloadAboutToStart) {
        return true;
    }

    if (downloadStarted) {
        if (QMessageBox::question(this, tr("Warning!"),
             tr("Download in progress! Are you sure to stop downloading?"),
             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ==
                QMessageBox::No) {
            return false;
        }
    }
    downloaderObject->cancelDownload();
    forceStopDownload();

    return true;
}

void DataBaseUpdater::closeEvent(QCloseEvent* e)
{
    DataBaseUpdater::downloadLocation = ui->lineEditDownloadLocation->text();
    keepDownloadedFiles = !downloadLocation.isEmpty() && ui->groupBoxKeepDownload->isChecked();
    if (!doStopDownload()) {
        e->ignore();    //download in progress
    }
}

void DataBaseUpdater::downloadItem(QTreeWidgetItem* item, bool install)
{
    if (!item) {
        return;
    }
    item->treeWidget()->setCurrentItem(item);
    bool keepDownlaodedFiles = ui->groupBoxKeepDownload->isChecked();
    downloadStarted = true;
    QString urlStr = item->data(0, DownloadUrl_DATA).toString();
    QUrl downloadUrl(urlStr);
    downloaderObject->downloadFile(downloadUrl, sessionDownloadFolder, item->text(0));

    QFileInfo fileInfo(downloadUrl.path());
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty()) {
        fileName = "index.html";
    }

    if (install) {
        ui->labelDownloadStatus->setText(tr("Installing..."));
        QString fileType = item->data(0, FileExt_DATA).toString();
        installItemToDB(fileName, sessionDownloadFolder, fileType);
        ui->labelDownloadStatus->setText(tr("Installed."));
    }

    if (!keepDownlaodedFiles) {
        QFile::remove(sessionDownloadFolder + "/" + fileName);
        //TODO: Fix me!
        //QDir downDir(sessionDownloadFolder);
        //downDir.rmdir(sessionDownloadFolder);
    }
}

void DataBaseUpdater::forceStopDownload()
{
    downloadStarted = downloadAboutToStart = false;

    ui->pushButtonDownload->setText(tr("Download && Install"));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
    disconnect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(doStopDownload()));
    disconnect(ui->pushButtonDownload, SIGNAL(clicked()), downloaderObject->loop, SLOT(quit()));

    QDir tmpDir(randomFolder);
    tmpDir.rmdir(randomFolder);
}

QString DataBaseUpdater::getTempDir(const QString &path, bool makeDir)
{
    QString currentPath = path.isEmpty() ? QDir::tempPath() : path;
    QFileInfo currentPathInfo(currentPath);
    if (!currentPathInfo.isDir()) {
        currentPath = QDir::tempPath();
    }
    QString tmpPath = currentPath + "/~tmp_saaghar_0"; //+QString::number(qrand());
    QDir tmpDir(tmpPath);
    while (tmpDir.exists()) {
        tmpPath += QString::number(qrand());
        tmpDir.setPath(tmpPath);
    }
    if (makeDir) {
        tmpDir.mkpath(tmpPath);
    }

    return tmpPath;
}

void DataBaseUpdater::installItemToDB(const QString &fullFilePath, const QString &fileType)
{
    if (fullFilePath.isEmpty()) {
        return;
    }
    QFileInfo file(fullFilePath);
    installItemToDB(file.fileName(), file.canonicalPath(), fileType);
}

void DataBaseUpdater::installItemToDB(const QString &fileName, const QString &path, const QString &fileType)
{
    QString type = fileType;
    if (type == ".gdb" || type == "gdb" || type == ".s3db") {
        type = "s3db";
    }

    if (type == ".zip") {
        type = "zip";
    }

    if (type.isEmpty()) {
        if (fileName.endsWith(".gdb") || fileName.endsWith(".s3db")) {
            type = "s3db";
        }
        else {
            type = "zip";
        }
    }

    QString file = path + "/" + fileName;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (type == "zip") {
        if (!QFile::exists(file)) {
            qDebug() << "File does not exist.";
            QApplication::restoreOverrideCursor();
            installCompleted = false;
            return;
        }

        UnZip::ErrorCode ec;
        UnZip uz;

        ec = uz.openArchive(file);
        if (ec != UnZip::Ok) {
            qDebug() << "Unable to open archive: " << uz.formatError(ec);
            uz.closeArchive();
            installCompleted = false;
            //try open it as SQLite database!
            installItemToDB(fileName, path, "s3db");
            QApplication::restoreOverrideCursor();
            return;
        }
        else {
            QList<UnZip::ZipEntry> list = uz.entryList();
            if (list.isEmpty()) {
                installCompleted = false;
                QApplication::restoreOverrideCursor();
                return;
            }

            QString tmpExtractPath = getTempDir(path, true);
            for (int i = 0; i < list.size(); ++i) {
                QString extractPath = tmpExtractPath;
                const UnZip::ZipEntry &entry = list.at(i);
                QString entryFileName = entry.filename;

                if (entry.type == UnZip::Directory ||
                        (!entryFileName.endsWith(".png") &&
                         !entryFileName.endsWith(".gdb") &&
                         !entryFileName.endsWith(".s3db"))) {
                    continue;
                }

                if (entryFileName.endsWith(".png")) {
                    extractPath = SaagharWidget::poetsImagesDir;
                    QDir photoDir(extractPath);
                    qDebug() << "make photo dir =>" << photoDir.mkpath(extractPath);
                    qDebug() << "The author's photo =>" << extractPath + "/" + entryFileName;
                }

                ec = uz.extractFile(entryFileName, extractPath, UnZip::SkipPaths);
                qDebug() << "extract-ErrorCode=" << uz.formatError(ec) << entryFileName << extractPath;

                if (entryFileName.endsWith(".gdb") || entryFileName.endsWith(".s3db")) {
                    s_saagharWindow->importDataBase(extractPath + "/" + entryFileName, &installCompleted);
                    QFile::remove(extractPath + "/" + entryFileName);
                    QDir extractDir(extractPath);
                    extractDir.rmdir(extractPath);
                }
            }
            //TODO: Fix me!
            //QDir extractDir(extractPath);
            //qDebug() <<"remove Extract Dir--"<<extractDir.rmdir(extractPath);

            uz.closeArchive();
            QApplication::restoreOverrideCursor();
        }
    } // end "zip" type
    else if (type == "s3db") {
        qDebug() << "FILE TYPE IS S3DB! [SQLite 3 Data Base]";
        s_saagharWindow->importDataBase(file, &installCompleted);
    }

    QApplication::restoreOverrideCursor();
}

void DataBaseUpdater::resizeColumnsToContents()
{
    if (!newRootItem || !oldRootItem) {
        return;
    }
    bool newItemState = newRootItem->isExpanded();
    bool oldItemState = oldRootItem->isExpanded();

    newRootItem->setExpanded(true);
    oldRootItem->setExpanded(true);

    ui->repoSelectTree->resizeColumnToContents(0);
    ui->repoSelectTree->resizeColumnToContents(1);
    ui->repoSelectTree->resizeColumnToContents(2);
    ui->repoSelectTree->resizeColumnToContents(3);

    newRootItem->setExpanded(newItemState);
    oldRootItem->setExpanded(oldItemState);
}

void DataBaseUpdater::addRemoveRepository()
{
    QDialog addRemove(this);
    addRemove.setWindowTitle(tr("Add/Remove Repository Address"));
    QVBoxLayout vLayout(&addRemove);
    QLabel label(tr("Insert each address in a separate line!"));
    QTextEdit textEdit;
    textEdit.setPlainText(repositoriesUrls.join("\n"));
    QDialogButtonBox buttonBox;
    addRemove.setObjectName(QString::fromUtf8("addRemove"));
    addRemove.resize(400, 300);

    vLayout.setObjectName(QString::fromUtf8("vLayout"));
    label.setObjectName(QString::fromUtf8("label"));
    textEdit.setObjectName(QString::fromUtf8("textEdit"));

    vLayout.addWidget(&label);
    vLayout.addWidget(&textEdit);

    buttonBox.setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox.setOrientation(Qt::Horizontal);
    buttonBox.setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    vLayout.addWidget(&buttonBox);

    addRemove.setLayout(&vLayout);

    QObject::connect(&buttonBox, SIGNAL(accepted()), &addRemove, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &addRemove, SLOT(reject()));

    if (addRemove.exec() == QDialog::Accepted) {
        setRepositories(textEdit.toPlainText().split("\n", QString::SkipEmptyParts));
        fillRepositoryList();
    }
}

void DataBaseUpdater::reject()
{
    if (!doStopDownload()) {
        return;
    }

    QDialog::reject();
}
