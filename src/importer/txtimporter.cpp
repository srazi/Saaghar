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

void TxtImporter::import(const QString &data) const
{
    m_catContents.clear();

    QString rawdata = data;
    rawdata = rawdata.replace(QChar('\r\n'), QLatin1String("\n"));
    rawdata = rawdata.replace(QChar('\r'), QLatin1String("\n"));

    QStringList lines = rawdata.split("\n");
m_options.poemStartPattern = "Chapter [0-9]+"; //"  [A-Z][A-Z]+";//       +[^\w]+";
m_options.contentTypes = Options::NormalText;
    const bool justWhitePoem = m_options.contentTypes == Options::WhitePoem;
    const bool justNormalText = m_options.contentTypes == Options::NormalText;
    const bool justClassicalPoem = m_options.contentTypes == Options::Poem;
//    const bool justNormalText = m_options.contentTypes == Options::NormalText;
//    const bool justClassicalPoem = m_options.contentTypes == Options::Poem;

//    m_catContents.poet._ID = 1000000;
//    m_catContents.poet._CatID = 10000000;
//    m_catContents.poet._Name = QObject::tr("New Poet");


    const int catId = 1000000 + (qrand()/2);
    const int poemId = 10000000 + (qrand()/100);

    bool startPassed = false;
    bool createNewPoem = true;
    bool noTitle = false;
    bool maybeParagraph = false;
    bool maybePoem = false;
    GanjoorPoem poem;
    QList<GanjoorVerse> verses;
    int emptyLineCount = 0;
    int vorder = 0;
qDebug() << "\n============\nLine count: " <<  lines.size() << "\n============\n";
    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines.at(i);

        if (!startPassed && line.trimmed().isEmpty()) {
            continue;
        }

        startPassed = true;


        qDebug() << __LINE__ << "\n============\nLine: " << line << "++++++" << line.size() << "\n============\n";
        if (createNewPoem) {
            if (!poem.isNull()) {
                m_catContents.verses.insert(poem._ID, verses);
                m_catContents.poems.append(poem);
            }
            verses.clear();
            poem.setNull();
            poem._CatID = catId ;
            poem._ID = poemId + i;
            //poem._Title = QObject::tr("New Poem %1").arg(m_catContents.poems.size() + 1);
            createNewPoem = false;
            noTitle = false;
            maybePoem = false;
            maybeParagraph = false;
            vorder = 0;
            continue;
        }

        qDebug() << __LINE__ << "\n============\nLine: " << line << "++++++" << line.size() << "\n============\n";
        if (!line.trimmed().isEmpty() && line.trimmed().size() < 50 && ((maybePoem && m_options.poemStartPattern.isEmpty()) ||
                (!m_options.poemStartPattern.isEmpty() &&
                line.contains(QRegularExpression(m_options.poemStartPattern))))) {
            if (!poem.isNull()) {
                m_catContents.verses.insert(poem._ID, verses);
                m_catContents.poems.append(poem);
            }
            verses.clear();
            poem.setNull();
            poem._CatID = catId;
            poem._ID = poemId + i;
            poem._Title = line.trimmed();
            createNewPoem = false;
            noTitle = true;
            maybePoem = false;
            maybeParagraph = false;
            vorder = 0;
            continue;
        }

        qDebug() << __LINE__ << "\n============\nLine: " << line << "++++++" << line.size() << "\n============\n";
        if (line.trimmed().isEmpty()) {
            ++emptyLineCount;
//            GanjoorVerse verse;
//            verse._PoemID = poem._ID;
//            verse._Order = vorder;
//            ++vorder;

//            verses.append(verse);
            continue;
        }

        qDebug() << __LINE__ << "\n============\nLine: " << line << "++++++" << line.size() << "\n============\n";
        if (!noTitle && poem._Title.isEmpty()) {
            if (line.size() < 50) {
                poem._Title = line.trimmed();
                continue;
            }
        }

        if (emptyLineCount == 1) {
            if (!justWhitePoem && !justClassicalPoem) {
                maybeParagraph = true;
            }
            else if (justWhitePoem || justClassicalPoem) {
                maybePoem = true;
            }
        }
        else if (emptyLineCount > 2) {
            maybePoem = true;
        }

        emptyLineCount = 0;

        qDebug() << __LINE__ << "\n============\nLine: " << line << "++++++" << line.size() << "\n============\n";
        if (!noTitle && line.size() > 100) {
            noTitle = true;
            poem._Title = QObject::tr("New Poem %1").arg(m_catContents.poems.size() + 1);
        }

        GanjoorVerse verse;
        verse._PoemID = poem._ID;
        verse._Order = vorder;
        if (justWhitePoem) {
            verse._Text = line;
            verse._Position = VersePosition::Single;
        }
        else if (justClassicalPoem) {
            verse._Text = line.trimmed();
            verse._Position = VersePosition(vorder % 2);
        }
        else if (justNormalText || maybeParagraph || maybePoem) {
            verse._Text = line.trimmed();
            verse._Position = VersePosition::Paragraph;
            maybeParagraph = false;
        }
        else {
            verse._Text = "Error: " + line.trimmed();
            verse._Position = VersePosition::Paragraph;
            maybeParagraph = false;
        }
        ++vorder;

        verses.append(verse);
    }
}
//#include "databasebrowser.h"
QString TxtImporter::preview() const
{
    if (m_catContents.isNull()) {
        return "EMPTY PREVIEW";
    }

//    QList<GanjoorPoem*> ppoems = DatabaseBrowser::instance()->getPoems(24);
//    QString content;
//    int count = 0;
//    foreach (GanjoorPoem *poem, ppoems) {
//        ++count;
//        QList<GanjoorVerse*> pverses = DatabaseBrowser::instance()->getVerses(poem->_ID);
//        content += QString("Poem Title: %1\nPoem ID: %2\nPoem Verse Count: %3\n----------------\n")
//                .arg(poem->_Title).arg(poem->_ID).arg(pverses.count());

//        foreach (GanjoorVerse *verse, pverses) {
//            content += QString("%1 - %2\n")
//                    .arg(verse->_Order + 1).arg(verse->_Text);
//        }
//        content += "\n=================================\n\n";
//        if (count > 4) {
//            break;
//        }
//    }

    QString content;
    foreach (const GanjoorPoem &poem, m_catContents.poems) {
        QList<GanjoorVerse> verses = m_catContents.verses.value(poem._ID);
        content += QString("Poem Title: %1\nPoem ID: %2\nPoem Verse Count: %3\n----------------\n")
                .arg(poem._Title).arg(poem._ID).arg(verses.count());

        foreach (const GanjoorVerse &verse, verses) {
            content += QString("%1 - %2\n")
                    .arg(verse._Order).arg(verse._Text);
        }
        content += "\n=================================\n\n";
    }

    return content;
}

ImporterInterface::CatContents TxtImporter::importData() const
{
    if (state() != Success) {
        m_catContents.clear();
    }

    return m_catContents;
}
