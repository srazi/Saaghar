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

#ifndef SAAGHARAPPLICATION_H
#define SAAGHARAPPLICATION_H

#define sApp SaagharApplication::instance()

#include "progressmanager_p.h"
#include "progressmanager.h"

#include <QApplication>

class SaagharWindow;

class SaagharApplication : public QApplication
{
public:
    SaagharApplication(int &argc, char **argv);
    ~SaagharApplication();

    static SaagharApplication* instance();

    Core::ProgressManager* progressManager();

private:
    void init();

    SaagharWindow* m_mainWindow;

    Core::Internal::ProgressManagerPrivate* m_progressManager;
};

#endif // SAAGHARAPPLICATION_H
