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
 
#include "QGanjoorDbBrowser.h"
#include <QApplication>
#include <QMessageBox>
#include <QVariant>
#include <QSqlQuery>
#include <QFileInfo>
#include <QFileDialog>
#include <QProgressDialog>
#include <QTime>

QStringList QGanjoorDbBrowser::dataBasePath = QStringList();

const int minNewPoetID = 1001;
const int minNewCatID  = 10001;
const int minNewPoemID = 100001;

const int DatabaseVersion = 1;

QGanjoorDbBrowser::QGanjoorDbBrowser(QString sqliteDbCompletePath)
{
	bool flagSelectNewPath = false;
	QString newPath = "";

	QFileInfo dBFile(sqliteDbCompletePath);
	dBName = dBFile.fileName();
	dBConnection = QSqlDatabase::addDatabase("QSQLITE", dBName);
	dBConnection.setDatabaseName(sqliteDbCompletePath);
	while (!dBFile.exists() || !dBConnection.open())
	{
		QString errorString = dBConnection.lastError().text();

		dBConnection = QSqlDatabase();
		QSqlDatabase::removeDatabase(dBName);

		QMessageBox warnDataBaseOpen(0);
		warnDataBaseOpen.setWindowTitle(tr("Cannot open Database File!"));
		warnDataBaseOpen.setIcon(QMessageBox::Information);
		if ( errorString.simplified().isEmpty() /*contains("Driver not loaded")*/ )
		{
			warnDataBaseOpen.setText(tr("Cannot open database file...\nDataBase Path=%1\nSaaghar needs its database for working properly. Press 'OK' if you want to terminate application or press 'Browse' for select a new path for database.").arg(sqliteDbCompletePath));
			warnDataBaseOpen.addButton(tr("Browse..."), QMessageBox::AcceptRole);
		}
		else
			warnDataBaseOpen.setText(tr("Cannot open database file...\nThere is an error!\nError: %1\nDataBase Path=%2").arg(errorString).arg(sqliteDbCompletePath));

		warnDataBaseOpen.setStandardButtons(QMessageBox::Ok);
		warnDataBaseOpen.setEscapeButton(QMessageBox::Ok);
		warnDataBaseOpen.setDefaultButton(QMessageBox::Ok);
		int ret = warnDataBaseOpen.exec();
		if ( ret != QMessageBox::Ok && errorString.simplified().isEmpty() )
		{
			QString dir = QFileDialog::getExistingDirectory(0,tr("Add Path For Data Base"), dBFile.absoluteDir().path(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
			if ( !dir.isEmpty() ) 
			{
				dir.replace(QString("\\"),QString("/"));
				if ( !dir.endsWith('/') ) dir+="/";
				dBFile.setFile(dir+"/ganjoor.s3db");
				dBName = dBFile.fileName();
				dBConnection = QSqlDatabase::addDatabase("QSQLITE", dBName);
				dBConnection.setDatabaseName(dir+"/ganjoor.s3db");

				flagSelectNewPath = true;
				newPath = dir;
			}
			else
			{
				exit(1);
			}
		}
		else
		{
			exit(1);
		}
	}

	if (flagSelectNewPath)
	{
		QGanjoorDbBrowser::dataBasePath.clear();//in this version Saaghar just use its first search path
		QGanjoorDbBrowser::dataBasePath << newPath;
	}

	cachedMaxCatID = cachedMaxPoemID = 0;
}

QGanjoorDbBrowser::~QGanjoorDbBrowser()
{
	foreach(const QString& connectionName, QSqlDatabase::connectionNames())
	{
		QSqlDatabase::database(connectionName).close();
		QSqlDatabase::removeDatabase(connectionName);
	}
}

bool QGanjoorDbBrowser::isConnected(const QString& connectionID)
{
	QString connectionName = dBName;
	if (!connectionID.isEmpty()) connectionName = connectionID;
	dBConnection = QSqlDatabase::database(connectionName, true);//dBName
	return dBConnection.isOpen();
}

QList<GanjoorPoet *> QGanjoorDbBrowser::getPoets(const QString& connectionID, bool sort)
{
	QList<GanjoorPoet *> poets;
	if ( isConnected(connectionID) )
	{
		QSqlDatabase dataBaseObject = dBConnection;
		if (!connectionID.isEmpty())
			/*dBConnection*/ dataBaseObject = QSqlDatabase::database(connectionID);
		QSqlQuery q(dataBaseObject);
		bool descriptionExists = false;
		if ( q.exec("SELECT id, name, cat_id, description FROM poet") )
			descriptionExists = true;
		else
			q.exec("SELECT id, name, cat_id FROM poet");

		q.first();
		while( q.isValid() && q.isActive() )
		{
			GanjoorPoet *gPoet = new GanjoorPoet();
			QSqlRecord qrec = q.record();
			if (descriptionExists)
				gPoet->init(qrec.value(0).toInt(),	qrec.value(1).toString(), qrec.value(2).toInt(), qrec.value(3).toString() );
			else
				gPoet->init(qrec.value(0).toInt(),	qrec.value(1).toString(), qrec.value(2).toInt(), "" );
			poets.append(gPoet);
			if (!q.next()) break;
		}
		if (sort)
			qSort(poets.begin(), poets.end(), comparePoetsByName);
	}
	return poets;
}

bool QGanjoorDbBrowser::comparePoetsByName(GanjoorPoet *poet1, GanjoorPoet *poet2)
{
	return (QString::localeAwareCompare(poet1->_Name, poet2->_Name) < 0);
}

GanjoorCat QGanjoorDbBrowser::getCategory(int CatID)
{
	GanjoorCat gCat;
	gCat.init();
	if (isConnected())
	{
		QSqlQuery q(dBConnection);
		q.exec("SELECT poet_id, text, parent_id, url FROM cat WHERE id = " + QString::number(CatID) );
		q.first();
		if (q.isValid() && q.isActive())
		{
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

bool QGanjoorDbBrowser::compareCategoriesByName(GanjoorCat *cat1, GanjoorCat *cat2)
{
	return (QString::localeAwareCompare(cat1->_Text, cat2->_Text) < 0);
}

QList<GanjoorCat *> QGanjoorDbBrowser::getSubCategories(int CatID)
{
	QList<GanjoorCat *> lst;
	if (isConnected())
	{
	   QSqlQuery q(dBConnection);
	   q.exec("SELECT poet_id, text, url, ID FROM cat WHERE parent_id = " + QString::number(CatID));
	   q.first();
	   while( q.isValid() && q.isActive() )
	   {
		   QSqlRecord qrec = q.record();
		   GanjoorCat *gCat = new GanjoorCat();
		   gCat->init((qrec.value(3)).toInt(), (qrec.value(0)).toInt(), (qrec.value(1)).toString(), CatID, (qrec.value(2)).toString());
		   lst.append(gCat);
		   if (!q.next()) break;
	   }
	   if (CatID == 0)
			qSort(lst.begin(), lst.end(), compareCategoriesByName);
	}
	return lst;
}

QList<GanjoorCat> QGanjoorDbBrowser::getParentCategories(GanjoorCat Cat)
{
	QList<GanjoorCat> lst;
	if (isConnected())
	{
		while (!Cat.isNull() && Cat._ParentID != 0)
		{
			Cat = getCategory(Cat._ParentID);
			lst.insert(0, Cat);
		}
		GanjoorCat gCat;
		gCat.init(0, 0, QString::fromLocal8Bit("خانه"), 0, "");
		lst.insert(0, gCat);
	}
	return lst;
}

QList<GanjoorPoem *> QGanjoorDbBrowser::getPoems(int CatID)
{
	QList<GanjoorPoem *> lst;
	if (isConnected())
	{
		QString selectQuery = QString("SELECT ID, title, url FROM poem WHERE cat_id = %1 ORDER BY ID").arg(CatID);
		QSqlQuery q(dBConnection);
		q.exec(selectQuery);
		q.first();
		QSqlRecord qrec;
		while( q.isValid() && q.isActive() )
		{
			qrec = q.record();
			GanjoorPoem *gPoem = new GanjoorPoem();
			gPoem->init((qrec.value(0)).toInt(), CatID, (qrec.value(1)).toString(), (qrec.value(2)).toString(), false, "");
			lst.append(gPoem);
			if (!q.next()) 
				break;
		}
	}
	return lst;
}

QList<GanjoorVerse *> QGanjoorDbBrowser::getVerses(int PoemID)
{
	return getVerses(PoemID, 0);
}

QString QGanjoorDbBrowser::getFirstMesra(int PoemID) //just first Mesra
{
	if (isConnected())
	{
		QSqlQuery q(dBConnection);
		QString selectQuery = QString( "SELECT vorder, text FROM verse WHERE poem_id = %1 order by vorder  LIMIT 1").arg(PoemID);
		q.exec(selectQuery);
		q.first();
		while( q.isValid() && q.isActive() )
		{
			QStringList words = q.record().value(1).toString().split(QRegExp("\\b"));
			int length = qMin(words.size(), 30);
			QString text = "";
			if ( length<words.size() )
				text = "...";
			words = words.mid(0, length);
			return words.join("")+text;
		}
	}
	return "";
}

QList<GanjoorVerse *> QGanjoorDbBrowser::getVerses(int PoemID, int Count)
{
	QList<GanjoorVerse *> lst;
	if (isConnected())
	{
		QSqlQuery q(dBConnection);
		QString selectQuery = QString( "SELECT vorder, position, text FROM verse WHERE poem_id = %1 order by vorder" + (Count>0 ? " LIMIT "+QString::number(Count) : "") ).arg(PoemID);
		q.exec(selectQuery);
		q.first();
		while( q.isValid() && q.isActive() )
		{
			QSqlRecord qrec = q.record();
			GanjoorVerse *gVerse = new GanjoorVerse();
			gVerse->init(PoemID, (qrec.value(0)).toInt(), (VersePosition)((qrec.value(1)).toInt()), (qrec.value(2)).toString());
			lst.append(gVerse);
			if (!q.next()) break;
		}
	}
	return lst;
}

GanjoorPoem QGanjoorDbBrowser::getPoem(int PoemID)
{
	GanjoorPoem gPoem;
	gPoem.init();
	if (isConnected())
	{
		QSqlQuery q(dBConnection);
		q.exec("SELECT cat_id, title, url FROM poem WHERE ID = " + QString::number(PoemID) );
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			gPoem.init(PoemID, (qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toString(), false, QString(""));
			return  gPoem;
		}
	}
	return gPoem;
}

GanjoorPoem QGanjoorDbBrowser::getNextPoem(int PoemID, int CatID)
{
	GanjoorPoem gPoem;
	gPoem.init();
	if (isConnected() && PoemID!=-1) // PoemID==-1 when getNextPoem(GanjoorPoem poem) pass null poem
	{
		QSqlQuery q(dBConnection);
		q.exec(QString("SELECT ID FROM poem WHERE cat_id = %1 AND id>%2 LIMIT 1").arg(CatID).arg(PoemID) );
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			gPoem = getPoem((qrec.value(0)).toInt());
			return gPoem;
		}
	}
	return gPoem;
}

GanjoorPoem QGanjoorDbBrowser::getNextPoem(GanjoorPoem poem)
{
	return getNextPoem(poem._ID, poem._CatID);
}

GanjoorPoem QGanjoorDbBrowser::getPreviousPoem(int PoemID, int CatID)
{	
	GanjoorPoem gPoem;
	gPoem.init();
	if (isConnected() && PoemID!=-1) // PoemID==-1 when getPreviousPoem(GanjoorPoem poem) pass null poem
	{
		QSqlQuery q(dBConnection);
		q.exec(QString("SELECT ID FROM poem WHERE cat_id = %1 AND id<%2 ORDER BY ID DESC LIMIT 1").arg(CatID).arg(PoemID) );
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			gPoem = getPoem((qrec.value(0)).toInt());
			return  gPoem;
		}
	}
	return gPoem;
}

GanjoorPoem QGanjoorDbBrowser::getPreviousPoem(GanjoorPoem poem)
{
	return getPreviousPoem(poem._ID, poem._CatID);
}

GanjoorPoet QGanjoorDbBrowser::getPoetForCat(int CatID)
{
	GanjoorPoet gPoet;
	gPoet.init();
	if (CatID == 0)
	{
		gPoet.init(0, tr("All"), 0);
		return gPoet;
	}
	if (isConnected())
	{
		QSqlQuery q(dBConnection);
		q.exec("SELECT poet_id FROM cat WHERE id = " + QString::number(CatID) );
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			return getPoet((qrec.value(0)).toInt());
		}
	}
	return gPoet;
}

GanjoorPoet QGanjoorDbBrowser::getPoet(int PoetID)
{
	GanjoorPoet gPoet;
	gPoet.init();
	if (PoetID == 0)
	{
		gPoet.init(0, tr("All"), 0);
		return gPoet;
	}
	if (isConnected())
	{
		QSqlQuery q(dBConnection);
		
		bool descriptionExists = false;
		if ( q.exec("SELECT id, name, cat_id, description FROM poet WHERE id = " + QString::number(PoetID) ) )
			descriptionExists = true;
		else
			q.exec("SELECT id, name, cat_id FROM poet WHERE id = " + QString::number(PoetID) );
		
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			if (descriptionExists)
				gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), (qrec.value(3)).toString());
			else
				gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), "" );

			return gPoet;
		}
	}
	return gPoet;
}

