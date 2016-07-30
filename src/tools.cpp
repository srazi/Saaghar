/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2014 by S. Razi Alavizadeh                          *
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

#include "tools.h"
#include "searchresultwidget.h"

#include <QFileInfo>

#ifdef Q_OS_WIN
#define BUFSIZE 4096
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <tchar.h>
#endif

QObject* Tools::s_splashScreen = 0;

////////////////////////////
//from unicode table:
//  "»" cell= 187 row= 0
//  "«" cell= 171 row= 0
//  "." cell= 46 row= 0
//  "؟" cell= 31 row= 6
//  "،" cell= 12 row= 6
//  "؛" cell= 27 row= 6
const QStringList Tools::someSymbols = QStringList()
                                       << QChar(31, 6) << QChar(12, 6) << QChar(27, 6)
                                       << QChar(187, 0) << QChar(171, 0) << QChar(46, 0)
                                       << ")" << "(" << "[" << "]" << ":" << "!" << "-" << "." << "?";

//the zero index of following stringlists is equipped by expected variant!
//  "ؤ" cell= 36 row= 6
//  "و" cell= 72 row= 6
const QStringList Tools::Ve_Variant = QStringList()
                                      << QChar(72, 6) << QChar(36, 6);

//  "ى" cell= 73 row= 6 //ALEF MAKSURA
//  "ي" cell= 74 row= 6 //Arabic Ye
//  "ئ" cell= 38 row= 6
//  "ی" cell= 204 row= 6 //Persian Ye
const QStringList Tools::Ye_Variant = QStringList()
                                      << QChar(204, 6) << QChar(38, 6) << QChar(74, 6) << QChar(73, 6);

//  "آ" cell= 34 row= 6
//  "أ" cell= 35 row= 6
//  "إ" cell= 37 row= 6
//  "ا" cell= 39 row= 6
const QStringList Tools::AE_Variant = QStringList()
                                      << QChar(39, 6) << QChar(37, 6) << QChar(35, 6) << QChar(34, 6);

//  "ة" cell= 41 row= 6
//  "ۀ" cell=192 row= 6
//  "ه" cell= 71 row= 6
const QStringList Tools::He_Variant = QStringList()
                                      << QChar(71, 6) << QChar(41, 6) << QChar(192, 6);

// tashdid+keshide+o+a+e+an+en+on+saaken+zwnj
const QString &Tools::OTHER_GLYPHS = QString(QChar(78, 6)) +
                                     QChar(79, 6) + QChar(80, 6) + QChar(81, 6) + QChar(64, 6) +
                                     QChar(75, 6) + QChar(76, 6) + QChar(77, 6) + QChar(82, 6) + QChar(0x200C);
/***************************/
const QRegExp Ve_EXP = QRegExp("[" +
                               Tools::Ve_Variant.join("").remove(Tools::Ve_Variant.at(0))
                               + "]");
const QRegExp Ye_EXP = QRegExp("[" +
                               Tools::Ye_Variant.join("").remove(Tools::Ye_Variant.at(0))
                               + "]");
const QRegExp AE_EXP = QRegExp("[" +
                               Tools::AE_Variant.join("").remove(Tools::AE_Variant.at(0))
                               + "]");
const QRegExp He_EXP = QRegExp("[" +
                               Tools::He_Variant.join("").remove(Tools::He_Variant.at(0))
                               + "]");
const QRegExp SomeSymbol_EXP = QRegExp("[" +
                                       Tools::someSymbols.join("")
                                       + "]+");
/***************************/
////////////////////////////

const QStringList Tools::No_KASHIDA_FONTS = QStringList()
                                            << "IranNastaliq";

QString Tools::getLongPathName(const QString &fileName)
{
#ifndef Q_OS_WIN
    QFileInfo file(fileName);
    if (!file.exists()) {
        return fileName;
    }
    else {
        return file.canonicalFilePath();
    }
#else
    DWORD  retval = 0;
//#ifdef D_MSVC_CC
//    TCHAR  bufLongFileName[BUFSIZE] = TEXT("");
//    TCHAR  bufFileName[BUFSIZE] = TEXT("");
//#else
//    TCHAR  bufLongFileName[BUFSIZE] = TEXT(L"");
//    TCHAR  bufFileName[BUFSIZE] = TEXT(L"");
//#endif
    TCHAR  bufLongFileName[BUFSIZE] = TEXT("");
    TCHAR  bufFileName[BUFSIZE] = TEXT("");

    QString winFileName = fileName;
    winFileName.replace("/", "\\");
    winFileName.toWCharArray(bufFileName);

    retval = GetLongPathName(bufFileName, bufLongFileName, BUFSIZE);

    if (retval == 0) {
        qWarning("GetLongPathName failed (%d)\n", int(GetLastError()));
        QFileInfo file(fileName);
        if (!file.exists()) {
            return fileName;
        }
        else {
            return file.canonicalFilePath();
        }
    }
    else {
        QString longFileName = QString::fromWCharArray(bufLongFileName);
        longFileName.replace("\\", "/");

        return longFileName;
    }
#endif
}

