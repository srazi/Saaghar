/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2012 by S. Razi Alavizadeh                          *
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

#include "unzip.h"
#include "downloader.h"
#include "DataBaseUpdater.h"
#include "SaagharWidget.h"

QString DataBaseUpdater::downloadLocation = QString();
QStringList DataBaseUpdater::repositoriesUrls = QStringList();

int PoetID_DATA = Qt::UserRole+1;
int CatID_DATA = Qt::UserRole+2;
int FileExt_DATA = Qt::UserRole+3;
int ImageUrl_DATA = Qt::UserRole+4;
int LowestPoemID_DATA = Qt::UserRole+5;
int DownloadUrl_DATA = Qt::UserRole+6;
int BlogUrl_DATA = Qt::UserRole+7;

DataBaseUpdater::DataBaseUpdater(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent,f), ui(new Ui::DataBaseUpdater)
{
	downloadStarted = downloadAboutToStart = false;

	ui->setupUi(this);
	downloaderObject = new Downloader(this, ui->downloadProgressBar, ui->labelDownloadStatus);
	setupUi();
	//readRepository("");

	connect(ui->refreshPushButton, SIGNAL(clicked()), this, SLOT(readRepository()));
	connect(ui->comboBoxRepoList, SIGNAL(currentIndexChanged(QString)), this, SLOT(readRepository(QString)));
	//connect(ui->repoSelectTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(doubleClicked(QTreeWidgetItem*,int)));
	connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
	connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
	connect(ui->pushButtonBrowse, SIGNAL(clicked()), this, SLOT(getDownloadLocation()));
	connect(downloaderObject,SIGNAL(downloadStopped()), this, SLOT(forceStopDownload()));
}

bool DataBaseUpdater::read(QIODevice *device)
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn))
	{
		return false;
	}
	return parseDocument();
}

bool DataBaseUpdater::read(const QByteArray &data)
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!domDocument.setContent(data, true, &errorStr, &errorLine, &errorColumn))
	{
		return false;
	}
	return parseDocument();
}

