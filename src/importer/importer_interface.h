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
#ifndef IMPORTER_INTERFACE_H
#define IMPORTER_INTERFACE_H

#include "databaseelements.h"

#include <QMap>
#include <QByteArray>

class ImporterInterface
{
public:
    enum State {
        Success,
        Faild,
        Unknown = -1
    };

    struct Options {
        enum ContentType {
            Unknown = 0x0,
            NormalText = 0x1,
            WhitePoem = 0x2,
            Poem = 0x4
        };
        Q_DECLARE_FLAGS(ContentTypes, ContentType)

        ContentTypes contentTypes;
        QString poemStartPattern;

        Options() : contentTypes(Unknown) {}
        bool isNull() { return contentTypes == Unknown && poemStartPattern.isEmpty(); }
        void clear() { contentTypes = Unknown; poemStartPattern.clear(); }
    };

    struct CatContents {
        QList<GanjoorPoem> poems;
        QMap<int, QList<GanjoorVerse> > verses;

        CatContents() {}
        bool isNull() const { return poems.isEmpty() || verses.isEmpty(); }
        void clear() { poems.clear(); verses.clear(); }
    };

    ImporterInterface() : m_state(Unknown) {}
    virtual ~ImporterInterface() {}

    virtual QString readableName() const = 0;
    virtual QString suffix() const = 0;
    virtual void import(const QString &data) = 0;
    virtual CatContents importData() const = 0;

    State state() const { return m_state; }
    void setState(State state) { m_state = state; }

    Options options() const { return m_options; }
    void setOptions(const Options &options) { m_options = options; }

private:
    State m_state;

protected:
    CatContents m_catContents;
    Options m_options;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(ImporterInterface::Options::ContentTypes)
#endif // IMPORTER_INTERFACE_H
