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
#include <QRegularExpression>
#include <QDebug>

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

    const bool justWhitePoem = m_options.contentTypes == Options::WhitePoem;
    const bool justNormalText = m_options.contentTypes == Options::NormalText;
    const bool justClassicalPoem = m_options.contentTypes == Options::Poem;

    const int catId = 10000;
    const int poemId = 100000;

    bool createNewPoem = true;
    bool startPassed = false;
    bool noTitle = false;
    bool maybeParagraph = false;
    bool maybeSingle = false;
    bool maybePoem = false;
    GanjoorVerse verse;
    GanjoorPoem poem;
    QList<GanjoorVerse> verses;
    int emptyLineCount = 0;
    int vorder = 0;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines.at(i);

        if (!startPassed && line.trimmed().isEmpty()) {
            continue;
        }

        startPassed = true;

        if (createNewPoem) {
        if (!line.trimmed().isEmpty() && ((maybePoem && line.trimmed().size() < 50 && m_options.poemStartPattern.isEmpty()) ||
                (!m_options.poemStartPattern.isEmpty() &&
                line.contains(QRegularExpression(m_options.poemStartPattern))))) {
            if (!poem.isNull()) {
                m_catContents.verses.insert(poem._ID, verses);
                m_catContents.poems.append(poem);
            }
            verses.clear();
            poem.setNull();
            poem._CatID = catId ;
            poem._ID = poemId + i;
            poem._Title = line.trimmed();
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
            poem._CatID = catId;
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
        if (!(m_options.contentTypes & Options::Poem) && (line.startsWith(QLatin1String("  ")) && line.size() <= 70)) {
            maybeSingle = true;

            if (!verse._Text.isEmpty()) {
                verse._Order = vorder;
                verses.append(verse);
                ++vorder;
                verse._Text.clear();
            }
        }

        if (maybeSingle || (line.size() <= 70 && m_options.contentTypes & Options::WhitePoem) || justWhitePoem) {
            verse._Order = vorder;
            verse._Text = line;
            verse._Position = VersePosition::Single;
            ++vorder;
            verses.append(verse);
            verse._Text.clear();
        }
        else if ((line.size() <= 70 && m_options.contentTypes & Options::Poem) || justClassicalPoem) {
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
            verse._Position = VersePosition::Paragraph;
            maybeParagraph = false;
        }
        else {
            QString prefix = verse._Text.isEmpty() ? "" : " ";
            verse._Text += prefix + line.trimmed();
            verse._Position = VersePosition::Paragraph;
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

    setState(Success);
}

CatContents TxtImporter::importData() const
{
    if (state() != Success) {
        return CatContents();
    }

    return m_catContents;
}
