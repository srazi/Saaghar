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
#include "importermanager.h"
#include "txtimporter.h"
#include "importeroptionsdialog.h"

#include <QDebug>
#include <QApplication>

ImporterManager* ImporterManager::s_importerManager = 0;

ImporterManager::ImporterManager()
{
    registerImporter("txt", new TxtImporter);
}

ImporterManager* ImporterManager::instance()
{
    if (!s_importerManager) {
        s_importerManager = new ImporterManager;
    }

    return s_importerManager;
}

ImporterManager::~ImporterManager()
{
    qDeleteAll(m_registeredImporters);

    delete s_importerManager;
    s_importerManager = 0;
}

ImporterInterface* ImporterManager::importer(const QString &id)
{
    ImporterInterface* importer = m_registeredImporters.value(id, 0);

    return importer;
}

bool ImporterManager::importerIsAvailable()
{
    return !m_registeredImporters.isEmpty();
}

bool ImporterManager::registerImporter(const QString &id, ImporterInterface *importer)
{
    if (!m_registeredImporters.contains(id)) {
        m_registeredImporters.insert(id, importer);

        return true;
    }

    return false;
}

void ImporterManager::unRegisterImporter(const QString &id)
{
    m_registeredImporters.take(id);
}

QStringList ImporterManager::availableFormats()
{
    QStringList formats;
    foreach (ImporterInterface *importer, m_registeredImporters) {
        formats << QString("%1 (*.%2)").arg(importer->readableName()).arg(importer->suffix());
    }

    return formats;
}

QString ImporterManager::convertTo(const ImporterInterface::CatContents &importData, ImporterManager::ConvertType type) const
{
    if (importData.isNull()) {
        return QObject::tr("<EMPTY PREVIEW>");
    }

    QString content;

    if (type == PlainText) {
        foreach (const GanjoorPoem &poem, importData.poems) {
            QList<GanjoorVerse> verses = importData.verses.value(poem._ID);
            content += QString("Poem Title: %1\nPoem ID: %2\nPoem Verse Count: %3\n----------------\n")
                    .arg(poem._Title).arg(poem._ID).arg(verses.count());

            foreach (const GanjoorVerse &verse, verses) {
                content += QString("%1 - %2\n")
                        .arg(verse._Order).arg(verse._Text);
            }
            content += "\n=================================\n\n";
        }
    }
    else if (type == HtmlText) {
        content += "<html><body>";

        foreach (const GanjoorPoem &poem, importData.poems) {
            QList<GanjoorVerse> verses = importData.verses.value(poem._ID);
            content += QString("<br><center><h2><b>%1</b></h2><br><h4>Poem ID: %2</h4><br><br><h4>Poem Verse Count: %3</h4><br>--------------------------------<br></center>")
                    .arg(poem._Title).arg(poem._ID).arg(verses.count());

            foreach (const GanjoorVerse &verse, verses) {
                QString openTags = "<p>";
                QString closeTags = "</p>";
                if (verse._Position == VersePosition::Paragraph) {
                    openTags = "<p style=\"margin-bottom: 0.2cm\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
                    closeTags = "</p>";
                }
                else if (verse._Position == VersePosition::Single) {
                    openTags = "<p>";
                    closeTags = "</p>";
                }
                else if (verse._Position == VersePosition::CenteredVerse1) {
                    openTags = "<center><p style=\"margin-top: 0.1cm\">";
                    closeTags = "</p></center>";
                }
                else if (verse._Position == VersePosition::CenteredVerse2) {
                    openTags = "<center><p style=\"margin-bottom: 0.2cm\">";
                    closeTags = "</p></center><br>";
                }
                else if (verse._Position == VersePosition::Right) {
                    openTags = "<center><p>";
                    closeTags = "</p></center>";
                }
                else if (verse._Position == VersePosition::Left) {
                    openTags = "<center><p style=\"margin-bottom: 1cm\">";
                    closeTags = "</p></center><br>";
                }

                content += QString("%1%3%2")
                        .arg(openTags).arg(closeTags).arg(verse._Text);
            }

            content += "<hr>";
        }

        content += "</body></html>";
    }
    else if (type == EditingText) {
        content = convertToSED(importData);
    }

    return content;
}

