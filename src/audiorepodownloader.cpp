/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2016 by S. Razi Alavizadeh                               *
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

#include "audiorepodownloader.h"
#include "unzip.h"
#include "settings.h"
#include "downloader.h"
#include "databasebrowser.h"
#include "saagharwidget.h"
#include "saagharwindow.h"
#include "saagharapplication.h"

#include "settingsmanager.h"

QString AudioRepoDownloader::downloadLocation;
bool AudioRepoDownloader::keepDownloadedFiles = false;
QStringList AudioRepoDownloader::repositoriesUrls = QStringList();
const QStringList AudioRepoDownloader::defaultRepositories = QStringList()
        << "http://a.ganjoor.net/index.xml";



AudioRepoDownloader::AudioRepoDownloader(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), ui(new Ui::AudioRepoDownloader)
{
    downloadStarted = downloadAboutToStart = false;

    ui->setupUi(this);
    ui->refreshPushButton->setIcon(QIcon(ICON_PATH + "/refresh.png"));
    ui->comboBoxRepoList->hide();
    ui->groupBoxKeepDownload->hide();

    downloaderObject = new Downloader(this, ui->downloadProgressBar, ui->labelDownloadStatus);
    setupUi();

    connect(ui->refreshPushButton, SIGNAL(clicked()), this, SLOT(readRepository()));
    connect(ui->comboBoxRepoList, SIGNAL(currentIndexChanged(QString)), this, SLOT(readRepository(QString)));
    connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
    connect(ui->pushButtonBrowse, SIGNAL(clicked()), this, SLOT(getDownloadLocation()));
    connect(downloaderObject, SIGNAL(downloadStopped()), this, SLOT(forceStopDownload()));

    if (downloadLocation.isEmpty()) {
        downloadLocation = VARS("AudioRepoDownloader/DownloadAlbumPath");
    }

    ui->lineEditDownloadLocation->setText(downloadLocation);

    setDisabledAll(downloadLocation.isEmpty());
}

bool AudioRepoDownloader::read(QIODevice* device)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn)) {
        return false;
    }
    return parseDocument();
}

bool AudioRepoDownloader::read(const QByteArray &data)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(data, true, &errorStr, &errorLine, &errorColumn)) {
        return false;
    }
    return parseDocument();
}

bool AudioRepoDownloader::parseDocument()
{
    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "DesktopGanjoorPoemAudioList") {
        return false;
    }

    insertedToList.clear();

    QDomElement child = root.firstChildElement("PoemAudio");
    while (!child.isNull()) {
        parseElement(child);
        child = child.nextSiblingElement("PoemAudio");
    }

    resizeColumnsToContents();

    return true;
}

void AudioRepoDownloader::parseElement(const QDomElement &element)
{
    QString audio_post_ID = element.firstChildElement("audio_post_ID").text();
    QString audio_order = element.firstChildElement("audio_order").text();
    QString audio_xml = element.firstChildElement("audio_xml").text();
    QString audio_mp3 = element.firstChildElement("audio_mp3").text();
    QString audio_title = element.firstChildElement("audio_title").text();
    QString audio_artist = element.firstChildElement("audio_artist").text();
    QString audio_artist_url = element.firstChildElement("audio_artist_url").text();
    QString audio_src = element.firstChildElement("audio_src").text();
    QString audio_src_url = element.firstChildElement("audio_src_url").text();
    QString audio_guid = element.firstChildElement("audio_guid").text();
    QString audio_fchecksum = element.firstChildElement("audio_fchecksum").text();

    int audio_mp3bsize = element.firstChildElement("audio_mp3bsize").text().toInt();
    audio_mp3bsize = audio_mp3bsize / 1024;
    QString fileSizeKB = QString::number(audio_mp3bsize) == "0" ? "" : QString::number(audio_mp3bsize) + " " + tr("KB");

    bool isNew = false;

    const QString key = audio_post_ID + ":" + audio_order;
    if (insertedToList.contains(key) && insertedToList.value(key) == audio_mp3) {
        return;
    }
    else {
        insertedToList.insert(key, audio_mp3);
    }

    QStringList visibleInfo = QStringList()
            << audio_title
            << fileSizeKB
            << audio_post_ID
            << audio_order
            << audio_artist
            << audio_artist_url;

    QTreeWidgetItem* item = new QTreeWidgetItem(isNew ? newRootItem : oldRootItem , visibleInfo);
    item->setToolTip(0, audio_title);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setCheckState(0, Qt::Unchecked);
    item->setData(0, AudioPostIDRole, audio_post_ID);
    item->setData(0, AudioOrderRole, audio_order);
    item->setData(0, AudioChecksumRole, audio_fchecksum);
    item->setData(0, AudioSrcRole, audio_src);
    item->setData(0, AudioArtistRole, audio_artist);
    item->setData(0, AudioSrcUrlRole, audio_src_url);
    item->setData(0, AudioArtistUrlRole, audio_artist_url);
    item->setData(0, AudioXmlRole, audio_xml);
    item->setData(0, AudioMP3Role, audio_mp3);
    item->setData(0, AudioGuidRole, audio_guid);

    if (!audio_artist_url.isEmpty()) {
        QLabel* link = new QLabel(ui->repoSelectTree);
        link->setTextFormat(Qt::RichText);
        link->setText(QString("<a href=\"%1\" title=\"%1\" >%2</a>").arg(audio_artist_url).arg(audio_artist_url));
        //blogLink->setToolTip(BlogUrl);
        link->setOpenExternalLinks(true);
        ui->repoSelectTree->setItemWidget(item, 5, link);
    }
}