QString Tools::simpleCleanString(const QString &text)
{
    QString simpleCleaned = text.simplified();
    QChar tatweel = QChar(0x0640);
    simpleCleaned.remove(tatweel);

    return simpleCleaned;
}

QString Tools::cleanString(const QString &text, const QStringList &excludeList)
{
    QString cleanedText = text;

    cleanedText.replace(QChar(0x200C), "", Qt::CaseInsensitive);//replace ZWNJ by ""

    // 14s-->10s
    if (SearchResultWidget::skipVowelLetters) {
        cleanedText.replace(Ve_EXP, QChar(72, 6) /*Ve_Variant.at(0)*/);
        cleanedText.replace(Ye_EXP, QChar(204, 6) /*Ye_Variant.at(0)*/);
        cleanedText.replace(AE_EXP, QChar(39, 6) /*AE_Variant.at(0)*/);
        cleanedText.replace(He_EXP, QChar(71, 6) /*He_Variant.at(0)*/);
    }

    for (int i = 0; i < cleanedText.size(); ++i) {
        QChar tmpChar = cleanedText.at(i);

        if (excludeList.contains(tmpChar)) {
            continue;
        }

        QChar::Direction chDir = tmpChar.direction();

        //someSymbols.contains(tmpChar) consumes lots of time, 8s --> 10s
        if ((SearchResultWidget::skipVowelSigns && chDir == QChar::DirNSM) ||
                tmpChar.isSpace() || someSymbols.contains(tmpChar)) {
            cleanedText.remove(tmpChar);
            --i;
            continue;
        }
    }
    return cleanedText;
}

QString Tools::cleanStringFast(const QString &text, const QStringList &excludeList)
{
    QString cleanedText = text;

    cleanedText.remove(QChar(0x200C));//remove ZWNJ by ""

    // 14s-->10s
    if (SearchResultWidget::skipVowelLetters) {
        cleanedText.replace(Ve_EXP, QChar(72, 6) /*Ve_Variant.at(0)*/);
        cleanedText.replace(Ye_EXP, QChar(204, 6) /*Ye_Variant.at(0)*/);
        cleanedText.replace(AE_EXP, QChar(39, 6) /*AE_Variant.at(0)*/);
        cleanedText.replace(He_EXP, QChar(71, 6) /*He_Variant.at(0)*/);
    }
    cleanedText.remove(SomeSymbol_EXP);

    for (int i = 0; i < cleanedText.size(); ++i) {
        QChar tmpChar = cleanedText.at(i);

        if (excludeList.contains(tmpChar)) {
            continue;
        }

        QChar::Direction chDir = tmpChar.direction();

        //someSymbols.contains(tmpChar) consumes lots of time, 8s --> 10s
        if ((SearchResultWidget::skipVowelSigns && chDir == QChar::DirNSM) || tmpChar.isSpace()) {
            cleanedText.remove(tmpChar);
            --i;
            continue;
        }
    }
    return cleanedText;
}

