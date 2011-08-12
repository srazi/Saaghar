/***************************************************************************
 *  This file is part of QSearchLineEdit, a custom Qt widget               *
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

#ifndef QSEARCHLINEEDIT_H
#define QSEARCHLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class QSearchLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	QSearchLineEdit(QWidget *parent = 0, const QString &clearIconFileName = "", const QString &optionsIconFileName = "");
	QToolButton *optionsButton();
	

protected:
	void resizeEvent(QResizeEvent *);

private slots:
	void updateCloseButton(const QString &text);
	void updateStyleSheet(const QString &text);

private:
	void moveToRight(QToolButton *button);
	void moveToLeft(QToolButton *button);
	QToolButton *clearButton;
	QToolButton *_optionsButton;

signals:
	void clearButtonPressed();
};

#endif // QSEARCHLINEEDIT_H
