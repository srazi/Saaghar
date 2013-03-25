/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2013 by S. Razi Alavizadeh                          *
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

#include <QtGui/QApplication>
//#include<QMessageBox>

#include <QExtendedSplashScreen>
#include "mainwindow.h"

#ifdef Q_WS_WIN
#ifdef STATIC
#include <QtPlugin>
Q_IMPORT_PLUGIN(qsqlite)
#endif
#endif

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    //'At Development Stage' message
    //QMessageBox::information(0, QObject::tr("At Development Stage"), QObject::tr("This is an experimental version! Don\'t release it!\nWWW: http://saaghar.pozh.org"));

    QPixmap pixmap(":/resources/images/saaghar-splash.png");
    QExtendedSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.setMessageOptions(QRect(pixmap.rect().topLeft() + QPoint(30, 400), pixmap.rect().bottomRight() + QPoint(-300, 0)), Qt::AlignLeft | Qt::AlignBottom, Qt::blue);

    MainWindow w(0, &splash);
    w.show();

    splash.finish(&w);

    w.checkRegistration();
//  w.emitReSizeEvent();//maybe a Qt BUG//before 'QMainWindow::show()' the computation of width of QMainWindow is not correct!

    return a.exec();
}
