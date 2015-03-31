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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#define LS QLatin1String

#define VAR_DECL(x, ...) SettingsManager::instance()->defineVariable(LS(x), __VA_ARGS__)
#define VAR_INIT(x, ...) SettingsManager::instance()->defineVariableInitialValue(LS(x), __VA_ARGS__)

#define VAR(x, ...) SettingsManager::instance()->variable(LS(x))
#define VARB(x, ...) SettingsManager::instance()->variable(LS(x)).toBool()
#define VARS(x, ...) SettingsManager::instance()->variable(LS(x)).toString()
#define VARI(x, ...) SettingsManager::instance()->variable(LS(x)).toInt()

#include <QHash>
#include <QVariant>

class SettingsManager : public QObject
{
public:
    static SettingsManager *instance();
    ~SettingsManager();

    void defineVariable(const QString &name, const QVariant &value = QVariant());
    void defineVariableInitialValue(const QString &name, const QVariant &value);
    QVariant variable(const QString &name) const;

    void clear();
    bool loadVariable(QIODevice *in, bool append = false);
    bool writeVariable(QIODevice *out);

private:
    Q_DISABLE_COPY(SettingsManager)
    SettingsManager(QObject *parent = 0);
    static SettingsManager *s_instance;

    QVariant variantEncode(const QVariant &v) const;
    QVariant variantDecode(const QVariant &v) const;

    mutable QHash<QString, QVariant> m_variables;
    QHash<QString, QVariant> m_variablesInitialValues;
};

#endif // SETTINGSMANAGER_H
