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
        return "<EMPTY PREVIEW>";
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
            content += QString("<br><center><h2><b>%1</b></h2><br><h4>Poem ID: %2</h4><br><br><h4>Poem Verse Count: %3</h4><br>--------------------------------<br></center>")//--------------------------------<br></center>")
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
            //content += "<br><center>==================================================</center><br><br><hr>";
            content += "<hr>";
        }

        content += "</body></html>";
    }
    else if (type == EditingText) {
        return "Not implemented!";
    }

    return content;
}

bool ImporterManager::initializeImport()//const QString &fileName)
{

    //ImporterInterface* fileImporter = importer(suffix);

    ImporterOptionsDialog* optionDialog = new ImporterOptionsDialog();//(fileImporter, fileInfo.canonicalFilePath(), content);

    return optionDialog->exec() == QDialog::Accepted;
//    if (optionDialog->exec() != QDialog::Accepted) {
//        return false;
//    }

//    fileImporter->import(content);
}
