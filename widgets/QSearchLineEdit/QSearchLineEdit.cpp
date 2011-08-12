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
	_optionsButton = new QToolButton(this);
	_optionsButton->setIcon(QIcon(optionsPixmap));
	_optionsButton->setIconSize(optionsPixmap.size());
	_optionsButton->setCursor(Qt::ArrowCursor);
	_optionsButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

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
	//connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(updateStyleSheet(const QString&)));
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	setMinimumSize(qMax(msz.width(), _optionsButton->sizeHint().width()+clearButton->sizeHint().width() /*+ frameWidth * 2 + 2*/),
					qMax(qMax(msz.height(), _optionsButton->sizeHint().height() /*+ frameWidth * 2 + 2*/), clearButton->sizeHint().height() /*+ frameWidth * 2 + 2*/) );
	setStyleSheet(QString("QLineEdit { padding-left: %1px; padding-right: %2px; } ").arg(clearButton->sizeHint().width() /*+ frameWidth + 1*/).arg(_optionsButton->sizeHint().width() /*+ frameWidth + 1*/));
	clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");//->setStyleSheet(QString("QToolButton { border: none; padding-right: %1px; }").arg(_optionsButton->width()+this->width()-clearPixmap.width()));
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
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	button->move(rect().left()/* - frameWidth - sz.width()*/, (rect().bottom() + 1 - sz.height())/2);
}

void QSearchLineEdit::resizeEvent(QResizeEvent *)
{
	if (parentWidget())
	{
		if (parentWidget()->layoutDirection() == Qt::RightToLeft)
		{
			moveToRight(_optionsButton);
			moveToLeft(clearButton);
			return;
		}
	}
	moveToRight(clearButton);
	moveToLeft(_optionsButton);
}

void QSearchLineEdit::updateStyleSheet(const QString &text)
{
//	setStyleSheet("QLineEdit { padding-right: 0; padding-left: 0; } ");
//	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
//	QString paddingType = layoutDirection() == Qt::LeftToRight ? "padding-right" : "padding-left";
//	setStyleSheet(QString("QLineEdit { %1: %2px; } ").arg(paddingType).arg(clearButton->sizeHint().width() + frameWidth + 1));
}

void QSearchLineEdit::updateCloseButton(const QString& text)
{
	clearButton->setVisible(!text.isEmpty());
//	if (text.isEmpty())
//	{
//		setStyleSheet("QLineEdit { padding-right: 0; padding-left: 0; } ");
//	}
//	else
//	{
//		int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
//		QString paddingType = "padding-left";//layoutDirection() == Qt::LeftToRight ? "padding-right" : "padding-left";
//		setStyleSheet(QString("QLineEdit { %1: %2px; } ").arg(paddingType).arg(clearButton->sizeHint().width() + frameWidth + 1));
//	}
}

QToolButton *QSearchLineEdit::optionsButton()
{
	return _optionsButton;
}
