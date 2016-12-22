/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2016 by S. Razi Alavizadeh                          *
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

#ifndef VERSION_H
#define VERSION_H

#define MAJOR_VERSION               3
#define MINOR_VERSION               0
#define PATCH_VERSION               0

const QString BUILD_TIME = __DATE__ " " __TIME__;
const QString SAAGHAR_VERSION = QString("%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION);

#define CONCAT(...)                 __VA_ARGS__
#define STR(s)                      #s
#define TO_STR(w, x, y, z)          STR(w) "." STR(x) "." STR(y) "." STR(z) "\0"
#define TO_STR_S(w, x)              STR(w) "." STR(x) "\0"

#define VER_FILEVERSION             CONCAT(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, 0)
#define VER_FILEVERSION_STR         TO_STR(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, 0)

#define VER_FILEBUILDTIME_STR       (BUILD_TIME + "\0")

#define VER_PRODUCTVERSION          CONCAT(MAJOR_VERSION, MINOR_VERSION)
#define VER_PRODUCTVERSION_STR      TO_STR_S(MAJOR_VERSION, MINOR_VERSION)\

#define VER_COMPANYNAME_STR         "Pozh"
#define VER_FILEDESCRIPTION_STR     "Saaghar, a Persian Poetry Software"
#define VER_INTERNALNAME_STR        "Saaghar"
#define VER_LEGALCOPYRIGHT_STR      "Copyright (c) 2010-2017, S. Razi Alavizadeh"
#define VER_LEGALTRADEMARKS1_STR    "This program is licensed under GPL v.3"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "Saaghar.exe"
#define VER_PRODUCTNAME_STR         "Saaghar"

#define VER_COMPANYDOMAIN_STR       "Pozh.org"

#endif // VERSION_H
