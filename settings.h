/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://www.pojh.co.cc>       *
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
 
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
	class Settings;
}

class Settings : public QDialog
{
	Q_OBJECT

public:
	explicit Settings(QWidget *parent = 0, bool iconThemeState=false, QString iconThemePath="", QMap<QString, QAction *> *actionsMap = 0, QStringList toolBarItems = QStringList() );
	~Settings();
	void acceptSettings(bool *iconThemeState, QString *iconThemePath, QStringList *toolbarItemsList);

private slots:
	void removeActionFromToolbarTable();
	void addActionToToolbarTable();
	void topAction();
	void bottomAction();
	void tableAllActionsEntered();
	void tableToolBarActionsEntered();
	void browseForBackground();
	void browseForIconTheme();
	void getColorForPushButton();

private:
	void replaceWithNeighbor(int neighbor);
	void initializeActionTables(QMap<QString, QAction *> *actionsMap, QStringList toolBarItems);
	Ui::Settings *ui;
};

#endif // SETTINGS_H
