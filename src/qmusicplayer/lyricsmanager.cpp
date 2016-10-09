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

#include <QDebug>

#include "lyricsmanager.h"

const int ID_DATA = Qt::UserRole + 1;

LyricsManager::LyricsManager(QObject* parent)
    : QObject(parent),
      m_scaleFactor(1)
{
}

bool LyricsManager::read(QIODevice* device, const QString &format)
{
    if (format != "GANJOOR_XML") {
        return false;
    }

    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!m_domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn)) {
        qDebug() << QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
        return false;
    }

    QDomElement root = m_domDocument.documentElement();
    if (root.tagName() != "DesktopGanjoorPoemAudioList") {
        qDebug() << "The file is not an DesktopGanjoorPoemAudioList file.";
        return false;
    }

    m_syncMap.clear();
    QDomNodeList syncInfoList = root.elementsByTagName("SyncInfo");

    for (int i = 0; i < syncInfoList.count(); ++i) {
        QDomElement e = syncInfoList.at(i).toElement();
        if (!e.isNull()) {
            bool ok;
            qint64 time = e.firstChildElement("AudioMiliseconds").text().toLongLong(&ok);
            int vorder = e.firstChildElement("VerseOrder").text().toInt(&ok);
            if (vorder >= 0) {
                ++vorder;
            }
            if (ok) {
                m_syncMap.insert(time, vorder);
            }
        }
    }

    m_domDocument.clear();

    return !m_syncMap.isEmpty();
}

int LyricsManager::vorderByTime(qint64 time)
{
    if (m_syncMap.isEmpty() || time < 0) {
        return -3;
    }

    QMap<qint64, int>::const_iterator it = m_syncMap.constBegin();
    qint64 key = time * m_scaleFactor;
    while (it != m_syncMap.constEnd()) {
        if (it.key() >= time * m_scaleFactor) {
            break;
        }
        key = it.key();
        ++it;
    }

    return m_syncMap.value(key, -3);
}

int LyricsManager::setScaleFactor(qint64 totalDuration)
{
    if (totalDuration <= 0 || m_syncMap.isEmpty()) {
        m_scaleFactor = 1;
        return -1;
    }

    if (totalDuration < m_syncMap.keys().last()) {
        m_scaleFactor = 2;
    }
    else {
        m_scaleFactor = 1;
    }

    return m_scaleFactor;
}
