/***************************************************************************
 *  This file is part of QExtendedSplashScreen, a custom Qt widget         *
 *                                                                         *
 *  Copyright (C) 2012-2014 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
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

#include "qextendedsplashscreen.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QDesktopWidget>
#include <QPainter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QProgressBar>
#include <QBitmap>
#include <QStyle>

//#include <QDebug>

QExtendedSplashScreen::QExtendedSplashScreen(const QPixmap &pixmap, Qt::WindowFlags f)
    : QWidget(0, Qt::FramelessWindowHint | f)
{
    init(pixmap);
}

QExtendedSplashScreen::QExtendedSplashScreen(QWidget* parent, const QPixmap &pixmap, Qt::WindowFlags f)
    : QWidget(parent, Qt::FramelessWindowHint | f)
{
    init(pixmap);
}

QExtendedSplashScreen::~QExtendedSplashScreen()
{
}

void QExtendedSplashScreen::init(const QPixmap &pixmap)
{
    m_isFinished = false;
    m_progressBar = 0;

    setAttribute(Qt::WA_TranslucentBackground);
    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    pixmapLabel = new QLabel(this);

    mainLayout->addWidget(pixmapLabel);
    this->setLayout(mainLayout);

    showMessage("", int(Qt::AlignLeft), Qt::black);
    setPixmap(pixmap);
}

void QExtendedSplashScreen::finish(QWidget* mainWin)
{
    if (mainWin) {
#if defined(Q_OS_X11)
        extern void qt_x11_wait_for_window_manager(QWidget * mainWin);
        qt_x11_wait_for_window_manager(mainWin);
#endif
    }
    close();
    m_isFinished = true;
}

void QExtendedSplashScreen::repaint()
{
    drawContents();
    QWidget::repaint();
    QApplication::flush();
}

void QExtendedSplashScreen::show()
{
    if (!m_isFinished) {
        QWidget::show();
    }
}

void QExtendedSplashScreen::forceRepaint()
{
    repaint();
    QCoreApplication::processEvents();
}

void QExtendedSplashScreen::setProgressBar(const QPixmap &maskPixmap, int minimum, int maximum, Qt::Orientation o)
{
    if (!m_progressBar) {
        m_progressBar = new QProgressBar(this);
        m_progressBar->setAttribute(Qt::WA_TranslucentBackground);
        m_progressBar->setTextVisible(false);

        bool badProgressBar = style()->objectName() == "cde" || style()->objectName() == "motif" ||
                              style()->objectName().startsWith("macintosh") || style()->objectName() == "windows" ||
                              style()->objectName() == "windowsxp";
        if (badProgressBar) {
            m_progressBar->setStyleSheet("QProgressBar{border: none; background: transparent; width: 100px; height: 100px;}"
                                         "QProgressBar::chunk {width: 15px; height: 15px; margin: -5px;"
                                         "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #9F9, stop: 0.4 #4F4, stop: 0.5 #4F4, stop: 1.0 #3E3);}");
        }
        else {
            m_progressBar->setStyleSheet("QProgressBar{ background: transparent; border:none;}");
        }
    }
    m_progressBar->setMinimum(minimum);
    m_progressBar->setMaximum(maximum);
    m_progressBar->setFixedSize(maskPixmap.size());
    m_progressBar->setOrientation(o);
    m_progressBar->setMask(maskPixmap.mask());

    forceRepaint();
}

void QExtendedSplashScreen::addToMaximum(int number, bool forceUpdate)
{
    if (!m_progressBar || m_progressBar->value() == m_progressBar->maximum()) {
        return;
    }

    if (number > 0) {
        m_progressBar->setMaximum(m_progressBar->maximum() + number);
    }
    else if (number == -1) {
        m_progressBar->setValue(m_progressBar->maximum());
    }

    if (forceUpdate && (number > 0 || number == -1)) {
        forceRepaint();
    }
}

void QExtendedSplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
    textColor = color;
    textAlign = alignment;
    showMessage(message);
}

void QExtendedSplashScreen::showMessage(const QString &message)
{
    if (message.startsWith("!QExtendedSplashScreenCommands:")) {
        QString command = message;
        command.remove("!QExtendedSplashScreenCommands:");
        if (command == "CLOSE") {
            close();
        }
        else if (command == "HIDE") {
            hide();
        }
        else if (command == "SHOW") {
            show();
        }

        return;
    }
    if (m_progressBar) {
        if (m_progressBar->value() < m_progressBar->maximum()) {
            m_progressBar->setValue(m_progressBar->value() + 1);
        }
    }
    currentStatus = message;
    forceRepaint();
    emit messageChanged(message);
}

void QExtendedSplashScreen::clearMessage()
{
    showMessage("");
}

void QExtendedSplashScreen::mouseDoubleClickEvent(QMouseEvent*)
{
    close();
}

void QExtendedSplashScreen::drawContents()
{
    if (!splashPixmap.isNull()) {
        QPixmap textPix = splashPixmap;
        QPainter painter(&textPix);
        painter.initFrom(this);
        drawContents(&painter);
        pixmapLabel->setPixmap(textPix);
    }
}

void QExtendedSplashScreen::drawContents(QPainter* painter)
{
    painter->setPen(textColor);

    if (Qt::mightBeRichText(currentStatus)) {
        QTextDocument doc;
#ifdef QT_NO_TEXTHTMLPARSER
        doc.setPlainText(currentStatus);
#else
        //this does not override any existence of color formatting
        doc.setHtml("<span style=\"color: " + textColor.name() + ";\">" + currentStatus + "</span>");
#endif
        doc.setTextWidth(textRect.width());
        QTextCursor cursor(&doc);
        cursor.select(QTextCursor::Document);
        QTextBlockFormat fmt;
        fmt.setAlignment(Qt::Alignment(textAlign));
        cursor.mergeBlockFormat(fmt);
        int docHeight = doc.blockCount() * doc.documentLayout()->blockBoundingRect(doc.begin()).height();

        painter->save();

        switch (Qt::Alignment(textAlign) & Qt::AlignVertical_Mask) {
        case Qt::AlignTop:
            painter->translate(textRect.topLeft());
            break;

        case Qt::AlignVCenter:
            painter->translate(QPoint(textRect.left(), textRect.top() + qMax((textRect.height() - docHeight) / 2 - 5, 0)));
            break;

        case Qt::AlignBottom:
            painter->translate(QPoint(textRect.left(), textRect.top() + qMax(textRect.height() - docHeight - 5, 0)));
            break;

        default:
            break;
        }

        doc.drawContents(painter);
        painter->restore();
    }
    else {
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
