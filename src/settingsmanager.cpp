/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2015 by S. Razi Alavizadeh                          *
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

#include "settingsmanager.h"

#include <QApplication>
#include <QDataStream>
#include <QDebug>

SettingsManager *SettingsManager::s_instance = 0;

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
      m_variables()
{
}

SettingsManager *SettingsManager::instance()
{
    if (!s_instance) {
        s_instance = new SettingsManager(qApp);
    }

    return s_instance;
}

SettingsManager::~SettingsManager()
{
    s_instance = 0;
}

void SettingsManager::defineVariable(const QString &name, const QVariant &value)
{
    m_variables.insert(name, value.isValid()
                       ? variantEncode(value)
                       : variantEncode(m_variablesInitialValues.value(name)));
}

void SettingsManager::defineVariableInitialValue(const QString &name, const QVariant &value)
{
    Q_ASSERT(!m_variablesInitialValues.contains(name));

    m_variablesInitialValues.insert(name, value);
}

QVariant SettingsManager::variable(const QString &name) const
{
    if (m_variables.contains(name)) {
        return variantDecode(m_variables.value(name));
    }
    else {
        const QVariant v = m_variablesInitialValues.value(name);
        m_variables.insert(name, variantEncode(v));

        return v;
    }
}

void SettingsManager::clear()
{
    m_variables.clear();
}

bool SettingsManager::loadVariable(QIODevice *in, bool append)
{
    if (!in || !in->isOpen() || !in->isReadable()) {
        qDebug() << "SettingsManager::loadVariableFromFile: Is not open or is not readable";
        return false;
    }

    QDataStream stream(in);
    stream.setVersion(QDataStream::Qt_4_6);

    QVariant v;
    stream >> v;

    if (append) {
        QHash<QString, QVariant> hash = v.toHash();

        QHash<QString, QVariant>::const_iterator i =  hash.constBegin();
        while (i != hash.constEnd()) {
            m_variables.insert(i.key(), i.value());

            ++i;
        }
    }
    else {
        m_variables = v.toHash();
    }

    return true;
}

bool SettingsManager::writeVariable(QIODevice *out)
{
    if (!out || !out->isOpen() || !out->isWritable()) {
        qDebug() << "SettingsManager::writeVariableToFile: Is not open or is not writable";
        return false;
    }

    if (!m_variables.isEmpty()) {
        QDataStream stream(out);
        stream.setVersion(QDataStream::Qt_4_6);

        stream << QVariant::fromValue(m_variables);
    }

    return true;
}

QVariant SettingsManager::variantEncode(const QVariant &v) const
{
    if (v.type() == QVariant::ByteArray) {
        QByteArray data = v.toByteArray();
        data = data.toBase64();

        return data;
    }
    else {
        return v;
    }
}

QVariant SettingsManager::variantDecode(const QVariant &v) const
{
    if (v.type() == QVariant::ByteArray) {
        QByteArray data = v.toByteArray();
        data = QByteArray::fromBase64(data);

        return data;
    }
    else {
        return v;
    }
}
