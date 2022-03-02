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
    static SearchPatternManager* instance();
    ~SearchPatternManager();

    enum Operator {
        Or = 0,
        WithOut = 10, //one argument just after it.
        WholeWord = 20,
        Any = 30,
        And = 40,
        NonAlphabetReplacer = 50,
        Near = 60 //one argument just after it
    };

    void init();
    void setOperator(SearchPatternManager::Operator op, const QString &str);
    void setWildcardCharacter(const QString &str);
    void setInputPhrase(const QString &str);
    QVector<QStringList> outputPhrases();
    QVector<QStringList> outputExcludedLlist();
    void filterResults(QStringList*);
    QStringList phraseToList(const QString &str, bool removeWildCard = true);

private:
    Q_DISABLE_COPY(SearchPatternManager)
    SearchPatternManager(QObject* parent = 0);
    static SearchPatternManager* s_instance;

    QString OP(SearchPatternManager::Operator op);
    QString clearedPhrase(const QString &str);

    QMap<SearchPatternManager::Operator, QString> m_operators;
    QString m_wildcardCharacter;
    QString m_phraseForSearch;
#if QT_VERSION_MAJOR >= 6
    QMultiMap<int, QString> m_computedPhraseList;
    QMultiMap<int, QString> m_relatedExcludeList;
#else
    QMap<int, QString> m_computedPhraseList;
    QMap<int, QString> m_relatedExcludeList;
#endif
    int m_uniqueKeysCount;
};
#endif // SEARCHPATTERNMANAGER_H
