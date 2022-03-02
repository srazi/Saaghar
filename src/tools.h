/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2014-2016 by S. Razi Alavizadeh                          *
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

#ifndef TOOLS_H
#define TOOLS_H

#include <QtGlobal>

#define splashScreen(T) (static_cast<T*>(Tools::s_splashScreen))
#define ICON_FILE(X) Tools::iconFileByKey(X)

// x is variant-hash
#define VAR_ADD(x, y) x.insert(#y, QVariant::fromValue(y))
#define VAR_GET(x, y) x.value(#y)

#if QT_VERSION >= 0x050F00
    #define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
    #define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

//#define DEV_TOOLS 1

#include <QFontMetrics>
#include <QStringList>

class QTableWidget;
class QTableWidgetItem;
class QScrollBar;

class Tools
{
public:
    static QString getLongPathName(const QString &fileName);
    static QString simpleCleanString(const QString &text);
    static QString cleanString(const QString &text, const QStringList &excludeList = QStringList() << " ");
    static QString cleanStringFast(const QString &text, const QStringList &excludeList = QStringList() << " ");
    static QString justifiedText(const QString &text, const QFontMetrics &fontmetric, int width);
    static QString snippedText(const QString &text, const QString &str, int from = 0, int maxNumOfWords = 10, bool elided = true, Qt::TextElideMode elideMode = Qt::ElideRight);
    static int getRandomNumber(int minBound, int maxBound);

    static int prefaceIDFromVersion(const QString &version);

    static QString iconFileByKey(const QString &key, bool fallback = true);

    static void setSplashScreen(QObject* splash);
    static void scrollTo(QScrollBar* scrollBar, int value, int duration = 125);
    static void scrollToItem(QTableWidget* table, const QTableWidgetItem* item, int duration = 125);


    static const QStringList someSymbols;
    static const QStringList Ve_Variant;
    static const QStringList Ye_Variant;
    static const QStringList AE_Variant;
    static const QStringList He_Variant;
    static const QString &OTHER_GLYPHS;

    static QObject* s_splashScreen;

    static const QStringList No_KASHIDA_FONTS;
};

#endif // TOOLS_H
