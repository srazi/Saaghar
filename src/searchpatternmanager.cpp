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

#include "searchpatternmanager.h"
#include "tools.h"

#include <QApplication>
#include <QDebug>

SearchPatternManager* SearchPatternManager::s_instance = 0;

SearchPatternManager::SearchPatternManager(QObject* parent)
    : QObject(parent),
      m_operators(),
      m_wildcardCharacter(QLatin1Char('%')),
      m_phraseForSearch(),
      m_computedPhraseList(),
      m_relatedExcludeList(),
      m_uniqueKeysCount(0)
{
    setOperator(SearchPatternManager::Or, QLatin1String("|"));
    setOperator(SearchPatternManager::Any, QLatin1String("*"));
    setOperator(SearchPatternManager::And, QLatin1String("+"));
    setOperator(SearchPatternManager::WithOut, QLatin1String("-"));
    setOperator(SearchPatternManager::WholeWord, QLatin1String("\""));
}

SearchPatternManager* SearchPatternManager::instance()
{
    if (!s_instance) {
        s_instance = new SearchPatternManager(qApp);
    }

    return s_instance;
}

SearchPatternManager::~SearchPatternManager()
{
}

void SearchPatternManager::init()
{
    m_computedPhraseList.clear();
    m_relatedExcludeList.clear();
    m_uniqueKeysCount = 0;
    QString initilizedPhrase = SearchPatternManager::clearedPhrase(SearchPatternManager::m_phraseForSearch);

    initilizedPhrase = initilizedPhrase.replace(OP(WholeWord), " ");//moved here from subAndList loop

    QStringList orList = initilizedPhrase.split(OP(Or), QString::SkipEmptyParts);

    for (int i = 0; i < orList.size(); ++i) {
        QStringList subAndList = orList.at(i).split(OP(And));
        for (int j = 0; j < subAndList.size(); ++j) {
            QString str = subAndList.at(j);
            if (str.startsWith(OP(WithOut))) {
                str = str.remove(0, 1);
                str = Tools::cleanString(str);
                m_relatedExcludeList.insertMulti(i, str);
                subAndList[j] = "";
                continue;
            }
//          else //if start with other mono operand's operator!
//          {
//              computedPhraseList.insertMulti(i, str);
//          }
        }
        subAndList.removeAll("");
        int andListSize = subAndList.size();
        for (int j = 0; j < andListSize; ++j) {
            QString str = subAndList.at(j);
            if (str.remove(m_wildcardCharacter).isEmpty()) { //for remove empty items
                continue;
            }
            //str = str.replace(OP(WholeWord), " ");//moved to the first
            //str = wildcardCharacter+str+wildcardCharacter;
            str = Tools::cleanString(subAndList.at(j));
            m_computedPhraseList.insertMulti(i, str);
        }
    }

    m_uniqueKeysCount = m_computedPhraseList.uniqueKeys().size();
}

void SearchPatternManager::setOperator(SearchPatternManager::Operator op, const QString &str)
{
    m_operators.insert(op, str);
}

void SearchPatternManager::setWildcardCharacter(const QString &str)
{
    m_wildcardCharacter = str;
}

void SearchPatternManager::setInputPhrase(const QString &str)
{
    SearchPatternManager::m_phraseForSearch = str;
}

/*static */
QVector<QStringList> SearchPatternManager::outputPhrases(/*int i*/)
{
//  QMap<int, QString>::const_iterator i = computedPhraseList.constBegin();
//  while (i != computedPhraseList.constEnd())
//  {
//      QStringList ithSubPhrase = computedPhraseList.values(i.key());
//      qDebug() << i.key()<< "th-SubPhrase=" << ithSubPhrase.join("|");
//      ++i;
//  }
    QList<int> keys = m_computedPhraseList.uniqueKeys();
    int lsize = keys.size();
    QVector<QStringList> vector(lsize, QStringList());
    for (int i = 0; i < lsize; ++i) {
        QStringList ithSubPhrase = m_computedPhraseList.values(keys.at(i));
        ithSubPhrase.join("|");
        vector.replace(i, ithSubPhrase);
    }
    return vector;
}

