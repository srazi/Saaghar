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
#include "txtimporter.h"
#include "importermanager.h"

#include <QObject>
#include <QDebug>

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

TxtImporter::TxtImporter() : ImporterInterface()
{
}

TxtImporter::~TxtImporter()
{
    ImporterManager::instance()->unRegisterImporter("txt");
}

QString TxtImporter::readableName() const
{
    return "UTF-8 Text file";
}

QString TxtImporter::suffix() const
{
    return "txt";
}

void TxtImporter::import(const QString &data)
{
    m_catContents.clear();

    QString rawdata = data;
    rawdata = rawdata.replace("\r\n", "\n");
    rawdata = rawdata.replace("\r", "\n");

    QStringList lines = rawdata.split("\n");

    const QString catEndMark = QLatin1String("#SED!CAT!END!");

#if QT_VERSION >= 0x050000
    const QRegularExpression catTitleRegExp("^#CAT!TITLE!(.+)$");
#else
    const QRegExp catTitleRegExp("^#CAT!TITLE!(.+)$");
#endif

    const bool justWhitePoem = m_options.contentTypes == Options::WhitePoem;
    const bool justNormalText = m_options.contentTypes == Options::NormalText;
    const bool justClassicalPoem = m_options.contentTypes == Options::Poem;

    const int catId = 10000;
    const int poemId = 100000;

    bool createNewPoem = true;
    bool createNewCat = false;
    bool startPassed = false;
    bool noTitle = false;
    bool maybeParagraph = false;
    bool maybeSingle = false;
    bool maybePoem = false;
    GanjoorCat cat;

    QList<GanjoorCat> parentCats;
    // null cat is considered as root category
    parentCats << cat;

    GanjoorPoem poem;
    GanjoorVerse verse;
    QList<GanjoorVerse> verses;
    int matchCatTitleCount = 0;
    int emptyLineCount = 0;
    int vorder = 0;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines.at(i);

        if (!startPassed && line.trimmed().isEmpty()) {
            continue;
        }

        startPassed = true;

        // go one level up
        if (line.trimmed() == catEndMark) {
            if (!cat.isNull() && !m_catContents.cats.contains(cat._ID)) {
                qDebug() << "\n----- 1 ------\n"
                         << cat._Text << "\n"
                         << cat._ID << "\n"
                         << cat._ParentID << "\n"
                         << poem._Title << "\n"
                         << poem._ID << "\n"
                         << poem._CatID << "\n"
                         << "\n----- 2 ------\n";
                m_catContents.cats.insert(cat._ID, cat);
            }

            if (parentCats.size() > 1) {
                cat = parentCats.takeFirst();
                qDebug() << "\n----- 3 ------\n"
                         << cat._Text << "\n"
                         << cat._ID << "\n"
                         << cat._ParentID << "\n"
                         << poem._Title << "\n"
                         << poem._ID << "\n"
                         << poem._CatID << "\n"
                         << "\n----- 4 ------\n";
            }
            else {
                cat.setNull();
            }

            createNewPoem = true;
            continue;
        }
#if QT_VERSION >= 0x050000
        QRegularExpressionMatch match = catTitleRegExp.match(line);
        if (match.hasMatch()) {
            QString cap1 = match.captured(1);
#else
        if (catTitleRegExp.indexIn(line) >= 0) {
            QString cap1 = catTitleRegExp.cap(1);
#endif
            ++matchCatTitleCount;

            if (!cat.isNull() && !m_catContents.cats.contains(cat._ID)) {
                qDebug() << "\n----- 5 ------\n"
                         << cat._Text << "\n"
                         << cat._ID << "\n"
                         << cat._ParentID << "\n"
                         << poem._Title << "\n"
                         << poem._ID << "\n"
                         << poem._CatID << "\n"
                         << "\n----- 6 ------\n";
                m_catContents.cats.insert(cat._ID, cat);
                parentCats.prepend(cat);
            }

//            if (matchCatTitleCount > 1) {
//                parentCatId = cat._ID;
//            }

            cat.setNull();
            cat._Text = cap1;
            cat._ID = catId + i;
            cat._ParentID = parentCats.at(0)._ID;
            qDebug() << "\n----- 7 ------\n"
                     << cat._Text << "\n"
                     << cat._ID << "\n"
                     << cat._ParentID << "\n"
                     << poem._Title << "\n"
                     << poem._ID << "\n"
                     << poem._CatID << "\n"
                     << "\n----- 8 ------\n";

            createNewPoem = true;
            continue;
        }

        matchCatTitleCount = 0;

        if (!line.trimmed().isEmpty() && ((maybePoem && line.trimmed().size() < 50 && m_options.poemStartPattern.isEmpty()) ||
                                          (!m_options.poemStartPattern.isEmpty() &&
#if QT_VERSION >= 0x050000
                                           line.contains(QRegularExpression(m_options.poemStartPattern))))) {
#else
                                           line.contains(QRegExp(m_options.poemStartPattern))))) {
#endif
            if (!poem.isNull()) {
                m_catContents.verses.insert(poem._ID, verses);
                m_catContents.poems.append(poem);
            }
            verses.clear();
            poem.setNull();
            poem._CatID = cat._ID;
            poem._ID = poemId + i;
            poem._Title = line.trimmed();
            if (poem._Title.contains("THE PICTURE") || poem._Title.contains("TUESDAY")) {
                qDebug() << "\n----- A ------\n"
                         << cat._Text << "\n"
                         << cat._ID << "\n"
                         << cat._ParentID << "\n"
                         << poem._Title << "\n"
                         << poem._ID << "\n"
                         << poem._CatID << "\n"
                         << "\n----- B ------\n";
            }
            createNewPoem = false;
            noTitle = true;
            maybePoem = false;
            maybeParagraph = false;
            vorder = 0;
            continue;
        }

        if (createNewPoem) {
            if (!poem.isNull()) {
                m_catContents.verses.insert(poem._ID, verses);
                m_catContents.poems.append(poem);
            }
            verses.clear();
            poem.setNull();
            poem._CatID = cat._ID;
            poem._ID = poemId + i;
            createNewPoem = false;
            noTitle = false;
            maybePoem = false;
            maybeParagraph = false;
            vorder = 0;
            continue;
        }

        if (line.trimmed().isEmpty()) {
            ++emptyLineCount;

            if (!verse._Text.isEmpty()) {
                verse._Order = vorder;
                verses.append(verse);
                ++vorder;
                verse._Text.clear();
            }

            continue;
        }

        if (m_options.poemStartPattern.isEmpty() && !noTitle && poem._Title.isEmpty()) {
            if (line.size() < 50) {
                poem._Title = line.trimmed();
                if (poem._Title.contains("THE PICTURE") || poem._Title.contains("TUESDAY")) {
                    qDebug() << "\n----- C ------\n"
                             << cat._Text << "\n"
                             << cat._ID << "\n"
                             << cat._ParentID << "\n"
                             << poem._Title << "\n"
                             << poem._ID << "\n"
                             << poem._CatID << "\n"
                             << "\n----- D ------\n";
                }
                continue;
            }
        }

        if (emptyLineCount == 1) {
            if (!justWhitePoem && !justClassicalPoem) {
                if (line.size() > 70) {
                    maybeParagraph = true;
                }
                else {
                    maybeParagraph = false;
                }
            }
            else if (justWhitePoem || justClassicalPoem) {
                maybePoem = true;
            }
        }
        else if (emptyLineCount > 2) {
            maybePoem = true;
        }

        if (m_options.poemStartPattern.isEmpty() && !noTitle && line.size() > 100) {
            noTitle = true;
            poem._Title = QObject::tr("New Poem %1").arg(m_catContents.poems.size() + 1);
        }

        verse._PoemID = poem._ID;
