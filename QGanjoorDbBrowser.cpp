/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://www.pojh.co.cc>       *
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
#include <QDateTime>

const int DatabaseVersion = 1;

QGanjoorDbBrowser::QGanjoorDbBrowser(QString sqliteDbCompletePath)
{
	QFileInfo dBFile(sqliteDbCompletePath);
    dBName = dBFile.fileName();
	dBConnection = QSqlDatabase::addDatabase("QSQLITE", dBName);
    dBConnection.setDatabaseName(sqliteDbCompletePath);
	if (!dBFile.exists() || !dBConnection.open())
    {
		QMessageBox::critical(0, tr("Cannot open Database File"), tr("Cannot open database file...\nError: %1\nDataBase Path=%2\nSaaghar needs its database for working properly, Saaghar is terminated for now.").arg(dBConnection.lastError().text()).arg(sqliteDbCompletePath));
		dBConnection = QSqlDatabase();
		QSqlDatabase::removeDatabase(dBName);
		exit(1);
	}
}

QGanjoorDbBrowser::~QGanjoorDbBrowser()
{
	dBConnection = QSqlDatabase::database(dBName);
	dBConnection.close();
}

bool QGanjoorDbBrowser::isConnected()
{
	dBConnection = QSqlDatabase::database(dBName);
	return dBConnection.isOpen();
}

QList<GanjoorPoet *> QGanjoorDbBrowser::getPoets()
{
    QList<GanjoorPoet *> poets;
    if ( isConnected() )
    {
		QSqlQuery q(dBConnection);
		q.exec("SELECT id, name, cat_id FROM poet");
		q.first();
		
		while( q.isValid() && q.isActive() )
		{
			GanjoorPoet *gPoet = new GanjoorPoet();
			QSqlRecord qrec = q.record();
			gPoet->init(qrec.value(0).toInt(),	qrec.value(1).toString(), qrec.value(2).toInt() );
			poets.append(gPoet);
			if (!q.next()) break;
		}
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
		q.first();//temp
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
		q.exec("SELECT id, name, cat_id FROM poet WHERE id = " + QString::number(PoetID) );
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt());
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
		q.exec("SELECT id, name, cat_id FROM poet WHERE name = \'" + PoetName + "\'");
		q.first();
		if (q.isValid() && q.isActive())
		{
			QSqlRecord qrec = q.record();
			gPoet.init((qrec.value(0)).toInt(), (qrec.value(1)).toString(), (qrec.value(2)).toInt());
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
		//q.first();
		while( q.next() /*q.isValid() && q.isActive()*/ )
		{
			QSqlRecord qrec = q.record();
			idList.append(qrec.value(0).toInt());
			//if (!q.next()) break;
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

int QGanjoorDbBrowser::getRandomPoemID(int CatID)
{
    if (isConnected())
    {
		QString strQuery="SELECT MIN(id), MAX(id) FROM poem";
		if (CatID != 0)
			strQuery += " WHERE cat_id=" + QString::number(CatID);
		QSqlQuery q(dBConnection);
		q.exec(strQuery);
		if ( q.next() )
		{
			QSqlRecord qrec = q.record();
			int minID = qrec.value(0).toInt();
			int maxID = qrec.value(1).toInt();
			uint numOfSecs = QDateTime::currentDateTime().toTime_t();
			uint seed = QCursor::pos().x()+QCursor::pos().y()+numOfSecs+QDateTime::currentDateTime().time().msec();
				
			qsrand(seed);
			int randNumber = qrand();
			while ( randNumber > maxID || randNumber < minID )
			{
				randNumber = qrand();
			}
			return randNumber;
		}
    }
	return -1;
}