QString QGanjoorDbBrowser::getPoetDescription(int PoetID)
{
	if (PoetID <= 0)
	{
		return "";
	}
	if (isConnected())
	{
		QSqlQuery q(dBConnection);
		q.exec("SELECT description FROM poet WHERE id = " + QString::number(PoetID) );
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			return (qrec.value(0)).toString();
		}
	}
	return "";
}

GanjoorPoet QGanjoorDbBrowser::getPoet(QString PoetName)
{
	GanjoorPoet gPoet;
	gPoet.init();
	if (isConnected())
	{
		QSqlQuery q(dBConnection);

		bool descriptionExists = false;
		if ( q.exec("SELECT id, name, cat_id, description FROM poet WHERE name = \'" + PoetName + "\'") )
			descriptionExists = true;
		else
			q.exec("SELECT id, name, cat_id FROM poet WHERE name = \'" + PoetName + "\'");
		
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			if (descriptionExists)
				gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), (qrec.value(3)).toString());
			else
				gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt(), "");

			return gPoet;
		}
	}
	return gPoet;
}

QList<int> QGanjoorDbBrowser::getPoemIDsContainingPhrase(QString phrase, int PageStart, int Count, int PoetID)
{
	QList<int> idList;
	if (isConnected())
	{
		QString strQuery;
		if (PoetID == 0)
			strQuery=QString("SELECT poem_id FROM verse WHERE text LIKE \'%" + phrase + "%\' GROUP BY poem_id LIMIT "+QString::number(PageStart)+","+QString::number(Count));
		else
			strQuery=QString("SELECT poem_id FROM (verse INNER JOIN poem ON verse.poem_id=poem.id) INNER JOIN cat ON cat.id =cat_id WHERE verse.text LIKE \'%" + phrase + "%\' AND poet_id=" + QString::number(PoetID) + " GROUP BY poem_id LIMIT " + QString::number(PageStart) + "," + QString::number(Count));
		
		QSqlQuery q(dBConnection);
		q.exec(strQuery);

		while( q.next() )
		{
			QSqlRecord qrec = q.record();
			idList.append(qrec.value(0).toInt());
		}
		return idList;
	}
	return idList;
}
QString QGanjoorDbBrowser::getFirstVerseContainingPhrase(int PoemID, QString phrase)
{
	QString text = "";
	if (isConnected())
	{
		QString strQuery="SELECT text FROM verse WHERE poem_id="+QString::number(PoemID)+" AND text LIKE\'%"+phrase+"%\' LIMIT 0,1";
		QSqlQuery q(dBConnection);
		q.exec(strQuery);
		if ( q.next() )
		{
			QSqlRecord qrec = q.record();
			text = qrec.value(0).toString();
		}
	}
	return text;
}