QVector<QStringList> SearchPatternManager::outputExcludedLlist()
{
//  QMap<int, QString>::const_iterator i = computedPhraseList.constBegin();
//  while (i != computedPhraseList.constEnd())
//  {
//      QStringList ithSubPhrase = computedPhraseList.values(i.key());
//      qDebug() << i.key()<< "th-SubPhrase=" << ithSubPhrase.join("|");
//      ++i;
//  }
    QList<int> keys = m_computedPhraseList.uniqueKeys();
    int lsize = keys.size();
    QVector<QStringList> vector(lsize, QStringList());
    for (int i = 0; i < lsize; ++i) {
        QStringList ithSubExclude = m_relatedExcludeList.values(keys.at(i));
        ithSubExclude.join("|");
        vector.replace(i, ithSubExclude);
    }
    return vector;
}

/*static */
void SearchPatternManager::filterResults(QStringList* /*resultList*/)
{}

/*static */
QString SearchPatternManager::OP(SearchPatternManager::Operator op)
{
    return m_operators.value(op, "");
}

QString SearchPatternManager::clearedPhrase(const QString &str)
{
    QString clearedString = str;

    clearedString.replace(OP(WithOut), " " + OP(WithOut));

    clearedString.simplified();

    clearedString.replace(OP(WithOut) + " ", " ");

    if (clearedString.count(OP(WholeWord)) % 2 != 0) {
        clearedString.remove(clearedString.lastIndexOf(OP(WholeWord)), 1);
    }

    clearedString.replace(OP(And) + " ", OP(And));
    clearedString.replace(" " + OP(And), OP(And));

    clearedString.replace(OP(Any) + " ", OP(Any));
    clearedString.replace(" " + OP(Any), OP(Any));

    clearedString.replace(OP(Or) + " ", OP(Or));
    clearedString.replace(" " + OP(Or), OP(Or));

    //maybe we change this behaivior//a tricky and temporary method
    clearedString.replace(OP(Any) + OP(Any), m_wildcardCharacter + m_wildcardCharacter);

    //clear doublicates
    clearedString = clearedString.split(OP(WithOut), QString::SkipEmptyParts).join(OP(WithOut));
    clearedString = clearedString.split(OP(Any), QString::SkipEmptyParts).join(OP(Any));
    clearedString = clearedString.split(OP(Or), QString::SkipEmptyParts).join(OP(Or));
    clearedString = clearedString.split(OP(And), QString::SkipEmptyParts).join(OP(And));

    //maybe we change this behaivior
    clearedString.replace(OP(Any), m_wildcardCharacter);

    QStringList wholeWordList = clearedString.split(OP(WholeWord));
    //The odd indices are the ones enclosed by SearchPatternManager::WholeWord
    //remove operators from wholeword phrase
    for (int i = 0; i < wholeWordList.size(); ++i) {
        QString tmp = wholeWordList.at(i);
        if (tmp.isEmpty()) {
            continue;
        }
        if (i % 2 == 1) {
            tmp.remove(OP(Or));
            tmp.remove(OP(Any));
            tmp.remove(OP(And));
            tmp.remove(OP(WithOut));
            //tmp.remove(wildcardCharacter);
            //tmp.replace(" ", OP(WithOut)+OP(WithOut));//a temporary raplacement
        }
        else {
            tmp.replace(" ", OP(And));
        }
        wholeWordList[i] = tmp;
    }
    clearedString = wholeWordList.join(OP(WholeWord));

    return clearedString;
}

/*static*/
QStringList SearchPatternManager::phraseToList(const QString &str,  bool removeWildCard)
{
    QString tmp = SearchPatternManager::clearedPhrase(str);
    tmp.replace(OP(Or), " ");
    tmp.replace(OP(Any), " ");
    tmp.replace(OP(And), " ");
    tmp.replace(OP(WholeWord), " ");
    tmp.replace(m_wildcardCharacter + m_wildcardCharacter, " ");
    /**********************************************/
    //TODO: CHANGE THIS!!
    //for Rhyme finder need to changed
    tmp.remove("=");
    /**********************************************/

    if (removeWildCard) {
        tmp.replace(m_wildcardCharacter, " ");
    }
    else {
        tmp.replace(m_wildcardCharacter, "@");
    }

    QStringList list = tmp.split(" ", QString::SkipEmptyParts);
    int listSize = list.size();
    for (int i = 0; i < listSize; ++i) {
        if (list.at(i).startsWith(OP(WithOut), Qt::CaseInsensitive)) {
            list[i] = "";
        }
        else {
            list[i] = Tools::cleanString(list.at(i));
        }
    }
    list.removeAll("");
    return list;
}