//        if (!maybePoem && !maybeParagraph && !(m_options.contentTypes & Options::Poem) && ((line.startsWith(QChar(' ')) && line.size() <= 70) || line.trimmed().size() < 50)) {
//            maybeSingle = true;
//        }
        if (!(m_options.contentTypes &Options::Poem) && (line.startsWith(QLatin1String("  ")) && line.size() <= 70)) {
            maybeSingle = true;

            if (!verse._Text.isEmpty()) {
                verse._Order = vorder;
                verses.append(verse);
                ++vorder;
                verse._Text.clear();
            }
        }

        if (maybeSingle || (line.size() <= 70 && m_options.contentTypes &Options::WhitePoem) || justWhitePoem) {
            verse._Order = vorder;
            verse._Text = line;
            verse._Position = Single;
            ++vorder;
            verses.append(verse);
            verse._Text.clear();
        }
        else if ((line.size() <= 70 && m_options.contentTypes &Options::Poem) || justClassicalPoem) {
            verse._Order = vorder;
            verse._Text = line.trimmed();
            verse._Position = VersePosition(vorder % 2);
            ++vorder;
            verses.append(verse);
            verse._Text.clear();
        }
        else if (justNormalText || maybeParagraph || maybePoem) {
            QString prefix = verse._Text.isEmpty() ? "" : " ";
            verse._Text += prefix + line.trimmed();
            verse._Position = Paragraph;
            maybeParagraph = false;
        }
        else {
            QString prefix = verse._Text.isEmpty() ? "" : " ";
            verse._Text += prefix + line.trimmed();
            verse._Position = Paragraph;
            maybeParagraph = false;
        }

        maybeSingle = false;

        emptyLineCount = 0;
    }

    if (!verse._Text.isEmpty()) {
        verse._Order = vorder;
        verses.append(verse);
    }

    if (!poem.isNull()) {
        m_catContents.verses.insert(poem._ID, verses);
        m_catContents.poems.append(poem);
    }

    if (!cat.isNull() && !m_catContents.cats.contains(cat._ID)) {
        qDebug() << "\n----- 9 ------\n"
                 << cat._Text << "\n"
                 << cat._ID << "\n"
                 << cat._ParentID << "\n"
                 << poem._Title << "\n"
                 << poem._ID << "\n"
                 << poem._CatID << "\n"
                 << "\n----- 10 -----\n";
        m_catContents.cats.insert(cat._ID, cat);
    }

    setState(Success);
}

CatContents TxtImporter::importData() const
{
    if (state() != Success) {
        return CatContents();
    }

    return m_catContents;
}
