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
 
#include "SearchItemDelegate.h"
#include "QGanjoorDbStuff.h"
#include "SaagharWidget.h"
#include <QtGui>

SaagharItemDelegate::SaagharItemDelegate(QWidget *parent, QStyle *style, QString phrase) : QStyledItemDelegate(parent)
{
	keyword = phrase;
	tableStyle = style;
}

void SaagharItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
						 const QModelIndex &index) const
{
	const bool flagRightToLeft = true;
	int textHMargin = 0;
	if (tableStyle)
	{
		textHMargin = tableStyle->pixelMetric(QStyle::PM_FocusFrameHMargin, &option) +1 ;
	}

	QBrush itemBrush = painter->brush();
	
	itemBrush.setColor(SaagharWidget::matchedTextColor);
	itemBrush.setStyle(Qt::SolidPattern);

	QString text="";
	//QString cleanedText = "";
	int lastX,x;
	lastX = x = option.rect.x()+textHMargin;
	const QFontMetrics fontMetric(option.fontMetrics);
	
	if (index.data().isValid())
	{
		text = index.data().toString();
		if (SaagharWidget::newSearchFlag)
			text = QGanjoorDbBrowser::cleanString(text, false);
			//cleanedText = QGanjoorDbBrowser::cleanString(index.data().toString(), SaagharWidget::newSearchSkipNonAlphabet);
		//text = fontMetric.elidedText(text, option.textElideMode, option.rect.width() );
	}

	Qt::Alignment itemAlignment = 0;
	QVariant alignValue = index.data(Qt::TextAlignmentRole);
	
	if (alignValue.isValid() && !alignValue.isNull())
		itemAlignment = Qt::Alignment(alignValue.toInt());

	if (!(keyword.isEmpty() || text.indexOf(keyword) == -1 ) )
	{
		QString txt;
		/*if (SaagharWidget::newSearchFlag)
			txt = cleanedText;
		else*/
			txt = text;
		while (txt.size() > 0)
		{
			int index = txt.indexOf(keyword);
			QString thisPart;
			if (index == -1)
			{
				thisPart = txt;
				txt = QString();
			}
			else
			{
				if (index == 0)
				{
					thisPart = txt.mid(0, keyword.size());
					if (txt == keyword)
						txt = QString();
					else
						txt = txt.mid(keyword.size(), txt.size() - keyword.size());
				}
				else
				{
					thisPart = txt.mid(0, index);
					txt = txt.mid(index);
				}
			}
				
			QSize sz = fontMetric.boundingRect(thisPart).size();
			if (index == 0)
			{
				if (flagRightToLeft)
				{
					switch (itemAlignment^Qt::AlignVCenter)
					{
						case Qt::AlignRight:
							lastX = option.rect.left()+textHMargin+fontMetric.boundingRect(text).width()-(lastX-option.rect.x()+sz.width()-textHMargin);
							break;

						case Qt::AlignHCenter:
							lastX = option.rect.left()+textHMargin+fontMetric.boundingRect(text).width()-(lastX-option.rect.x()+sz.width()-textHMargin)+((option.rect.width()-fontMetric.boundingRect(text).width()-textHMargin)/2);
							break;

						case Qt::AlignLeft:
						default:
							lastX = option.rect.right()+textHMargin-1 - (lastX-option.rect.x()+sz.width());
							break;
					}
				}

				QRectF rectf(lastX , option.rect.y()+((option.rect.height()-qMin(option.rect.height(), fontMetric.height()))/2), sz.width(), fontMetric.height() );
				qreal oldOpacity = painter->opacity();
				painter->setOpacity(0.65);
				painter->fillRect( rectf, itemBrush );
				painter->setOpacity(oldOpacity);
				painter->fillRect( rectf, itemBrush );
			}
			x += option.fontMetrics.width(thisPart);
			lastX = x;
		}
	}
	QStyledItemDelegate::paint(painter, option, index);
}

void SaagharItemDelegate::keywordChanged(const QString &text)
{
	keyword = text;
	//qApp->activeWindow()->repaint();
	QTableWidget *table=qobject_cast<QTableWidget *>(parent());
	if (table)
	{
		//table->viewport()->repaint();
		table->viewport()->update();
	}
}
