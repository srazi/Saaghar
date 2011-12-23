/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
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

#ifndef SEARCHPATTERNMANAGER_H
#define SEARCHPATTERNMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QVector>

class SearchPatternManager : public QObject
{
Q_OBJECT
public:
	SearchPatternManager();
	~SearchPatternManager();

	enum Operator
	{
		Or = 0,
		WithOut = 10, //one argument just after it.
		WholeWord = 20,
		Any = 30,
		And = 40,
		NonAlphabetReplacer = 50,
		Near = 60 //one argument just after it
	};

	static void init();
	static void setOperator(SearchPatternManager::Operator op, const QString &str);
	static void setWildcardCharacter(const QString &str);
	static void setInputPhrase(const QString &str);
	static QVector<QStringList> outputPhrases();
	static QVector<QStringList> outputExcludedLlist();
	static void filterResults(QStringList *resultList);
	static QStringList phraseToList(const QString &str, bool removeWildCard = true);
	//static void setSpaceOperator(SearchPatternManager::Operator op);
	static int uniqueKeysCount;

private:
	static QString OP(SearchPatternManager::Operator op);
	static QString clearedPhrase(const QString &str);
	static QMap<SearchPatternManager::Operator, QString> opList;
	static QString wildcardCharacter;
	static QString phraseForSearch;
	static QMap<int, QString> computedPhraseList;//a multi value Map
	static QMap<int, QString> relatedExcludeList;//a multi value Map
};
#endif // SEARCHPATTERNMANAGER_H