bool DataBaseUpdater::parseDocument()
{
	QDomElement root = domDocument.documentElement();
	if (root.tagName() != "DesktopGanjoorGDBList")
	{
		return false;
	}

	insertedToList.clear();

	QDomElement child = root.firstChildElement("gdb");
	while (!child.isNull())
	{
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
	fileSize = fileSize/1024;
	QString fileSizeKB = QString::number(fileSize) == "0" ? "" : QString::number(fileSize)+" "+tr("KB");
	//QStringList pubDateYMD = element.firstChildElement("PubDate").text().split("-",QString::SkipEmptyParts);
	QLocale persainLocale(QLocale::Persian, QLocale::Iran);
	QDate pubDate = persainLocale.toDate(element.firstChildElement("PubDate").text(), "yyyy-MM-dd");
//	if (PubDate.size() == 3)
//	{
//		QDate pubDate(pubDateYMD.at(0).toInt(), pubDateYMD.at(1).toInt(), pubDateYMD.at(2).toInt());
//		QLocale
//	}

	QStringList visibleInfo = QStringList() << CatName 
											<< fileSizeKB
											<< pubDate.toString("yyyy-MM-dd");
											//<< tr("Go to Release Information");//BlogUrl;//<< DownloadUrl 
//meta data
	QString PoetID = element.firstChildElement("PoetID").text();
	int CatID = element.firstChildElement("CatID").text().toInt();
	QString FileExt = element.firstChildElement("FileExt").text();
	QString ImageUrl = element.firstChildElement("ImageUrl").text();
	QString LowestPoemID = element.firstChildElement("LowestPoemID").text();

	bool isNew = SaagharWidget::ganjoorDataBase->getCategory(CatID).isNull();

	if (insertedToList.contains(CatID) && insertedToList.value(CatID) == DownloadUrl)
		return;
	else
		insertedToList.insert(CatID, DownloadUrl);

	QTreeWidgetItem *item = new QTreeWidgetItem(isNew ? newRootItem : oldRootItem , visibleInfo);
	item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
	item->setCheckState(0, Qt::Unchecked);
	item->setData(0, PoetID_DATA, PoetID);
	item->setData(0, CatID_DATA, CatID);
	item->setData(0, FileExt_DATA, FileExt);
	item->setData(0, ImageUrl_DATA, ImageUrl);
	item->setData(0, LowestPoemID_DATA, LowestPoemID);
	item->setData(0, DownloadUrl_DATA, DownloadUrl);
	item->setData(0, BlogUrl_DATA, BlogUrl);

	if (!BlogUrl.isEmpty())
	{
		QLabel *blogLink = new QLabel(ui->repoSelectTree);
		blogLink->setTextFormat(Qt::RichText);
		blogLink->setText(QString("<a href=\"%1\" title=\"%1\" >%2</a>").arg(BlogUrl).arg(tr("Go to Release Information")));
		//blogLink->setToolTip(BlogUrl);
		blogLink->setOpenExternalLinks(true);
		ui->repoSelectTree->setItemWidget(item, 3, blogLink);
	}
}

void DataBaseUpdater::setRepositories(const QStringList &urls)
{
	repositoriesUrls = urls;
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

	ui->comboBoxRepoList->addItems(QStringList() << repositoriesUrls << tr("Click To Add/Remove...") );
	ui->refreshPushButton->setEnabled(false);

	ui->lineEditDownloadLocation->setText(DataBaseUpdater::downloadLocation);
	ui->downloadProgressBar->hide();
	ui->labelDownloadStatus->hide();
	//ui->hButtonsLayout->addWidget(downloaderObject->downloadProgressBar(this));
	//downloaderObject->downloadProgressBar(this)->hide();
//	ui->downloadProgressBar->hide();
//	repoSelectTree = new QTreeWidget(parent);
//	repoSelectTree->setObjectName("repoSelectTree");
//	repoSelectTree->setColumnCount(5);
	ui->repoSelectTree->setLayoutDirection(Qt::RightToLeft);
	//setupTreeRootItems();
}

void DataBaseUpdater::setupTreeRootItems()
{
	QFont font(ui->repoSelectTree->font());
	font.setBold(true);

	oldRootItem = new QTreeWidgetItem(ui->repoSelectTree);
	oldRootItem->setText(0, tr("Installed"));
	oldRootItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsTristate);
	oldRootItem->setCheckState(0, Qt::Unchecked);
	oldRootItem->setFont(0, font);
	oldRootItem->setExpanded(false);
	oldRootItem->setDisabled(true);

	newRootItem = new QTreeWidgetItem(ui->repoSelectTree);
	newRootItem->setText(0, tr("Ready To Install"));
	newRootItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsTristate);
	newRootItem->setCheckState(0, Qt::Unchecked);
	newRootItem->setFont(0, font);
	newRootItem->setExpanded(true);
	newRootItem->setDisabled(true);
}

