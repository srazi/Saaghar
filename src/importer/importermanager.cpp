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
#include "importer_interface.h"
#include "txtimporter.h"

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