QString Tools::justifiedText(const QString &text, const QFontMetrics &fontmetric, int width)
{
    int textWidth = fontmetric.width(text);
    QChar tatweel = QChar(0x0640);
    int tatweelWidth = fontmetric.width(tatweel);

    //Current font has not a TATWEEL character
    if (tatweel == QChar(QChar::ReplacementCharacter)) {
        tatweel = QChar(QChar::Null);
        tatweelWidth = 0;
    }

//force justification by space on MacOSX
#ifdef Q_OS_MAC
    tatweel = QChar(QChar::Null);
    tatweelWidth = 0;
#endif

    int spaceWidth = fontmetric.width(" ");

    if (textWidth >= width || width <= 0) {
        return text;
    }

    int numOfTatweels = tatweelWidth ? (width - textWidth) / tatweelWidth : 0;
    int numOfSpaces = (width - (textWidth + numOfTatweels * tatweelWidth)) / spaceWidth;

    QList<int> tatweelPositions;
    QList<int> spacePositions;
    QStringList charsOfText = text.split("", QString::SkipEmptyParts);

    //founding suitable position for inserting TATWEEL or SPACE characters.
    for (int i = 0; i < charsOfText.size(); ++i) {
        if (i > 0 && i < charsOfText.size() - 1 && charsOfText.at(i) == " ") {
            spacePositions << i;
            continue;
        }

        if (charsOfText.at(i).at(0).joining() != QChar::Dual) {
            continue;
        }

        if (i < charsOfText.size() - 1 && ((charsOfText.at(i + 1).at(0).joining() == QChar::Dual) ||
                                           (charsOfText.at(i + 1).at(0).joining() == QChar::Right))) {
            tatweelPositions << i;
            continue;
        }
    }

    if (tatweelPositions.isEmpty()) {
        numOfTatweels = 0;
        numOfSpaces = (width - textWidth) / spaceWidth;
    }

    if (numOfTatweels > 0) {
        ++numOfTatweels;
        width = numOfSpaces * spaceWidth + numOfTatweels * tatweelWidth + textWidth;
    }
    else {
        ++numOfSpaces;
        width = numOfSpaces * spaceWidth + numOfTatweels * tatweelWidth + textWidth;
    }

    for (int i = 0; i < tatweelPositions.size(); ++i) {
        if (numOfTatweels <= 0) {
            break;
        }
        charsOfText[tatweelPositions.at(i)] = charsOfText.at(tatweelPositions.at(i)) + tatweel;
        --numOfTatweels;
        if (i == tatweelPositions.size() - 1) {
            i = -1;
        }
    }
    for (int i = 0; i < spacePositions.size(); ++i) {
        if (numOfSpaces <= 0) {
            break;
        }
        charsOfText[spacePositions.at(i)] = charsOfText.at(spacePositions.at(i)) + " ";
        --numOfSpaces;
        if (i == spacePositions.size() - 1) {
            i = -1;
        }
    }
    return charsOfText.join("");
}

QString Tools::snippedText(const QString &text, const QString &str, int from, int maxNumOfWords, bool elided, Qt::TextElideMode elideMode)
{
    if (!str.isEmpty() && !text.contains(str)) {
        return "";
    }

    QString elideString;
    if (elided) {
        elideString = QLatin1String("...");
    }

    if (!str.isEmpty() && text.contains(str)) {
        int index = text.indexOf(str, from);
        if (index == -1) {
            return "";    //it's not possible
        }
        int partMaxNumOfWords = maxNumOfWords / 2;
        if (text.right(text.size() - str.size() - index).split(" ", QString::SkipEmptyParts).size() < partMaxNumOfWords) {
            partMaxNumOfWords = maxNumOfWords - text.right(text.size() - str.size() - index).split(" ", QString::SkipEmptyParts).size();
        }
        QString leftPart = Tools::snippedText(text.left(index), "", 0, partMaxNumOfWords, elided, Qt::ElideLeft);

        if (leftPart.split(" ", QString::SkipEmptyParts).size() < maxNumOfWords / 2) {
            partMaxNumOfWords = maxNumOfWords - leftPart.split(" ", QString::SkipEmptyParts).size();
        }
        QString rightPart = Tools::snippedText(text.right(text.size() - str.size() - index), "", 0, partMaxNumOfWords, elided, Qt::ElideRight);

        return leftPart + str + rightPart;
    }

    QStringList words = text.split(QRegExp("\\b"), QString::SkipEmptyParts);
    int textWordCount = words.size();
    if (textWordCount < maxNumOfWords || maxNumOfWords <= 0) {
        return text;
    }

    QStringList snippedList;
    int numOfInserted = 0;
    for (int i = 0; i < textWordCount; ++i) {
        if (numOfInserted >= maxNumOfWords) {
            break;
        }

        if (elideMode == Qt::ElideLeft) {
            snippedList.prepend(words.at(textWordCount - 1 - i));
            if (words.at(i) != " ") {
                ++numOfInserted;
            }
        }
        else {
            snippedList << words.at(i);
            if (words.at(i) != " ") {
                ++numOfInserted;
            }
        }
    }

    QString snippedString = snippedList.join("");
    if (snippedString == text) {
        return text;
    }
    if (elideMode == Qt::ElideLeft) {
        return elideString + snippedList.join("");
    }
    else {
        return snippedList.join("") + elideString;
    }
}


int Tools::getRandomNumber(int minBound, int maxBound)
{
    if ((maxBound < minBound) || (minBound < 0)) {
        return -1;
    }
    if (minBound == maxBound) {
        return minBound;
    }

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
    for (;;) {
        coinFlips = qrand();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        //At this point, the exponent is 60, 45, 30, 15, or 0.
        //If the exponent is 0, then the number equals 0.0f.
        if (!(ret & 0x3F800000)) {
            return 0.0f;
        }
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
        RND_INNER_LOOP();
    }

    float rand = *((float*)(&ret));
    return (int)(rand * (maxBound - minBound + 1) + minBound);
}

void Tools::setSplashScreen(QObject* splash)
{
    s_splashScreen = splash;
}