void DataBaseUpdater::readRepository(const QString &url)
{
//	disconnect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
//	//clear tree
//	delete newRootItem;
//	delete oldRootItem;
//	setupTreeRootItems();
//	connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
	disconnect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
	ui->pushButtonDownload->setEnabled(false);

	if (ui->comboBoxRepoList->currentIndex() == ui->comboBoxRepoList->count()-1)
	{
		disconnect(ui->comboBoxRepoList, SIGNAL(currentIndexChanged(QString)), this, SLOT(readRepository(QString)));
		addRemoveRepository();
		connect(ui->comboBoxRepoList, SIGNAL(currentIndexChanged(QString)), this, SLOT(readRepository(QString)));
		ui->comboBoxRepoList->setCurrentIndex(-1);//maybe a bug in Qt, set to '0' does not work!
		ui->comboBoxRepoList->setCurrentIndex(0);
		return;
	}

	//clear tree
	//delete newRootItem;
	//delete oldRootItem;
//	QTreeWidgetItem *itm0 = ui->repoSelectTree->takeTopLevelItem(0);
//	QTreeWidgetItem *itm1 = ui->repoSelectTree->takeTopLevelItem(1);
//	qDebug() << "REMOOOOOOOOOVE->0"<<itm0<<itm1;
//	delete itm0;
//	delete itm1;
	ui->repoSelectTree->takeTopLevelItem(1);
	ui->repoSelectTree->takeTopLevelItem(0);

	//itm1 = ui->repoSelectTree->takeTopLevelItem(1);
	//delete oldRootItem;

	QUrl repoUrl = QUrl::fromUserInput(ui->comboBoxRepoList->currentText());
	if ( !repoUrl.isValid() )
	{
		qDebug()<<"if-repoUrl=="<<repoUrl.toString()<<"url="<<url;
		ui->refreshPushButton->setEnabled(false);
		return;
	}

	qDebug()<<"else-repoUrl=="<<repoUrl.toString()<<"url="<<url;
	ui->refreshPushButton->setEnabled(true);

	if (!url.isEmpty())
	{
		//ui->pushButtonDownload->setEnabled(false);
//		//QFile file("D:/Z[Work]/Saaghar/OTHERS/repositories-XML/sitegdbs.xml");
//	//	file.open(QFile::ReadOnly);
//	//	QByteArray fileContent = file.readAll();
////		QBuffer buffer(&fileContent);
//	//ui->comboBoxRepoList->setItemData(ui->comboBoxRepoList->currentIndex(), fileContent, Qt::UserRole);

//		QByteArray currentData = ui->comboBoxRepoList->itemData(0, Qt::UserRole+1+ui->comboBoxRepoList->currentIndex()).toByteArray();
//		qDebug()<<"url.isEmpty()=="<<url<<"----------------------------\ncurrentData="<<currentData;
//		if (!currentData.isEmpty())
//		{
//			//QBuffer buffer(&currentData);
//			//buffer.open(QIODevice::ReadOnly);
//		//	qDebug()<<"#####################################\nbuffer.readAll==\n"<<buffer.readAll()<<"#####################################\n";
//			//if (fileContent == buffer.readAll())
//		//		qDebug()<< "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
//			read(currentData);
//			return;
//		}
		//if (itemsCache.contains(ui->comboBoxRepoList->currentIndex()))
		{
			QPair<QTreeWidgetItem *, QTreeWidgetItem *> cachedItems = itemsCache.value(ui->comboBoxRepoList->currentText(), QPair<QTreeWidgetItem *, QTreeWidgetItem *>());
			if (cachedItems.first && cachedItems.second)
			{
				ui->repoSelectTree->takeTopLevelItem(1);
				ui->repoSelectTree->takeTopLevelItem(0);
				newRootItem = cachedItems.first;
				oldRootItem = cachedItems.second;
				ui->repoSelectTree->addTopLevelItem(oldRootItem);
				ui->repoSelectTree->addTopLevelItem(newRootItem);
				resizeColumnsToContents();
				newRootItem->setExpanded(true);
				itemDataChanged(0,0);
				connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
				return;
			}
		}
	}
	else
	{
		itemsCache.insert(ui->comboBoxRepoList->currentText(), QPair<QTreeWidgetItem *, QTreeWidgetItem *>());
		//ui->comboBoxRepoList->setItemData(0, "", Qt::UserRole+1+ui->comboBoxRepoList->currentIndex());
	}

	//if (repositoriesUrls.isEmpty()) return;
	//repositoriesUrls.clear();

	//for (int i=0;i<repositoriesUrls.size();++i)
	{
		//QString urlStr = repositoriesUrls.at(i);
		bool isRemote = false;
		//url.remove("http://");
		//qDebug()<<"repoUrl=="<<repoUrl.toString();
		QString urlStr = repoUrl.toString();
		qDebug()<<"url===="<<urlStr;
		setupTreeRootItems();
		if (!urlStr.contains("file:///"))
		{
			isRemote = true;
			QString tmpPath = getTempDir();
			QDir downDir(tmpPath);
			if (!downDir.exists() && !downDir.mkpath(tmpPath))
			{
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
			ui->refreshPushButton->setEnabled(true);
			ui->labelDownloadStatus->hide();
			QString filepath = tmpPath+"/"+urlInfo.fileName();
			QFile file(filepath);
//			qDebug()<<"urlInfo.fileName="<<urlInfo.fileName()<<"full="<<tmpPath+"/"+urlInfo.fileName();
//			qDebug()<<"from file="<<file.fileName();
//			file.open(QFile::ReadOnly);
//			QByteArray fileContent = file.readAll();
//			QBuffer buffer(&fileContent);
			
			if (read(&file))//read(&file))
			{
				int a;a;
				//ui->comboBoxRepoList->setItemData(0, fileContent, Qt::UserRole+1+ui->comboBoxRepoList->currentIndex());
			}
			file.remove();
			downDir.rmdir(tmpPath);
		}
		else
		{
			urlStr.remove("file:///");
			QFile file(urlStr);
			read(&file);
		}

		newRootItem->setDisabled(newRootItem->childCount() == 0);
		oldRootItem->setDisabled(oldRootItem->childCount() == 0);
		if (newRootItem->childCount() == 0 && oldRootItem->childCount() == 0)
		{
			ui->repoSelectTree->clear();
			newRootItem = 0;
			oldRootItem = 0;
		}
		itemsCache.insert(ui->comboBoxRepoList->currentText(), QPair<QTreeWidgetItem *, QTreeWidgetItem *>(newRootItem, oldRootItem));
		connect(ui->repoSelectTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemDataChanged(QTreeWidgetItem*,int)));
	}
}