void AudioRepoDownloader::fillRepositoryList()
{
    ui->comboBoxRepoList->clear();
    ui->comboBoxRepoList->addItems(QStringList()
                                   << tr("Select From Repositories List...")
                                   << repositoriesUrls
                                   << tr("Click To Add/Remove..."));
    ui->comboBoxRepoList->insertSeparator(1);
    ui->comboBoxRepoList->insertSeparator(ui->comboBoxRepoList->count() - 1);
}

void AudioRepoDownloader::importDataBase(const QString &fileName, bool* ok)
{
    QSqlDatabase dataBaseObject = DatabaseBrowser::database();
    QFileInfo dataBaseFile(dataBaseObject.databaseName());
    if (!dataBaseFile.isWritable()) {
        QMessageBox::warning(sApp->activeWindow(), tr("Error!"), tr("You have not write permission to database file, the import procedure can not proceed.\nDataBase Path: %2").arg(dataBaseFile.fileName()));
        if (ok) {
            *ok = false;
        }
        return;
    }
    QList<GanjoorPoet*> poetsConflictList = sApp->databaseBrowser()->getConflictingPoets(fileName);

    dataBaseObject.transaction();

    if (!poetsConflictList.isEmpty()) {
        QMessageBox warnAboutConflict(sApp->activeWindow());
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
            sApp->databaseBrowser()->removePoetFromDataBase(poet->_ID);
        }
    }

    //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (sApp->databaseBrowser()->importDataBase(fileName)) {
        dataBaseObject.commit();
        if (ok) {
            *ok = true;
        }
    }
    else {
        if (ok) {
            *ok = false;
        }
        dataBaseObject.rollback();
        QMessageBox warning(QMessageBox::Warning, tr("Error!"), tr("There are some errors, the import procedure was not completed"), QMessageBox::Ok
                            , DatabaseBrowser::dbUpdater, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
        warning.exec();
    }
    //QApplication::restoreOverrideCursor();
}

void AudioRepoDownloader::setDisabledAll(bool disable)
{
    ui->refreshPushButton->setDisabled(disable);
    ui->pushButtonDownload->setDisabled(disable);
    ui->repoSelectTree->setDisabled(disable);
    ui->labelSubtitle->setDisabled(disable);
}

void AudioRepoDownloader::setRepositories(const QStringList &urls)
{
    repositoriesUrls.clear();
    repositoriesUrls << urls << defaultRepositories;
    repositoriesUrls.removeDuplicates();
}

QStringList AudioRepoDownloader::repositories()
{
    return repositoriesUrls;
}

void AudioRepoDownloader::setupUi()
{
#if QT_VERSION >= 0x040700
    ui->lineEditDownloadLocation->setPlaceholderText(tr("Please select download location..."));
#else
    ui->lineEditDownloadLocation->setToolTip(tr("Please select download location..."));
#endif

    fillRepositoryList();

    ui->lineEditDownloadLocation->setText(AudioRepoDownloader::downloadLocation);
    keepDownloadedFiles = keepDownloadedFiles && !downloadLocation.isEmpty();
    ui->groupBoxKeepDownload->setChecked(keepDownloadedFiles);
    ui->downloadProgressBar->hide();
    ui->labelDownloadStatus->hide();

    ui->repoSelectTree->setLayoutDirection(Qt::RightToLeft);
    ui->repoSelectTree->setTextElideMode(Qt::ElideMiddle);
}

void AudioRepoDownloader::setupTreeRootItems()
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