static QString versePositionToString(VersePosition position) {
    switch (position) {
    case Right:
        return "_RIGHT_";
        break;
    case Left:
        return "_LEFT_";
        break;
    case CenteredVerse1:
        return "_CENTERED_VERSE_1_";
        break;
    case CenteredVerse2:
        return "_CENTERED_VERSE_2_";
        break;
    case Single:
        return "_SINGLE_";
        break;
    case Paragraph:
        return "_PARAGRAPH_";
        break;
    default:
        break;
    }
    return "_UNKNOWN_";
}

/*******************************************************************************/
// tags for Version-0.1 of Saaghar Easy Editable Dataset Format (SED)
// #SAAGHAR!SED!         // start of SED file
// #SED!LANGUAGE!        // SED file language
// #SED!CAT!START!       // start of cat
// #CAT!ID!              // ganjoor id of category
// #CAT!UID!             // unique id of category
// #CAT!CATID!           // ganjoor id of toplevel category of current category
// #CAT!CATUID!          // unique id of toplevel category of current category
// #CAT!TITLE!           // title of category
// #SED!POEM!START!      // start of poem
// #POEM!CATID!          // ganjoor id of toplevel category of current poem
// #POEM!CATUID!         // unique id of toplevel category of current poem
// #POEM!ID!             // ganjoor id of poem
// #POEM!UID!            // unique id of poem
// #POEM!TITLE!          // title of poem
// #POEM!VERSECOUNT!     // title of poem
// #SED!VERSE!START!     // start of verse
// #VERSE!ORDER!         // verse order within poem
// #VERSE!POSITION!      // verse type and position
// #VERSE!TEXT!          // verse text
// #SED!VERSE!END!       // end of verse
// #SED!POEM!END!        // end of poem
// #SED!CAT!END!         // end of cat
/*******************************************************************************/
QString ImporterManager::convertToSED(const ImporterInterface::CatContents &importData) const
{
    QString content;

    QString poetUID = "poetUID";
    QString catUID = "catUID";
    QString poetTitle = "poetTitle";
    QString catTitle = "catTitle";

    content += "#SAAGHAR!SED!v0.1\n#SED!LANGUAGE!FA_IR\n";
    content += QString("#SED!CAT!START!\n#CAT!UID!%1\n#CAT!CATID!0\n#CAT!TITLE!%2\n"
                       "#SED!CAT!START!\n#CAT!UID!%3\n#CAT!CATUID!%1\n#CAT!TITLE!%4\n")
            .arg(poetUID).arg(poetTitle).arg(catUID).arg(catTitle);

    content += "\n######################\n######################\n";

    foreach (const GanjoorPoem &poem, importData.poems) {
        QList<GanjoorVerse> verses = importData.verses.value(poem._ID);
        content += QString("#SED!POEM!START!\n#POEM!CATUID!%1\n#POEM!ID!%2\n#POEM!TITLE!%3\n#POEM!VERSECOUNT!%4\n")
                .arg(catUID).arg(poem._ID).arg(poem._Title).arg(verses.count());

        foreach (const GanjoorVerse &verse, verses) {
            content += QString("#SED!VERSE!START!\n#VERSE!ORDER!%1\n#VERSE!POSITION!%2\n#VERSE!TEXT!%3\n")
                    .arg(verse._Order).arg(versePositionToString(verse._Position)).arg(verse._Text);
        }
        content += "\n######################\n";
    }

    return content;
}

bool ImporterManager::initializeImport()
{
    ImporterOptionsDialog* optionDialog = new ImporterOptionsDialog(qApp->activeWindow());

    return optionDialog->exec() == QDialog::Accepted;
}
