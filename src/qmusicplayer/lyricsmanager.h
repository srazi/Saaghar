/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2014 by S. Razi Alavizadeh                               *
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

#ifndef LYRICSMANAGER_H
#define LYRICSMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QMap>

class LyricsManager : public QObject
{
    Q_OBJECT

public:
    LyricsManager(QObject* parent = 0);

    bool read(QIODevice* device, const QString &format);
    int vorderByTime(qint64 time);

private:
    QDomDocument m_domDocument;
    QMap<qint64, int> m_syncMap;
};

#endif // LYRICSMANAGER_H
