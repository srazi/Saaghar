/***************************************************************************
 *  This file is part of QSearchLineEdit, a custom Qt widget               *
 *                                                                         *
 *  Copyright (C) 2012 by S. Razi Alavizadeh                               *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
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

//embeded progress-bar
#include <QProgressBar>
#include <QToolTip>

class QSearchLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	QSearchLineEdit(QWidget *parent = 0,
					const QString &clearIconFileName = "",
					const QString &optionsIconFileName = "",
					const QString &cancelIconFileName = "");
	QToolButton *optionsButton();
	void notFound();
	

protected:
	void resizeEvent(QResizeEvent *);

private slots:
	void updateCloseButton(const QString &text);
	void resetNotFound();

private:
	void moveToRight(QToolButton *button);
	void moveToLeft(QToolButton *button);
	QToolButton *clearButton;
	QToolButton *optionButton;
	bool maybeFound;

signals:
	void clearButtonPressed();

//embeded progress-bar
public:
	bool searchWasCanceled();
	QProgressBar *searchProgressBar();

public slots:
	void searchStart(bool *canceled = 0, int min = 0, int max = 0);
	void searchStop();
	void setSearchProgressText(const QString &str);

private:
	QProgressBar *sPbar;
	QToolButton *stopButton;
	bool searchStarted;
	bool *cancelPointer;
	QString cancelButtonIcon;

signals:
	void searchCanceled();
};

#endif // QSEARCHLINEEDIT_H
