/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
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

#include "ui_settings.h"
#include "QMultiSelectWidget.h"

#include <QDialog>

namespace Ui {
	class Settings;
}

class Settings : public QDialog
{
	Q_OBJECT

public:
	explicit Settings(QWidget *parent = 0);
	~Settings();
	Ui::Settings *ui;
	void initializeActionTables(const QMap<QString, QAction *> &actionsMap, const QStringList &toolBarItems);

private slots:
	void removeActionFromToolbarTable();
	void addActionToToolbarTable();
	void topAction();
	void bottomAction();
	void tableAllActionsEntered();
	void tableToolBarActionsEntered();
	void browseForBackground();
	void browseForIconTheme();
	void browseForDataBasePath();
	void getColorForPushButton();

private:
	void replaceWithNeighbor(int neighbor);
};

class CustomizeRandomDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CustomizeRandomDialog(QWidget *parent = 0, bool checked = false);
	~CustomizeRandomDialog();
	void acceptSettings(bool *checked);
	QMultiSelectWidget *selectRandomRange;

private:
	QLabel *label;
	QGridLayout *gridLayout;
	QVBoxLayout *verticalLayout;
	QCheckBox *checkBox;
	QHBoxLayout *horizontalLayout;
	QSpacerItem *horizontalSpacer;
	QPushButton *pushButtonOk;
	QPushButton *pushButtonCancel;
	void setupui();
	void retranslateUi();
};

#endif // SETTINGS_H