int QGanjoorDbBrowser::getRandomPoemID(int *CatID)
{
	if (isConnected())
	{
		QList<GanjoorCat *> subCats = getSubCategories(*CatID);
		int randIndex = QGanjoorDbBrowser::getRandomNumber(0, subCats.size());
		if (randIndex >= 0 && randIndex != subCats.size()) // '==' is when we want to continue with current 'CatID'. notice maybe 'randIndex == subCats.size() == 0'.
		{
			*CatID = subCats.at(randIndex)->_ID;
			return getRandomPoemID(CatID);
		}

		QString strQuery="SELECT MIN(id), MAX(id) FROM poem";
		if (*CatID != 0)
			strQuery += " WHERE cat_id=" + QString::number(*CatID);
		QSqlQuery q(dBConnection);
		q.exec(strQuery);
		if ( q.next() )
		{
			QSqlRecord qrec = q.record();
			int minID = qrec.value(0).toInt();
			int maxID = qrec.value(1).toInt();

			if (maxID <= 0 || minID < 0)
			{
				QList<GanjoorCat *> subCats = getSubCategories(*CatID);
				*CatID = subCats.at(QGanjoorDbBrowser::getRandomNumber(0, subCats.size()-1))->_ID;
				return getRandomPoemID(CatID);
			}
			return QGanjoorDbBrowser::getRandomNumber(minID, maxID);
		}
	}
	return -1;
}

