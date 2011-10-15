/***************************************************************************
 *  This file is part of QExtendedSplashScreen, a custom Qt widget         *
 *                                                                         *
 *  Copyright (C) 2011 by S. Razi Alavizadeh                               *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
 *                                                                         *
 *  The initial idea is from a custom lineEdit from qt labs.               *
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

#include "QExtendedSplashScreen.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QDesktopWidget>
#include<QPainter>
#include<QTextDocument>
#include<QTextCursor>
#include<QTextBlock>
#include<QAbstractTextDocumentLayout>

//#include <QDebug>

QExtendedSplashScreen::QExtendedSplashScreen(const QPixmap &pixmap, Qt::WindowFlags f)
	: QWidget(0, Qt::FramelessWindowHint | f)
{
	init(pixmap);
}

QExtendedSplashScreen::QExtendedSplashScreen(QWidget *parent, const QPixmap &pixmap, Qt::WindowFlags f)
	: QWidget(parent, Qt::FramelessWindowHint | f)
{
	init(pixmap);
}

QExtendedSplashScreen::~QExtendedSplashScreen()
{
}

void QExtendedSplashScreen::init(const QPixmap &pixmap)
{
	setAttribute(Qt::WA_TranslucentBackground);
	mainLayout = new QVBoxLayout;
	mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->setSpacing(0);
	pixmapLabel = new QLabel(this);

	mainLayout->addWidget(pixmapLabel);
	this->setLayout(mainLayout);

	showMessage("", int(Qt::AlignLeft), Qt::black);
	setPixmap(pixmap);
}

void QExtendedSplashScreen::finish(QWidget *mainWin)
{
	if (mainWin)
	{
#if defined(Q_WS_X11)
		extern void qt_x11_wait_for_window_manager(QWidget *mainWin);
		qt_x11_wait_for_window_manager(mainWin);
#endif
	}
	close();
}

void QExtendedSplashScreen::repaint()
{
	drawContents();
	QWidget::repaint();
	QApplication::flush();
}

void QExtendedSplashScreen::forceRepaint()
{
	repaint();
	QCoreApplication::processEvents();
}

void QExtendedSplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
	textColor = color;
	textAlign = alignment;
	showMessage(message);
}

void QExtendedSplashScreen::showMessage(const QString &message)
{
	if (message.startsWith("!QTransparentSplashInternalCommands:"))
	{
		QString command = message;
		command.remove("!QTransparentSplashInternalCommands:");
		if (command == "CLOSE")
			close();
		else if (command == "HIDE")
			hide();
		else if (command == "SHOW")
			show();

		return;
	}
	currentStatus = message;
	forceRepaint();
	emit messageChanged(message);
}

void QExtendedSplashScreen::clearMessage()
{
	showMessage("");
}

void QExtendedSplashScreen::mouseDoubleClickEvent(QMouseEvent *)
{
	close();
}

void QExtendedSplashScreen::drawContents()
{
	if (!splashPixmap.isNull())
	{
		QPixmap textPix = splashPixmap;
		QPainter painter(&textPix);
		painter.initFrom(this);
		drawContents(&painter);
		pixmapLabel->setPixmap(textPix);
	}
}

void QExtendedSplashScreen::drawContents(QPainter *painter)
{
	painter->setPen(textColor);

	if (Qt::mightBeRichText(currentStatus))
	{
		QTextDocument doc;
#ifdef QT_NO_TEXTHTMLPARSER
		doc.setPlainText(currentStatus);
#else
		//this does not override any existence of color formatting
		doc.setHtml("<span style=\"color: "+textColor.name()+";\">"+currentStatus+"</span>");
#endif
		doc.setTextWidth(textRect.width());
		QTextCursor cursor(&doc);
		cursor.select(QTextCursor::Document);
		QTextBlockFormat fmt;
		fmt.setAlignment(Qt::Alignment(textAlign));
		cursor.mergeBlockFormat(fmt);
		int docHeight = doc.blockCount()*doc.documentLayout()->blockBoundingRect(doc.begin()).height();

		painter->save();

		switch (Qt::Alignment(textAlign) & Qt::AlignVertical_Mask )
		{
			case Qt::AlignTop:
					painter->translate(textRect.topLeft());
				break;

			case Qt::AlignVCenter:
					painter->translate(QPoint(textRect.left(), textRect.top()+qMax( (textRect.height()-docHeight)/2-5, 0) ) );
				break;

			case Qt::AlignBottom:
					painter->translate(QPoint(textRect.left(), textRect.top()+qMax(textRect.height()-docHeight-5, 0) ) );
				break;

			default:
				break;
		}

		doc.drawContents(painter);
		painter->restore();
	}
	else
	{
		painter->drawText(textRect, textAlign, currentStatus);
	}
}

void QExtendedSplashScreen::setMessageOptions(QRect rect, int alignment, const QColor &color)
{
	textRect = rect;
	textAlign = alignment;
	textColor = color;
	forceRepaint();
}

const QPixmap QExtendedSplashScreen::pixmap() const
{
	return splashPixmap;
}

void QExtendedSplashScreen::setPixmap(const QPixmap &pixmap)
{
	splashPixmap = pixmap;
	textRect = splashPixmap.rect();
	QRect r(0, 0, splashPixmap.size().width(), splashPixmap.size().height());
	move(QApplication::desktop()->screenGeometry().center() - r.center());
	pixmapLabel->setPixmap(splashPixmap);
	forceRepaint();
}
