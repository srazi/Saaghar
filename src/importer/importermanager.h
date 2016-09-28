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
#include <QPointer>

#include "importer_interface.h"

class QLabel;
class QTreeWidget;

class ImporterManager : public QObject
{
    Q_OBJECT

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
    void storeAsDataset(const CatContents &importData, bool storeAsGDB = false);
    QString convertTo(const CatContents &importData, ConvertType type) const;
    QString convertToSED(const CatContents &importData) const;

    bool initializeImport();

private slots:
    void importPathChanged();
    void clearImportPath();
    void importHere();

signals:
    void importDialogDone();
private:
    Q_DISABLE_COPY(ImporterManager)
    ImporterManager();
    QMap<QString, ImporterInterface*> m_registeredImporters;

    static ImporterManager* s_importerManager;

    bool m_forceCreateNew;
    QList<int> m_importPath;
    QPointer<QLabel> m_importPathLabel;
    QPointer<QTreeWidget> m_importPathView;
    CatContents m_importData;
};

#endif // IMPORTERMANAGER_H