int QGanjoorDbBrowser::getRandomNumber(int minBound, int maxBound)
{
	if ( (maxBound < minBound) || (minBound < 0) ) return -1;
	if (minBound == maxBound) return minBound;

	//Generate a random number in the range [0.5f, 1.0f).
	unsigned int ret = 0x3F000000 | (0x7FFFFF & ((qrand() << 8) ^ qrand()));
	unsigned short coinFlips;

	//If the coin is tails, return the number, otherwise
	//divide the random number by two by decrementing the
	//exponent and keep going. The exponent starts at 63.
	//Each loop represents 15 random bits, a.k.a. 'coin flips'.
	#define RND_INNER_LOOP() \
	if( coinFlips & 1 ) break; \
	coinFlips >>= 1; \
	ret -= 0x800000
	for(;;){
	coinFlips = qrand();
	RND_INNER_LOOP(); RND_INNER_LOOP(); RND_INNER_LOOP();
	//At this point, the exponent is 60, 45, 30, 15, or 0.
	//If the exponent is 0, then the number equals 0.0f.
	if( ! (ret & 0x3F800000) ) return 0.0f;
	RND_INNER_LOOP(); RND_INNER_LOOP(); RND_INNER_LOOP();
	RND_INNER_LOOP(); RND_INNER_LOOP(); RND_INNER_LOOP();
	RND_INNER_LOOP(); RND_INNER_LOOP(); RND_INNER_LOOP();
	RND_INNER_LOOP(); RND_INNER_LOOP(); RND_INNER_LOOP();
	}

	float rand = *((float *)(&ret));
	return (int)(rand*(maxBound-minBound+1)+minBound);
}