void DataBaseUpdater::itemDataChanged(QTreeWidgetItem */*item*/, int /*column*/)
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
	ui->repoSelectTree->setEnabled(false);
	ui->groupBoxKeepDownload->setEnabled(false);

	sessionDownloadFolder = DataBaseUpdater::downloadLocation;
	QDir downDir;
	if (ui->groupBoxKeepDownload->isChecked())
	{
		if (sessionDownloadFolder.isEmpty())
		{
			QMessageBox::information(this, tr("Warning!"), tr("Please select download location."));
			doStopDownload();
			return;
		}
	}
	else
	{
//		sessionDownloadFolder = QDir::tempPath()+"/~tmp_saaghar_"+QString::number(qrand());
//		downDir.setPath(sessionDownloadFolder);
//		while (downDir.exists())
//		{
//			sessionDownloadFolder+=QString::number(qrand());
//			downDir.setPath(sessionDownloadFolder);
//		}
		randomFolder = sessionDownloadFolder = getTempDir();
		qDebug() << "randomFolder="<<randomFolder;
	}

	downDir.setPath(sessionDownloadFolder);
	if (!downDir.exists() && !downDir.mkpath(sessionDownloadFolder))
	{
		QMessageBox::information(this, tr("Error!"), tr("Can not create download path."));
		qDebug() << "sessionDownloadFolder="<<sessionDownloadFolder;
		doStopDownload();
		return;
	}

	for (int i=0; i<newRootItem->childCount(); ++i)
	{
		QTreeWidgetItem *child = newRootItem->child(i);
		if (!child || child->checkState(0) != Qt::Checked)
			continue;
		installCompleted = false;
		downloadItem(child, true);

		if (installCompleted)
		{
			//after install we change the parent
			newRootItem->removeChild(child);
			oldRootItem->addChild(child);
			child->setCheckState(0, Qt::Unchecked);
			child->setFlags(Qt::NoItemFlags);
		}
	}

	for (int i=0; i<oldRootItem->childCount(); ++i)
	{
		QTreeWidgetItem *child = oldRootItem->child(i);
		if (!child || child->checkState(0) != Qt::Checked)
			continue;
		installCompleted = false;
		downloadItem(child, true);
		
		if (installCompleted)
		{
			//after install we change the parent
			child->setCheckState(0, Qt::Unchecked);
			child->setFlags(Qt::NoItemFlags);
		}
	}

	if (!ui->groupBoxKeepDownload->isChecked())
	{
		//qDebug() <<"remove down File--"<< QFile::remove(sessionDownloadFolder+"/"+fileName);
		QDir downDir(sessionDownloadFolder);
		qDebug() <<"remove down Dir--"<< downDir.rmdir(sessionDownloadFolder);
	}

	ui->labelDownloadStatus->setText(tr("Finished!"));

	//download ended
	downloadAboutToStart = downloadStarted = false;
	doStopDownload();
}

