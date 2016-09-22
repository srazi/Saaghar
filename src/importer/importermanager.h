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
#ifndef IMPORTERMANAGER_H
#define IMPORTERMANAGER_H

#include <QString>
#include <QMap>

#include "importer_interface.h"

class ImporterManager
{
public:
    static ImporterManager* instance();
    ~ImporterManager();
    enum ConvertType {
        EditingText,
        PlainText,
        HtmlText
    };

    ImporterInterface* importer(const QString &id);

    bool importerIsAvailable();

    bool registerImporter(const QString &id, ImporterInterface* importer);
    void unRegisterImporter(const QString &id);
    QStringList availableFormats();
    QString convertTo(const ImporterInterface::CatContents &importData, ConvertType type) const;
    QString convertToSED(const ImporterInterface::CatContents &importData) const;

    bool initializeImport();

private:
    Q_DISABLE_COPY(ImporterManager)
    ImporterManager();
    QMap<QString, ImporterInterface*> m_registeredImporters;

    static ImporterManager* s_importerManager;
};

#endif // IMPORTERMANAGER_H