QList<GanjoorPoet *> QGanjoorDbBrowser::getDataBasePoets(const QString fileName)
{
	QList<GanjoorPoet *> poetsInDataBase;
	QString dataBaseID = getIdForDataBase(fileName);
	QSqlDatabase databaseObject = QSqlDatabase::database(dataBaseID);
	if (!databaseObject.open())
	{
		QSqlDatabase::removeDatabase(dataBaseID);
		return poetsInDataBase;
	}

	poetsInDataBase = getPoets(dataBaseID, false);
	QSqlQuery q(databaseObject);

	databaseObject.close();
	QSqlDatabase::removeDatabase(dataBaseID);

	return poetsInDataBase;
}

QString QGanjoorDbBrowser::getIdForDataBase(const QString &sqliteDataBaseName)
{
	QFileInfo dataBaseFile(sqliteDataBaseName);
	QString connectionID = dataBaseFile.fileName();//we need to be sure this name is unique
	QSqlDatabase databaseObject = QSqlDatabase::addDatabase("QSQLITE", connectionID);
	databaseObject.setDatabaseName(sqliteDataBaseName);

	return connectionID;
}

QList<GanjoorPoet *> QGanjoorDbBrowser::getConflictingPoets(const QString fileName)
{
	QList<GanjoorPoet *> conflictList;
	if ( isConnected() )
	{
		QList<GanjoorPoet *> dataBasePoets = getDataBasePoets(fileName);
		QList<GanjoorPoet *> installedPoets = getPoets();
		foreach(GanjoorPoet *poet, installedPoets)
		{
			foreach(GanjoorPoet *dbPoet, dataBasePoets)
			{
				if ( dbPoet->_ID == poet->_ID )
				{
					conflictList << poet;
					break;
				}
			}
		}
	}

    return conflictList;
}

void QGanjoorDbBrowser::removePoetFromDataBase(int PoetID)
{
	if (isConnected())
	{
		QString strQuery;
		QSqlQuery q(dBConnection);

		removeCatFromDataBase( getCategory(getPoet(PoetID)._CatID) );

		strQuery = "DELETE FROM poet WHERE id="+QString::number(PoetID);
		q.exec(strQuery);
	}
}

void QGanjoorDbBrowser::removeCatFromDataBase(const GanjoorCat &gCat)
{
	if ( gCat.isNull() )
        return;
    QList<GanjoorCat *> subCats = getSubCategories(gCat._ID);
    foreach (GanjoorCat *cat, subCats)
        removeCatFromDataBase(*cat);
	if (isConnected())
	{
		QString strQuery;
		QSqlQuery q(dBConnection);
		strQuery="DELETE FROM verse WHERE poem_id IN (SELECT id FROM poem WHERE cat_id="+QString::number(gCat._ID)+")";
		q.exec(strQuery);
		strQuery="DELETE FROM poem WHERE cat_id="+QString::number(gCat._ID);
		q.exec(strQuery);
		strQuery="DELETE FROM cat WHERE id="+QString::number(gCat._ID);
		q.exec(strQuery);
	}
}

int QGanjoorDbBrowser::getNewPoetID()
{
	int newPoetID = -1;

	if (isConnected())
	{
		QString strQuery = "SELECT MAX(id) FROM poet";
		QSqlQuery q(dBConnection);
		q.exec(strQuery);
		
		if (q.first())
		{
			bool ok = false;
			newPoetID = q.value(0).toInt(&ok)+1;
			if (!ok)
				newPoetID = minNewPoetID+1;
		}
	}

	if (newPoetID < minNewPoetID)
		newPoetID = minNewPoetID+1;

    return newPoetID;
}
int QGanjoorDbBrowser::getNewPoemID()
{
    int newPoemID = -1;

    if (cachedMaxPoemID == 0)
    {
		if (isConnected())
		{
			QString strQuery = "SELECT MAX(id) FROM poem";
			QSqlQuery q(dBConnection);
			q.exec(strQuery);
			
			if (q.first())
			{
				bool ok = false;
				newPoemID = q.value(0).toInt(&ok)+1;
				if (!ok)
					newPoemID = minNewPoemID+1;
			}
		}

		if (newPoemID < minNewPoemID)
			newPoemID = minNewPoemID+1;

		cachedMaxPoemID = newPoemID;
	}
	else
		newPoemID = ++cachedMaxPoemID;

    return newPoemID;
}

