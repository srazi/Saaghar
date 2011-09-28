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

#include "SearchPatternManager.h"
#include "SearchItemDelegate.h"
#include "QGanjoorDbStuff.h"
#include "SaagharWidget.h"
#include <QtGui>
#include <QIcon>
#define QStyledItemDelegate QItemDelegate
SaagharItemDelegate::SaagharItemDelegate(QWidget *parent, QStyle *style, QString phrase) : QStyledItemDelegate(parent)
{
	keywordList.clear();
	keywordList = SearchPatternManager::phraseToList(phrase);
	keywordList.removeDuplicates();

	tableStyle = style;
	if (parent->objectName()=="searchTable")
		qDebug() << keywordList;
}

void SaagharItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
						 const QModelIndex &index) const
{
//	if (parent()->objectName()=="searchTable")
//	{
//		qDebug() << keywordList;
//	}
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
	QString cleanedText = "";
	int lastX,x;
	const QFontMetrics fontMetric(option.fontMetrics);
	const QString tatweel = QString(0x0640);

	int iconWidth = 0;

	if (index.data().isValid())
	{
		text = index.data().toString();

		if (index.data(Qt::DecorationRole).isValid())
		{
			QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
			iconWidth = icon.pixmap(100, 100).width()+5;
		}

		cleanedText = QGanjoorDbBrowser::cleanString(text/*, SaagharWidget::newSearchSkipNonAlphabet*/);
//		if (text.contains( QString::fromLocal8Bit(
//							  "پسر چون ز مادر بران گونه زاد"
////			                      "پرپر شد ..."
//					/*"وَز ســوری و نــعــمـان وزد، هـر دم شـمـیـم عـنـبـریـن"*/)))
//		{
//			qDebug() << "textt="<<text;
//			qDebug() << "clean="<<cleanedText;
//		}
		text = QGanjoorDbBrowser::cleanString(text/*, false*/);
	}
	//qDebug() << "text="<<text<<"cleanedText="<<cleanedText;
	Qt::Alignment itemAlignment = 0;
	QVariant alignValue = index.data(Qt::TextAlignmentRole);
	
	if (alignValue.isValid() && !alignValue.isNull())
		itemAlignment = Qt::Alignment(alignValue.toInt());

	int keywordsCount = keywordList.size();
	for (int i=0; i<keywordsCount; ++i)
	{
		lastX = x = option.rect.x()+textHMargin;
		//QString keyword = keywordList.isEmpty() ? "" : keywordList.at(0);
		QString keyword = keywordList.at(i);

		keyword.replace(QChar(0x200C), "", Qt::CaseInsensitive);//replace ZWNJ by ""
//		qDebug() << "keyword1="<<keyword;
		keyword = keyword.split("", QString::SkipEmptyParts).join(tatweel+"*");
//		qDebug() << "keyword2="<<keyword;
		QRegExp maybeTatweel(keyword, Qt::CaseInsensitive);
		maybeTatweel.indexIn(text);
//		qDebug() << text<<"count=" << anySearch.captureCount()<<anySearch.capturedTexts();
//		qDebug() << "Match: "<<anySearch.cap(0);
		keyword = maybeTatweel.cap(0);
	if (!(keyword.isEmpty() || text.indexOf(keyword) == -1 ) )
	{
		QString txt = text;
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
				rectf.adjust(-iconWidth, 0, -iconWidth, 0);
				painter->fillRect( rectf, itemBrush );
				painter->setOpacity(oldOpacity);
				//painter->fillRect( rectf, itemBrush );
			}
			x += option.fontMetrics.width(thisPart);
			lastX = x;
		}
	}
	else if (!(keyword.isEmpty() || cleanedText.indexOf(keyword) == -1 ) )
	{
		qreal oldOpacity = painter->opacity();
		painter->setOpacity(0.35);
		painter->fillRect( option.rect, itemBrush );
		painter->setOpacity(oldOpacity);
		//painter->fillRect( rectf, itemBrush );
	}
	}
	QStyledItemDelegate::paint(painter, option, index);
}

void SaagharItemDelegate::keywordChanged(const QString &text)
{
	keywordList.clear();
	QString tmp = text;
	tmp.replace(QChar(0x200C), "", Qt::CaseInsensitive);//replace ZWNJ by ""
	keywordList = SearchPatternManager::phraseToList(tmp);
	keywordList.removeDuplicates();

	QTableWidget *table=qobject_cast<QTableWidget *>(parent());
	if (table)
	{
		table->viewport()->update();
	}
}
