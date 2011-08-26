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

#include "QSearchLineEdit.h"
#include <QToolButton>
#include <QStyle>

QSearchLineEdit::QSearchLineEdit(QWidget *parent, const QString &clearIconFileName, const QString &optionsIconFileName)
	: QLineEdit(parent)
{
	QSize msz = minimumSizeHint();

	QPixmap optionsPixmap(optionsIconFileName);
	optionsPixmap = optionsPixmap.scaledToHeight(fontMetrics().height() , Qt::SmoothTransformation);
	optionButton = new QToolButton(this);
	optionButton->setIcon(QIcon(optionsPixmap));
	optionButton->setIconSize(optionsPixmap.size());
	optionButton->setCursor(Qt::ArrowCursor);
	optionButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

	clearButton = new QToolButton(this);
	QPixmap clearPixmap(clearIconFileName);
	clearPixmap = clearPixmap.scaledToHeight(fontMetrics().height() /*msz.height()*/, Qt::SmoothTransformation);
	clearButton->setIcon(QIcon(clearPixmap));
	clearButton->setIconSize(clearPixmap.size());
	clearButton->setCursor(Qt::ArrowCursor);
	clearButton->hide();



	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(clearButton, SIGNAL(clicked()), this, SIGNAL(clearButtonPressed()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));

	setMinimumSize(qMax(msz.width(), optionButton->sizeHint().width()+clearButton->sizeHint().width() ),
					qMax(qMax(msz.height(), optionButton->sizeHint().height() ), clearButton->sizeHint().height() ) );
	setStyleSheet(QString("QLineEdit { padding-left: %1px; padding-right: %2px; } ").arg(clearButton->sizeHint().width() ).arg(optionButton->sizeHint().width() ));
	clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
}

void QSearchLineEdit::moveToRight(QToolButton *button)
{
	QSize sz = button->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	button->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height())/2);
}

void QSearchLineEdit::moveToLeft(QToolButton *button)
{
	QSize sz = button->sizeHint();
	button->move(rect().left(), (rect().bottom() + 1 - sz.height())/2);
}

void QSearchLineEdit::resizeEvent(QResizeEvent *)
{
	if (parentWidget())
	{
		if (parentWidget()->layoutDirection() == Qt::RightToLeft)
		{
			moveToRight(optionButton);
			moveToLeft(clearButton);
			return;
		}
	}
	moveToRight(clearButton);
	moveToLeft(optionButton);
}

void QSearchLineEdit::updateCloseButton(const QString& text)
{
	clearButton->setVisible(!text.isEmpty());
}

QToolButton *QSearchLineEdit::optionsButton()
{
	return optionButton;
}
