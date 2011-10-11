/***************************************************************************
 *  This file is part of QTextBrowserDialog, a custom Qt widget            *
 *                                                                         *
 *  Copyright (C) 2011 by S. Razi Alavizadeh                               *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
 *                                                                         *
 *  The initial idea is from a custom lineEdit from qt labs................*
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

#ifndef QTEXTBROWSERDIALOG_H
#define QTEXTBROWSERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTextBrowser>

class QDialogButtonBox;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;


class QTextBrowserDialog : public QTextBrowser
{
	Q_OBJECT

public:
	explicit QTextBrowserDialog(QWidget *parent = 0, const QString &title = "",const QString &text = "", const QPixmap &pixmapIcon = QPixmap(), Qt::WindowFlags f = 0);
	inline int exec() {return containerDialog->exec();}
	void setIconPixmap(const QPixmap &pixmap);
	inline const QPixmap *iconPixmap()
		{return labelIcon->pixmap();}

private:
	QDialog *containerDialog;
	QGridLayout *gridLayout;
	QTextBrowser *textBrowser;
	QDialogButtonBox *buttonBox;
	QVBoxLayout *verticalLayout;
	QLabel *labelIcon;
	QSpacerItem *verticalSpacer;
	void setupui();
	void retranslateUi();
};
#endif // QTEXTBROWSERDIALOG_H
