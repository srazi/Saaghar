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

#include <QFile>
#include <QFileInfo>
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

    Q_ASSERT(importer);

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
    if (type != PlainText) {
        qWarning() << "Not implemented!";
    }

    if (importData.isNull()) {
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

    return content;
}

bool ImporterManager::initializeImport(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    QFileInfo fileInfo(fileName);

    if (!file.open(QFile::ReadOnly)) {
        return false;
    }

    const QString content = QString::fromUtf8(file.readAll());
    const QString suffix = fileInfo.suffix().toLower();

    ImporterInterface* fileImporter = importer(suffix);

    ImporterOptionsDialog* optionDialog = new ImporterOptionsDialog(fileImporter, fileInfo.canonicalFilePath(), content);

    return optionDialog->exec() == QDialog::Accepted;
//    if (optionDialog->exec() != QDialog::Accepted) {
//        return false;
//    }

//    fileImporter->import(content);
}