int QGanjoorDbBrowser::getNewCatID()
{
    int newCatID = -1;

    if (cachedMaxCatID == 0)
    {
		if (isConnected())
		{
			QString strQuery = "SELECT MAX(id) FROM cat";
			QSqlQuery q(dBConnection);
			q.exec(strQuery);
			
			if (q.first())
			{
				bool ok = false;
				newCatID = q.value(0).toInt(&ok)+1;
				if (!ok)
					newCatID = minNewCatID+1;
			}
		}

		if (newCatID < minNewCatID)
			newCatID = minNewCatID+1;

		cachedMaxCatID = newCatID;
	}
	else
		newCatID = ++cachedMaxCatID;

    return newCatID;
}

bool QGanjoorDbBrowser::importDataBase(const QString fileName)
{
	QString connectionID = getIdForDataBase(fileName);
	QSqlDatabase dataBaseObject = QSqlDatabase::database(connectionID);
	if (!dataBaseObject.open())
	{
		QSqlDatabase::removeDatabase(connectionID);
		return false;
	}

	QList<GanjoorPoet *> poets = getPoets(connectionID, false);
	QString strQuery;
	QSqlQuery queryObject(dataBaseObject);
    QMap<int, int> mapPoets;
    QMap<int, int> mapCats;
    
	foreach (GanjoorPoet *newPoet, poets)
	{
		bool insertNewPoet = true;

		GanjoorPoet poet = getPoet(newPoet->_Name);
		if (!poet.isNull())//conflict on Names
		{
			if (poet._ID == newPoet->_ID)
			{
				insertNewPoet = false;
				mapPoets.insert(newPoet->_ID, newPoet->_ID);
				GanjoorCat poetCat = getCategory(newPoet->_CatID);
				if (!poetCat.isNull())
				{
					if (poetCat._PoetID == newPoet->_ID)
					{
						mapCats.insert(newPoet->_CatID, newPoet->_CatID);
					}
					else
					{
						int aRealyNewCatID = getNewCatID();
						mapCats.insert(newPoet->_CatID, getNewCatID());
						newPoet->_CatID = aRealyNewCatID;
					}
				}
			}
			else
			{
				mapPoets.insert(newPoet->_ID, poet._ID);
				mapPoets.insert(newPoet->_CatID, poet._CatID);
				newPoet->_CatID = poet._CatID;
			}
		}
		else
		{
			if ( !getPoet(newPoet->_ID).isNull() )//conflict on IDs
			{
				int aRealyNewPoetID = getNewPoetID();
				mapPoets.insert(newPoet->_ID, aRealyNewPoetID);
				newPoet->_ID = aRealyNewPoetID;

				int aRealyNewCatID = getNewCatID();
				mapCats.insert(newPoet->_CatID, aRealyNewCatID);
				newPoet->_CatID = aRealyNewCatID;
			}
			else //no conflict, insertNew
			{
				mapPoets.insert(newPoet->_ID, newPoet->_ID);
				GanjoorCat newPoetCat = getCategory(newPoet->_CatID);
				if (newPoetCat.isNull())
					mapCats.insert(newPoet->_CatID, newPoet->_CatID);
				else
				{
					int aRealyNewCatID = getNewCatID();
					mapCats.insert(newPoet->_CatID, getNewCatID());
					newPoet->_CatID = aRealyNewCatID;
				}
			}
		}
        
		if(insertNewPoet && isConnected())
		{
			strQuery = QString("INSERT INTO poet (id, name, cat_id, description) VALUES (%1, \"%2\", %3, \"%4\");").arg(newPoet->_ID).arg(newPoet->_Name).arg(newPoet->_CatID).arg(newPoet->_Description);
			QSqlQuery q(dBConnection);
			q.exec(strQuery);
		}                               
	}

	strQuery = QString("SELECT id, poet_id, text, parent_id, url FROM cat");
	queryObject.exec(strQuery);

	QList<GanjoorCat *> gCatList;
	while (queryObject.next())
	{
		GanjoorCat *gCat = new GanjoorCat();
		gCat->init(queryObject.value(0).toInt(), queryObject.value(1).toInt(), queryObject.value(2).toString(), queryObject.value(3).toInt(), queryObject.value(4).toString());
		gCatList.append(gCat);
	}

	foreach (GanjoorCat *newCat, gCatList)
    {
		newCat->_PoetID = mapPoets.value(newCat->_PoetID, newCat->_PoetID);
		newCat->_ParentID = mapCats.value(newCat->_ParentID, newCat->_ParentID);

        bool insertNewCategory = true;
		GanjoorCat gCat = getCategory(newCat->_ID);
		if (!gCat.isNull())
        {
			if (gCat._PoetID == newCat->_PoetID)
            {
                insertNewCategory = false;
                
				if (mapCats.value(newCat->_ID, -1) == -1)
					mapCats.insert(newCat->_ID, newCat->_ID);
            }
            else
            {
                int aRealyNewCatID = getNewCatID();
                mapCats.insert(newCat->_ID, aRealyNewCatID);
                newCat->_ID = aRealyNewCatID;
            }
        }
        else
        {
			if (mapCats.value(newCat->_ID, -1) == -1)
				mapCats.insert(newCat->_ID, newCat->_ID);
        }

		if(insertNewCategory && isConnected())
		{
			strQuery = QString("INSERT INTO cat (id, poet_id, text, parent_id, url) VALUES (%1, %2, \"%3\", %4, \"%5\");").arg(newCat->_ID).arg(newCat->_PoetID).arg(newCat->_Text).arg(newCat->_ParentID).arg(newCat->_Url);
			QSqlQuery q(dBConnection);
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
	QList<GanjoorPoem *> poemList;

	while(queryObject.next())
	{
		GanjoorPoem *gPoem = new GanjoorPoem();
		gPoem->init(queryObject.value(0).toInt(), queryObject.value(1).toInt(), queryObject.value(2).toString(), queryObject.value(3).toString());
		poemList.append(gPoem);
	}

	foreach (GanjoorPoem *newPoem, poemList)
    {
		int tmp = newPoem->_ID;
		if (!getPoem(newPoem->_ID).isNull())
            newPoem->_ID = getNewPoemID();
		newPoem->_CatID = mapCats.value(newPoem->_CatID, newPoem->_CatID);
		dicPoemID.insert(tmp, newPoem->_ID);
		
		if (isConnected())
		{
			strQuery = QString("INSERT INTO poem (id, cat_id, title, url) VALUES (%1, %2, \"%3\", \"%4\");").arg(newPoem->_ID).arg(newPoem->_CatID).arg(newPoem->_Title).arg(newPoem->_Url);
			QSqlQuery q(dBConnection);
			q.exec(strQuery);
		}
    }


    strQuery = "SELECT poem_id, vorder, position, text FROM verse";
	queryObject.exec(strQuery);
	QList<GanjoorVerse *> verseList;

	while(queryObject.next())
	{
		GanjoorVerse *gVerse = new GanjoorVerse();
		gVerse->init(queryObject.value(0).toInt(), queryObject.value(1).toInt(), (VersePosition)queryObject.value(2).toInt(), queryObject.value(3).toString());
		verseList.append(gVerse);
	}

	if (isConnected())
	{
		strQuery = "INSERT INTO verse (poem_id, vorder, position, text) VALUES (:poem_id,:vorder,:position,:text)";
		QSqlQuery q(dBConnection);
		q.prepare(strQuery);

		foreach (GanjoorVerse *newVerse, verseList)
		{
			newVerse->_PoemID = dicPoemID.value(newVerse->_PoemID, newVerse->_PoemID);
			q.bindValue( ":poem_id", newVerse->_PoemID );
			q.bindValue( ":vorder", newVerse->_Order );
			q.bindValue( ":position", newVerse->_Position );
			q.bindValue( ":text", newVerse->_Text );
			q.exec();
		}
	}

	dataBaseObject.close();
	QSqlDatabase::removeDatabase(connectionID);
	return true;
}

QString QGanjoorDbBrowser::cleanString(const QString &text, bool skipNonAlphabet)
{
	QString cleanedText = text;

	for (int i=0; i<cleanedText.size(); ++i)
	{
		QChar tmpChar = cleanedText.at(i);
		QChar::Direction chDir = tmpChar.direction();
		if (chDir == QChar::DirNSM)
		{
			cleanedText.remove(tmpChar);
			--i;
		}

		if (skipNonAlphabet)
		{
			if (chDir != QChar::DirAL && chDir != QChar::DirL && chDir != QChar::DirR)
			{
				cleanedText.remove(tmpChar);
				--i;
			}
		}
	}
	return cleanedText;
}

QList<int> QGanjoorDbBrowser::getPoemIDsContainingPhrase_NewMethod(const QString &phrase, int PoetID, bool skipNonAlphabet)
{
	QList<int> idList;
    if (isConnected())
	{
		QString strQuery;
		QString phraseForSearch = QGanjoorDbBrowser::cleanString(phrase, skipNonAlphabet);
		//qDebug() << "text=" << phrase << "cleanedText=" << phraseForSearch;

		QString ch = QString(phraseForSearch.at(0));
		int numOfFounded=0;

		if (PoetID == 0)
			strQuery=QString("SELECT poem_id FROM verse WHERE text LIKE \'%" + ch + "%\' GROUP BY poem_id");
		else
			strQuery=QString("SELECT poem_id FROM (verse INNER JOIN poem ON verse.poem_id=poem.id) INNER JOIN cat ON cat.id =cat_id WHERE verse.text LIKE \'%" + ch + "%\' AND poet_id=" + QString::number(PoetID) + " GROUP BY poem_id");
		
		QSqlQuery q(dBConnection);
		q.exec(strQuery);

		int numOfNearResult=0;
		
		//progress dialog
		int maxOfProgressBar = 50000;
		QProgressDialog progress(QGanjoorDbBrowser::tr("Searching Data Base..."), QGanjoorDbBrowser::tr("Cancel"), 0, maxOfProgressBar, qApp->activeModalWidget());
		progress.setWindowModality(Qt::WindowModal);
		while( q.next() )
		{
			++numOfNearResult;
			if (numOfNearResult > maxOfProgressBar)
			{
				maxOfProgressBar+=30000;
				progress.setMaximum(maxOfProgressBar);
			}
			 progress.setValue(numOfNearResult);

			 if (progress.wasCanceled())
				 break;
			QSqlRecord qrec = q.record();

			QStringList verseTexts = getVerseListContainingPhrase(qrec.value(0).toInt(), ch);
			for (int k=0; k<verseTexts.size();++k)
			{
				QString foundedVerse = QGanjoorDbBrowser::cleanString(verseTexts.at(k), skipNonAlphabet);

				if (foundedVerse.contains(phraseForSearch, Qt::CaseInsensitive))
				{
					++numOfFounded;
					QString labelText =  QGanjoorDbBrowser::tr("Search Result(s): %1").arg(numOfFounded);
					progress.setLabelText(labelText);

					idList.append(qrec.value(0).toInt());
					
					break;//we need just first result
				}
			}
		}
		progress.setValue(maxOfProgressBar);

	    return idList;
	}
    return idList;
}

QStringList QGanjoorDbBrowser::getVerseListContainingPhrase(int PoemID, const QString &phrase)
{
	QStringList text;
    if (isConnected())
    {
		QString strQuery="SELECT text FROM verse WHERE poem_id="+QString::number(PoemID)+" AND text LIKE\'%"+phrase+"%\'";
		QSqlQuery q(dBConnection);
		q.exec(strQuery);
		while ( q.next() )
		{
			QSqlRecord qrec = q.record();
			text << qrec.value(0).toString();
		}
    }
	return text;
}

QString QGanjoorDbBrowser::justifiedText(const QString &text, const QFontMetrics &fontmetric, int width)
{
	int textSize = text.size();
	int textWidth = fontmetric.width(text);
	QChar tatweel = QChar(0x0640);
	int tatweelWidth = fontmetric.width(tatweel);
	if (tatweel == QChar(QChar::ReplacementCharacter))//Current font has not a TATWEEL character
	{
		tatweel = QChar(QChar::Null);
		tatweelWidth = 0;
	}

	int spaceWidth = fontmetric.width(" ");

	if ( textWidth >= width || width <= 0)
		return text;

	int numOfTatweels = tatweelWidth ? (width - textWidth)/tatweelWidth : 0;
	int numOfSpaces = (width - (textWidth + numOfTatweels*tatweelWidth))/spaceWidth;

	QList<int> tatweelPositions;
	QList<int> spacePositions;
	QStringList charsOfText = text.split("", QString::SkipEmptyParts);

	//founding suitable position for inserting TATWEEL or SPACE characters.
	for (int i=0; i<charsOfText.size(); ++i)
	{
		if (i>0 && i<charsOfText.size()-1 && charsOfText.at(i) == " ")
		{
			spacePositions << i;
			continue;
		}

		if (charsOfText.at(i).at(0).joining() != QChar::Dual) continue;
		
		if ( i<charsOfText.size()-1 && ((charsOfText.at(i+1).at(0).joining() == QChar::Dual) ||
			(charsOfText.at(i+1).at(0).joining() == QChar::Right)) )
		{
			tatweelPositions << i;
			continue;
		}
	}

	if (tatweelPositions.isEmpty())
	{
		numOfTatweels = 0;
		numOfSpaces = (width - textWidth)/spaceWidth;
	}

	if (numOfTatweels>0)
	{
		++numOfTatweels;
		width = numOfSpaces*spaceWidth+numOfTatweels*tatweelWidth+textWidth;
	}
	else
	{
		++numOfSpaces;
		width = numOfSpaces*spaceWidth+numOfTatweels*tatweelWidth+textWidth;
	}

	for (int i=0; i<tatweelPositions.size(); ++i)
	{
		if (numOfTatweels<=0) break;
		charsOfText[tatweelPositions.at(i)] = charsOfText.at(tatweelPositions.at(i))+tatweel;
		--numOfTatweels;
		if (i==tatweelPositions.size()-1) i=-1;
	}
	for (int i=0; i<spacePositions.size(); ++i)
	{
		if (numOfSpaces<=0) break;
		charsOfText[spacePositions.at(i)] = charsOfText.at(spacePositions.at(i))+" ";
		--numOfSpaces;
		if (i==spacePositions.size()-1) i=-1;
	}
	return charsOfText.join("");
}