bool DataBaseUpdater::doStopDownload()
{
	if (!downloadAboutToStart)
		return true;

	if (downloadStarted)
	{
		if (QMessageBox::question(this, tr("Warning!"), tr("Download in progress! Are you sure to stop downloading?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
			 ==
			QMessageBox::No)
			return false;
	}
	downloaderObject->cancelDownload();
	forceStopDownload();
	qDebug() <<"doStopDownload!!!!!!";
	return true;
}

void DataBaseUpdater::closeEvent(QCloseEvent *e)
{
	DataBaseUpdater::downloadLocation = ui->lineEditDownloadLocation->text();
	if (!doStopDownload())
		e->ignore();//download in progress
}

void DataBaseUpdater::downloadItem(QTreeWidgetItem *item, bool install)
{
	if (!item) return;
	bool keepDownlaodedFiles = ui->groupBoxKeepDownload->isChecked();
	downloadStarted = true;
	qDebug() << "downloadItem-sessionDownloadFolder="<<sessionDownloadFolder;
	QString urlStr = item->data(0, DownloadUrl_DATA).toString();
	qDebug() << "urlStr="<<urlStr;
	QUrl downloadUrl(urlStr);
	downloaderObject->downloadFile(downloadUrl, sessionDownloadFolder, item->text(0));

	QFileInfo fileInfo(downloadUrl.path());
	QString fileName = fileInfo.fileName();
	if (fileName.isEmpty())
		fileName = "index.html";

	if (install)
	{
		ui->labelDownloadStatus->setText(tr("Installing..."));
		QString fileType = item->data(0, FileExt_DATA).toString();
		installItemToDB(fileName, sessionDownloadFolder, fileType);
		ui->labelDownloadStatus->setText(tr("Installed."));
	}

	if (!keepDownlaodedFiles)
	{
		qDebug() <<"remove down File--"<< QFile::remove(sessionDownloadFolder+"/"+fileName);
		//QDir downDir(sessionDownloadFolder);
		//qDebug() <<"remove down Dir--"<< downDir.rmdir(sessionDownloadFolder);
	}
}

void DataBaseUpdater::forceStopDownload()
{
	downloadStarted = downloadAboutToStart = false;

	ui->pushButtonDownload->setText(tr("Download && Install"));
	connect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(initDownload()));
	disconnect(ui->pushButtonDownload, SIGNAL(clicked()), this, SLOT(doStopDownload()));
	disconnect(ui->pushButtonDownload, SIGNAL(clicked()), downloaderObject->loop, SLOT(quit()));
	ui->repoSelectTree->setEnabled(true);
	ui->groupBoxKeepDownload->setEnabled(true);

	QDir tmpDir(randomFolder);
	qDebug() <<"emit downloadStopped!!!!!!rmDir-randomFolder="<<randomFolder<<"bool="<< tmpDir.rmdir(randomFolder);
}

QString DataBaseUpdater::getTempDir(const QString &path, bool makeDir)
{
	QString currentPath = path.isEmpty() ? QDir::tempPath() : path;
	QFileInfo currentPathInfo(currentPath);
	if (!currentPathInfo.isDir())
		currentPath = QDir::tempPath();
	QString tmpPath = currentPath+"/~tmp_saaghar_0";//+QString::number(qrand());
	QDir tmpDir(tmpPath);
	while (tmpDir.exists())
	{
		tmpPath+=QString::number(qrand());
		tmpDir.setPath(tmpPath);
	}
	if (makeDir)
		tmpDir.mkpath(tmpPath);
	qDebug()<<"getTempDir()="<<tmpPath;
	return tmpPath;
}

void DataBaseUpdater::installItemToDB(const QString &fileName, const QString &path, const QString &fileType)
{
	QString type = fileType;
	if (type == ".gdb" || type == "gdb" || type == ".s3db")
		type = "s3db";

	if (type == ".zip")
		type = "zip";

	if (type.isEmpty())
	{
		if (fileName.endsWith(".gdb") || fileName.endsWith(".s3db"))
			type = "s3db";
		else
			type = "zip";
	}

	QString file = path+"/"+fileName;

	qDebug() << "fileName="<<fileName<<"type="<<type<<"path="<<path;

	if (type == "zip")
	{
		if (!QFile::exists(file))
		{
			qDebug() << "File does not exist.";
			return;
		}
	
		UnZip::ErrorCode ec;
		UnZip uz;
	
		ec = uz.openArchive(file);
		if (ec != UnZip::Ok)
		{
			qDebug() << "Unable to open archive: " << uz.formatError(ec);
			uz.closeArchive();
			installItemToDB(fileName, path, "s3db");//try open it as SQLite database!
			return;
		}
		else
		{
			//qDebug() <<"ziiiiiiip file list"<< uz.fileList();
			QList<UnZip::ZipEntry> list = uz.entryList();
			if (list.isEmpty())
			{
				qDebug() << "Empty archive.";
				return;
			}
			QString extractPath = getTempDir(path, true);
			for (int i = 0; i < list.size(); ++i)
			{
				const UnZip::ZipEntry& entry = list.at(i);
				QString entryFileName = entry.filename;
				qDebug() << "entryFileName="<<entryFileName;
				if (entry.type == UnZip::Directory ||
					(!entryFileName.endsWith(".png") &&
					!entryFileName.endsWith(".gdb") &&
					!entryFileName.endsWith(".s3db")))
					continue;

				if (entryFileName.endsWith(".png"))
				{
					extractPath = SaagharWidget::poetsImagesDir;
					qDebug() << "The author's photo =>" <<extractPath+"/"+entryFileName;
				}

				ec = uz.extractFile(entryFileName, extractPath, UnZip::SkipPaths);
				qDebug() << "extract-ErrorCode="<< uz.formatError(ec)<<entryFileName<<extractPath;

				if (entryFileName.endsWith(".gdb") || entryFileName.endsWith(".s3db"))
				{
					qDebug() << "emit installRequest="<<extractPath+"/"+entryFileName;
					emit installRequest(extractPath+"/"+entryFileName, &installCompleted);
					qDebug() <<"remove Extract File--"<<QFile::remove(extractPath+"/"+entryFileName);
					QDir extractDir(extractPath);
					qDebug() <<"remove Extract Dir--"<<extractDir.rmdir(extractPath);
				}

				//qDebug() <<"remove Extract File--"<<QFile::remove(extractPath+"/"+entryFileName);
			}
			//QDir extractDir(extractPath);
			//qDebug() <<"remove Extract Dir--"<<extractDir.rmdir(extractPath);

			uz.closeArchive();
		}
	} // end "zip" type
	else if (type == "s3db")
	{
		qDebug() << "FILE TYPE IS S3DB! [SQLite 3 Data Base]";
		emit installRequest(file, &installCompleted);
	}
}

void DataBaseUpdater::resizeColumnsToContents()
{
	if (!newRootItem || !oldRootItem) return;
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
	QLabel label(tr("Insert each address in its own line!"));
	QTextEdit textEdit;
	textEdit.setPlainText(repositoriesUrls.join("\n"));
	//QHBoxLayout hLayout(this);
	QDialogButtonBox buttonBox;
	//QPushButton
	addRemove.setObjectName(QString::fromUtf8("addRemove"));
	addRemove.resize(400, 300);

	vLayout.setObjectName(QString::fromUtf8("vLayout"));
	label.setObjectName(QString::fromUtf8("label"));
	textEdit.setObjectName(QString::fromUtf8("textEdit"));

	vLayout.addWidget(&label);
	vLayout.addWidget(&textEdit);

	buttonBox.setObjectName(QString::fromUtf8("buttonBox"));
	buttonBox.setOrientation(Qt::Horizontal);
	buttonBox.setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

	vLayout.addWidget(&buttonBox);

	addRemove.setLayout(&vLayout);

	QObject::connect(&buttonBox, SIGNAL(accepted()), &addRemove, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &addRemove, SLOT(reject()));

	if (addRemove.exec() == QDialog::Accepted)
	{
		repositoriesUrls.clear();
		repositoriesUrls = textEdit.toPlainText().split("\n", QString::SkipEmptyParts);
		ui->comboBoxRepoList->clear();
		ui->comboBoxRepoList->addItems(QStringList()
										<< tr("Select From Repositories List...")
										<< repositoriesUrls
										<< tr("Click To Add/Remove...") );
	}
}

void DataBaseUpdater::reject()
{
	if (!doStopDownload())
		return;

	QDialog::reject();
}
