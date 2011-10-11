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

#include "QTextBrowserDialog.h"

#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QApplication>

QTextBrowserDialog::QTextBrowserDialog(QWidget *parent, const QString &title,const QString &text, const QPixmap &pixmapIcon, Qt::WindowFlags f)
	: QTextBrowser()
{
	containerDialog = new QDialog(parent, f);
	containerDialog->setObjectName(QString::fromUtf8("containerDialog"));

	QPalette p(containerDialog->palette());
	p.setColor(QPalette::Window, palette().color(QPalette::Base));
	containerDialog->setPalette(p);

	this->setFrameStyle(QFrame::NoFrame);

	this->setParent(containerDialog);
	setupui();

	this->setText(text);
	this->setIconPixmap(pixmapIcon);
	containerDialog->setWindowTitle(title);
}

void QTextBrowserDialog::setIconPixmap(const QPixmap &pixmap)
{
	if (pixmap.isNull())
	{
		labelIcon->hide();
		verticalLayout->removeItem(verticalSpacer);
	}
	else
	{
		verticalLayout->addItem(verticalSpacer);
		labelIcon->setPixmap(pixmap);
		labelIcon->show();
	}
}

void QTextBrowserDialog::setupui()
{
	if (this->objectName().isEmpty())
		this->setObjectName(QString::fromUtf8("QTextBrowserDialog"));
	containerDialog->resize(600, 300);
	gridLayout = new QGridLayout(containerDialog);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->setSpacing(2);

	gridLayout->addWidget(this, 0, 1, 1, 1);

	buttonBox = new QDialogButtonBox(containerDialog);
	buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Ok);
	buttonBox->setCenterButtons(true);

	gridLayout->addWidget(buttonBox, 1, 0, 1, 2);

	verticalLayout = new QVBoxLayout();
	verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	verticalLayout->setContentsMargins(0, 0, 0, 0);
	verticalLayout->setSpacing(0);

	labelIcon = new QLabel(containerDialog);
	labelIcon->setObjectName(QString::fromUtf8("labelIcon"));
	labelIcon->setText(QString());
	labelIcon->hide();

	verticalLayout->addWidget(labelIcon);

	verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

	gridLayout->addLayout(verticalLayout, 0, 0, 2, 1);

	retranslateUi();
	QObject::connect(buttonBox, SIGNAL(accepted()), containerDialog, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), containerDialog, SLOT(reject()));

	QMetaObject::connectSlotsByName(containerDialog);
}

void QTextBrowserDialog::retranslateUi()
{
	containerDialog->setWindowTitle(QApplication::translate("QTextBrowserDialog", "TextBrowser Dialog", 0, QApplication::UnicodeUTF8));
	this->setHtml(QApplication::translate("QTextBrowserDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
	"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
	"p, li { white-space: pre-wrap; }\n"
	"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
	"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p></body></html>", 0, QApplication::UnicodeUTF8));
}
