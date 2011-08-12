/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
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

#ifndef QGANJOORDBSTUFF_H
#define QGANJOORDBSTUFF_H

#include <QString>

enum VersePosition
{
	Right = 0,//مصرع اول
	Left = 1,// مصرع دوم
	CenteredVerse1 = 2,// مصرع اول یا تنهای ابیات ترجیع یا ترکیب
	CenteredVerse2 = 3,// مصرع دوم ابیات ترجیع یا ترکیب
	Single = 4, //مصرعهای شعرهای نیمایی یا آزاد
	Paragraph = -1, //نثر
};

class GanjoorVerse
{
	public:
		int _PoemID;
		int _Order;
		VersePosition _Position;
		QString _Text;

	GanjoorVerse()
	{
		_PoemID = -1;
		_Order = -1;
		_Position = Single;
		_Text = QString();
	}
	
	inline void init(int PoemID=-1, int Order=-1, VersePosition Position = Single, QString Text=QString() )
	{
		_PoemID = PoemID;
		_Order = Order;
		_Position = Position;
		_Text = Text;
	}
};

class GanjoorPoem
{
	public:
		int _ID;
		int _CatID;
		QString _Title;
		QString _Url;
		QString _HighlightText;
		bool _Faved;

	GanjoorPoem()
	{
		_ID = -1;
		_CatID = -1;
		_Title = QString();
		_Url = QString();
		_Faved = false;
		_HighlightText = QString();
	}

	inline void init(int ID=-1, int CatID=-1, QString Title=QString(), QString Url=QString(), bool Faved=false, QString HighlightText=QString())
	{
		_ID = ID;
		_CatID = CatID;
		_Title = Title;
		_Url = Url;
		_Faved = Faved;
		_HighlightText = HighlightText;
	}
	inline bool isNull() const {return _ID == -1;}
	inline void setNull()
	{
		_ID = -1;
		_CatID = -1;
		_Title = QString();
		_Url = QString();
		_HighlightText = QString();
		_Faved = false;
	}
};

class GanjoorPoet
{
	public:
		QString _Name;
		int _ID;
		int _CatID;
		QString _Description;

	GanjoorPoet()
	{
		_ID = -1;
		_Name = QString();
		_CatID = -1;
		_Description = QString();
	}

	inline void init(int ID=-1, QString Name=QString(), int CatID=-1, QString description=QString())
	{
		_Name = Name;
		_ID = ID;
		_CatID = CatID;
		_Description = description;
	}

	inline bool isNull() const {return _ID == -1;}
};

class GanjoorCat
{
	public:
		int _ID;
		int _PoetID;
		QString _Text;
		int _ParentID;
		QString _Url;

	GanjoorCat()
	{
		_ID = -1;
		_PoetID = -1;
		_Text = QString();
		_ParentID = -1;
		_Url = QString();
	}

	inline bool isNull() const {return _ID == -1;}
	inline void setNull() 
	{
		_ID = -1;
		_PoetID = -1;
		_Text = QString();
		_ParentID = -1;
		_Url = QString();
	}
	inline void init(int ID=-1, int PoetID=-1, QString Text="", int ParentID=-1, QString Url="") 
	{
		_ID = ID;
		_PoetID = PoetID;
		_Text = Text;
		_ParentID = ParentID;
		_Url = Url;
	}
};
#endif // QGANJOORDBSTUFF_H