void AudioRepoDownloader::readRepository(const QString &url)
{
    QString currentText = ui->comboBoxRepoList->currentText();

    if (url.isEmpty()) {
        currentText = defaultRepositories.at(0);
    }
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

    QUrl repoUrl = QUrl::fromUserInput(currentText);
    if (!repoUrl.isValid()) {
        qDebug() << "if-repoUrl==" << repoUrl.toString() << "url=" << url;
        ui->refreshPushButton->setEnabled(false);
        return;
    }

    qDebug() << "else-repoUrl==" << repoUrl.toString() << "url=" << url;
    ui->refreshPushButton->setEnabled(true);

    if (!url.isEmpty()) {
        QPair<QTreeWidgetItem*, QTreeWidgetItem*> cachedItems = itemsCache.value(currentText, QPair<QTreeWidgetItem*, QTreeWidgetItem*>());
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
        itemsCache.insert(currentText, QPair<QTreeWidgetItem*, QTreeWidgetItem*>());
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
    itemsCache.insert(currentText, QPair<QTreeWidgetItem*, QTreeWidgetItem*>(newRootItem, oldRootItem));
    connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
}

void AudioRepoDownloader::itemDataChanged(QTreeWidgetItem* /*item*/, int /*column*/)
{
    bool itemsChecked = (newRootItem->checkState(0) != Qt::Unchecked) || (oldRootItem->checkState(0) != Qt::Unchecked);
    ui->pushButtonDownload->setEnabled(itemsChecked);
}

void AudioRepoDownloader::getDownloadLocation()
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Select Album to Download Audios"),
                downloadLocation.isEmpty()
                ? QDir::homePath()
                : QFileInfo(downloadLocation).absolutePath(), QLatin1String("Saaghar Album (*.sal)")
                , NULL, QFileDialog::DontConfirmOverwrite);

    if (!fileName.isEmpty()) {
        downloadLocation = fileName;
        ui->lineEditDownloadLocation->setText(downloadLocation);

        VAR_DECL("AudioRepoDownloader/DownloadAlbumPath", downloadLocation);

        setDisabledAll(false);
    }
}

void AudioRepoDownloader::initDownload()
{
    downloadAboutToStart = true;
    ui->pushButtonDownload->setText(tr("Stop"));
    disconnect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(doStopDownload()));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), downloaderObject->loop, SLOT(quit()));

    sessionDownloadFolder = AudioRepoDownloader::downloadLocation;
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

bool AudioRepoDownloader::doStopDownload()
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

void AudioRepoDownloader::closeEvent(QCloseEvent* e)
{
    AudioRepoDownloader::downloadLocation = ui->lineEditDownloadLocation->text();
    keepDownloadedFiles = !downloadLocation.isEmpty() && ui->groupBoxKeepDownload->isChecked();
    if (!doStopDownload()) {
        e->ignore();    //download in progress
    }
}

void AudioRepoDownloader::downloadItem(QTreeWidgetItem* item, bool install)
{
    if (!item) {
        return;
    }
    item->treeWidget()->setCurrentItem(item);
    bool keepDownlaodedFiles = ui->groupBoxKeepDownload->isChecked();
    downloadStarted = true;
    QString urlStr = item->data(0, AudioSrcUrlRole).toString();
    QUrl downloadUrl(urlStr);
    downloaderObject->downloadFile(downloadUrl, sessionDownloadFolder, item->text(0));

    QFileInfo fileInfo(downloadUrl.path());
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty()) {
        fileName = "index.html";
    }

    if (install) {
        ui->labelDownloadStatus->setText(tr("Installing..."));
        QString fileType = item->data(0, AudioChecksumRole).toString();
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

void AudioRepoDownloader::forceStopDownload()
{
    downloadStarted = downloadAboutToStart = false;

    ui->pushButtonDownload->setText(tr("Download && Install"));
    connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
    disconnect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(doStopDownload()));
    disconnect(ui->pushButtonDownload, SIGNAL(clicked()), downloaderObject->loop, SLOT(quit()));

    QDir tmpDir(randomFolder);
    tmpDir.rmdir(randomFolder);
}

QString AudioRepoDownloader::getTempDir(const QString &path, bool makeDir)
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

void AudioRepoDownloader::installItemToDB(const QString &fullFilePath, const QString &fileType)
{
    if (fullFilePath.isEmpty()) {
        return;
    }
    QFileInfo file(fullFilePath);
    installItemToDB(file.fileName(), file.canonicalPath(), fileType);
}

void AudioRepoDownloader::installItemToDB(const QString &fileName, const QString &path, const QString &fileType)
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
                    importDataBase(extractPath + "/" + entryFileName, &installCompleted);
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
        importDataBase(file, &installCompleted);
    }

    QApplication::restoreOverrideCursor();
}

void AudioRepoDownloader::resizeColumnsToContents()
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

void AudioRepoDownloader::addRemoveRepository()
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

void AudioRepoDownloader::reject()
{
    if (!doStopDownload()) {
        return;
    }

    QDialog::reject();
}
