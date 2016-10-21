/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2016 by S. Razi Alavizadeh                               *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"

#include <QBitmap>
#include <QDebug>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent, Qt::Window | Qt::FramelessWindowHint)
    , ui(new Ui::AboutDialog)
{
    setWindowTitle(tr("About Saaghar"));

    ui->setupUi(this);

    QPixmap pixmap(":/resources/images/saaghar-about.png");
    setFixedSize(pixmap.size());

    QPalette p(palette());
    p.setBrush(QPalette::Window, QBrush(pixmap));

    setPalette(p);
    setMask(pixmap.mask());

    QFont fnt(ui->textBrowser->font());
    fnt.setPixelSize(qMax(18, fnt.pixelSize()));
    ui->textBrowser->setFont(fnt);
    ui->textBrowser->setStyleSheet("\
                                 QTextEdit{text-decoration: none; border: transparent;\
                                           background-image:url(\":/resources/images/semi-white.png\");\
                                 }\
                                 QScrollBar:vertical {\
                                     border: none;\
                                     background: transparent;\
                                     width: 13px;\
                                 }\
                                 QScrollBar::handle:vertical {\
                                     border: none;\
                                     background: #bebebe;\
                                     min-height: 20px;\
                                 }\
                                 QScrollBar::add-line:vertical {\
                                     height: 0px;\
                                 }\
                                 \
                                 QScrollBar::sub-line:vertical {\
                                     height: 0px;\
                                 }\
                                 \
                                 QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
                                     background: none;\
                                 }\
                                 \
                                 QScrollBar:vertical:hover {\
                                     background: transparent;\
                                 }\
                                 QScrollBar::handle:vertical:hover {\
                                     background: #aaa;\
                                 }\
                                 ");

    ui->closeLabel->setStyleSheet("QWidget {padding-top: 7px; border: transparent; background-image:url(\":/resources/images/semi-white.png\");}");
    ui->closeLabel->setText("<a style=\"color: #07F; text-decoration:none;\" href=#CLOSE>CLOSE</a>");

    QString gitRevision = GIT_REVISION;
    if (!gitRevision.isEmpty()) {
        gitRevision = tr("Revision: %1<br />").arg(gitRevision);
    }

    const QString listItem = QString("<li>%1</li>");
    QString relatedPages;
    relatedPages += listItem.arg(tr("Site (English): ") + "<a style=\"color: #07F; text-decoration:none;\" href=\"http://en.saaghar.pozh.org/\">en.saaghar.pozh.org</a>");
    relatedPages += listItem.arg(tr("Site (Persian): ") + "<a style=\"color: #07F; text-decoration:none;\" href=\"http://saaghar.pozh.org/\">saaghar.pozh.org</a>");
    relatedPages += listItem.arg(tr("Mailing List: ") + "<a style=\"color: #07F; text-decoration:none;\" href=\"http://groups.google.com/group/saaghar/\">" + tr("Saaghar Google Group") + "</a>");
    relatedPages += listItem.arg(tr("Facebook Page: ") + "<a style=\"color: #07F; text-decoration:none;\" href=\"http://www.facebook.com/saaghar.p/\">" + "saaghar.p" + "</a>");

    QString speceialThanks;
    speceialThanks += listItem.arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://www.phototak.com/\">" + tr("Nasser Alavizadeh") + "</a> " + tr("(Logo Idea/Design)"));
    speceialThanks += listItem.arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://www.gozir.com/\">" + tr("Hamid Reza Mohammadi") + "</a> " + tr("(Ganjoor Founder)"));
    speceialThanks += listItem.arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://\">" + tr("Siyvash Kiani") + "</a> " + tr("(Expert Assistance in Literature)"));
    speceialThanks += listItem.arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://homayounshajarian.blogfa.com/\">" + tr("Sahand Soltandoost") + "</a> " + tr("(Light-Gray Iconset)"));
    speceialThanks += listItem.arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://www.useiconic.com/open/\">" + tr("Open Iconic Project") + "</a> " + tr("(Iconic-Cyan Iconset, <i>modified version</i>)"));
    speceialThanks += listItem.arg("<a style=\"color: #07F; text-decoration:none;\" href=\"https://www.qt.io/ide/\">" + tr("Qt Creator Team") + "</a> " + tr("(Code of Tasks Notification)"));
    speceialThanks += listItem.arg("<a style=\"color: #07F; text-decoration:none;\" href=\"https://ganjoor.net/\">" + tr("All Ganjoor Project Contributors") + "</a> " + tr("(Persian-Datasets/Audio)"));


    const QString noWarranty = tr("<br /><br />This program is distributed in the hope that it will be useful,"
                                  " but WITHOUT ANY WARRANTY; without even the implied warranty of"
                                  " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");

    setText(tr("<h3>Saaghar %1</h3>%2Build Time: %3<br /><br />"
               "%4 is a Persian poem viewer software written using Qt (C++)."
               " It uses %5 database for Persian poems."
               "<br /><br />%6"
               "<h4>Related Pages:</h4>%7"
               "<h4>Special Thanks to:</h4>%8%9")
            .arg(SAAGHAR_VERSION).arg(gitRevision).arg(BUILD_TIME)
            .arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://saaghar.pozh.org\">" + tr("Saaghar") + "</a>")
            .arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://ganjoor.net/\">" + tr("Ganjoor") + "</a>")
            .arg(tr("Copyright 2010-2016 %1. All right reserved.").arg("<a style=\"color: #07F; text-decoration:none;\" href=\"http://pozh.org/\">" + tr("Razi Alavizadeh") + "</a>"))
            .arg(QString("<ul>%1</ul>").arg(relatedPages))
            .arg(QString("<ul>%1</ul>").arg(speceialThanks))
            .arg(noWarranty));

    connect(ui->closeLabel, SIGNAL(linkActivated(QString)), this, SLOT(accept()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::setText(const QString &text)
{
    ui->textBrowser->setText(text);
}
