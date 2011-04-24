/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>       *
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

#include <QTranslator>
#include <QtGui/QApplication>
#include "mainwindow.h"

#ifdef Q_WS_WIN
#ifdef STATIC
	#include <QtPlugin>
	Q_IMPORT_PLUGIN(qsqlite)
#endif
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTranslator* appTranslator=new QTranslator();
	QTranslator* basicTranslator=new QTranslator();

	QStringList translationDirs = QStringList() << "/usr/share/saaghar" << QCoreApplication::applicationDirPath() + "/../Resources" << QCoreApplication::applicationDirPath();

	for (int i=0; i<translationDirs.size(); ++i)
	{
		QString translationDir = translationDirs.at(i);
		if (appTranslator->load(QString("saaghar_fa"), translationDir))
		{
			QCoreApplication::installTranslator(appTranslator);
			if (basicTranslator->load(QString("qt_fa"), translationDir))
				QCoreApplication::installTranslator(basicTranslator);
			break;
		}
	}

	MainWindow w;
	w.show();
	
	w.emitReSizeEvent();//maybe a Qt BUG//before 'QMainWindow::show()' the computation of width of QMainWindow is not correct!

	return a.exec();
}
