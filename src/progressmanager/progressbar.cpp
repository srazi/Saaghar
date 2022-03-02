/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2015-2016 by S. Razi Alavizadeh                          *
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

/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "progressbar.h"

#include <QPropertyAnimation>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <QMouseEvent>

#include "tools.h"

static const int PROGRESSBAR_HEIGHT = 13;
static const int CANCELBUTTON_WIDTH = 16;
static const int SEPARATOR_HEIGHT = 2;

// Draws a CSS-like border image where the defined borders are not stretched
static void drawCornerImage(const QImage &img, QPainter* painter, const QRect &rect,
                            int left, int top, int right, int bottom)
{
    QSize size = img.size();
    if (top > 0) { //top
        painter->drawImage(QRect(rect.left() + left, rect.top(), rect.width() - right - left, top), img,
                           QRect(left, 0, size.width() - right - left, top));
        if (left > 0) //top-left
            painter->drawImage(QRect(rect.left(), rect.top(), left, top), img,
                               QRect(0, 0, left, top));
        if (right > 0) //top-right
            painter->drawImage(QRect(rect.left() + rect.width() - right, rect.top(), right, top), img,
                               QRect(size.width() - right, 0, right, top));
    }
    //left
    if (left > 0)
        painter->drawImage(QRect(rect.left(), rect.top() + top, left, rect.height() - top - bottom), img,
                           QRect(0, top, left, size.height() - bottom - top));
    //center
    painter->drawImage(QRect(rect.left() + left, rect.top() + top, rect.width() - right - left,
                             rect.height() - bottom - top), img,
                       QRect(left, top, size.width() - right - left,
                             size.height() - bottom - top));
    if (right > 0) //right
        painter->drawImage(QRect(rect.left() + rect.width() - right, rect.top() + top, right, rect.height() - top - bottom), img,
                           QRect(size.width() - right, top, right, size.height() - bottom - top));
    if (bottom > 0) { //bottom
        painter->drawImage(QRect(rect.left() + left, rect.top() + rect.height() - bottom,
                                 rect.width() - right - left, bottom), img,
                           QRect(left, size.height() - bottom,
                                 size.width() - right - left, bottom));
        if (left > 0) //bottom-left
            painter->drawImage(QRect(rect.left(), rect.top() + rect.height() - bottom, left, bottom), img,
                               QRect(0, size.height() - bottom, left, bottom));
        if (right > 0) //bottom-right
            painter->drawImage(QRect(rect.left() + rect.width() - right, rect.top() + rect.height() - bottom, right, bottom), img,
                               QRect(size.width() - right, size.height() - bottom, right, bottom));
    }
}


ProgressBar::ProgressBar(QWidget* parent)
    : QWidget(parent), m_titleVisible(true), m_separatorVisible(true), m_cancelEnabled(true),
      m_progressHeight(0),
      m_minimum(1), m_maximum(100), m_value(1), m_cancelButtonFader(0), m_finished(false),
      m_error(false)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setMouseTracking(true);
}

ProgressBar::~ProgressBar()
{
}

bool ProgressBar::event(QEvent* e)
{
    switch (e->type()) {
    case QEvent::Enter: {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "cancelButtonFader");
        animation->setDuration(125);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    break;
    case QEvent::Leave: {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "cancelButtonFader");
        animation->setDuration(225);
        animation->setEndValue(0.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    break;
    default:
        return QWidget::event(e);
    }
    return false;
}


void ProgressBar::reset()
{
    m_value = m_minimum;
    update();
}

void ProgressBar::setRange(int minimum, int maximum)
{
    m_minimum = minimum;
    m_maximum = maximum;
    if (m_value < m_minimum || m_value > m_maximum) {
        m_value = m_minimum;
    }
    update();
}

void ProgressBar::setValue(int value)
{
    if (m_value == value
            || m_value < m_minimum
            || m_value > m_maximum) {
        return;
    }
    m_value = value;
    update();
}

void ProgressBar::setFinished(bool b)
{
    if (b == m_finished) {
        return;
    }
    m_finished = b;
    update();
}

QString ProgressBar::title() const
{
    return m_title;
}

bool ProgressBar::hasError() const
{
    return m_error;
}

void ProgressBar::setTitle(const QString &title)
{
    m_title = title;
    updateGeometry();
    update();
}

void ProgressBar::setTitleVisible(bool visible)
{
    if (m_titleVisible == visible) {
        return;
    }
    m_titleVisible = visible;
    updateGeometry();
    update();
}

bool ProgressBar::isTitleVisible() const
{
    return m_titleVisible;
}

void ProgressBar::setSeparatorVisible(bool visible)
{
    if (m_separatorVisible == visible) {
        return;
    }
    m_separatorVisible = visible;
    update();
}

bool ProgressBar::isSeparatorVisible() const
{
    return m_separatorVisible;
}

void ProgressBar::setCancelEnabled(bool enabled)
{
    if (m_cancelEnabled == enabled) {
        return;
    }
    m_cancelEnabled = enabled;
    update();
}

bool ProgressBar::isCancelEnabled() const
{
    return m_cancelEnabled;
}

void ProgressBar::setError(bool on)
{
    m_error = on;
    update();
}

QSize ProgressBar::sizeHint() const
{
    int width = 50;
    int height = PROGRESSBAR_HEIGHT + 6;
    if (m_titleVisible) {
        QFontMetrics fm(titleFont());

        width = qMax(width, Tools::horizontalAdvanceByFontMetric(fm, m_title) + 16);
        height += fm.height() + 4;
    }
    if (m_separatorVisible) {
        height += SEPARATOR_HEIGHT;
    }
    return QSize(width, height);
}

namespace
{
const int INDENT = 6;
}

void ProgressBar::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::NoModifier) {
        if (m_cancelEnabled && m_cancelRect.contains(event->pos())) {
            event->accept();
            emit clicked();
            return;
        }
        else {
            emit barClicked();
        }
    }
    QWidget::mousePressEvent(event);
}

QFont ProgressBar::titleFont() const
{
#if defined(Q_OS_MAC)
    qreal fontSize = 10.0;
#else
    qreal fontSize = 7.5;
#endif

    QFont boldFont(font());
    boldFont.setPointSizeF(fontSize);
    boldFont.setBold(true);
    return boldFont;
}

void ProgressBar::mouseMoveEvent(QMouseEvent*)
{
    update();
}

void ProgressBar::paintEvent(QPaintEvent*)
{
    if (bar.isNull()) {
        bar.load(QLatin1String(":/progressmanager/images/progressbar.png"));
    }

    double range = maximum() - minimum();
    double percent = 0.;
    if (range != 0) {
        percent = (value() - minimum()) / range;
    }
    if (percent > 1) {
        percent = 1;
    }
    else if (percent < 0) {
        percent = 0;
    }

    if (finished()) {
        percent = 1;
    }

    QPainter p(this);
    QFont fnt(titleFont());
    p.setFont(fnt);
    QFontMetrics fm(fnt);

    int titleHeight = m_titleVisible ? fm.height() : 0;

    // Draw separator
    int separatorHeight = m_separatorVisible ? SEPARATOR_HEIGHT : 0;
    if (m_separatorVisible) {
        p.setPen(QColor(0, 0, 0, 40));
        p.drawLine(0, 0, size().width(), 0);

        p.setPen(QColor(255, 255, 255, 40));
        p.drawLine(1, 1, size().width(), 1);
    }

    if (m_titleVisible) {
        QRect textBounds = fm.boundingRect(m_title);
        textBounds.moveCenter(rect().center());
        int alignment = Qt::AlignHCenter;

        int textSpace = rect().width() - 8;
        // If there is not enough room when centered, we left align and
        // elide the text
        QString elidedtitle  = fm.elidedText(m_title, Qt::ElideRight, textSpace);

        QRect textRect = rect().adjusted(3, separatorHeight - 1, -3, 0);
        textRect.setHeight(titleHeight + 5);

        p.setPen(QColor(0, 0, 0, 120));
        p.drawText(textRect, alignment | Qt::AlignBottom, elidedtitle);
        p.translate(0, -1);
        p.setPen(Qt::white);
        p.drawText(textRect, alignment | Qt::AlignBottom, elidedtitle);
        p.translate(0, 1);
    }

    m_progressHeight = PROGRESSBAR_HEIGHT;
    m_progressHeight += ((m_progressHeight % 2) + 1) % 2; // make odd
    // draw outer rect
    const QRect rect(INDENT - 1, titleHeight + separatorHeight + (m_titleVisible ? 4 : 3),
                     size().width() - 2 * INDENT + 1, m_progressHeight);

    drawCornerImage(bar, &p, rect, 3, 3, 3, 3);

    // draw inner rect
    QColor c(Qt::white);
    p.setPen(Qt::NoPen);

    QRectF inner = rect.adjusted(2, 2, -2, -2);
    inner.adjust(0, 0, qRound((percent - 1) * inner.width()), 0);
    if (m_error) {
        QColor red(255, 60, 0, 210);
        c = red;
        // avoid too small red bar
        if (inner.width() < 10) {
            inner.adjust(0, 0, 10 - inner.width(), 0);
        }
    }
    else if (m_finished) {
        c = QColor(90, 170, 60);
    }

    // Draw line and shadow after the gradient fill
    if (value() > 0 && value() < maximum()) {
        p.fillRect(QRect(inner.right(), inner.top(), 2, inner.height()), QColor(0, 0, 0, 20));
        p.fillRect(QRect(inner.right(), inner.top(), 1, inner.height()), QColor(0, 0, 0, 60));
    }
    p.setPen(Qt::NoPen);

    QLinearGradient grad(inner.topLeft(), inner.bottomLeft());
    grad.setColorAt(0, c.lighter(130));
    grad.setColorAt(0.4, c.lighter(106));
    grad.setColorAt(0.41, c.darker(106));
    grad.setColorAt(1, c.darker(130));
    p.setBrush(grad);

    p.drawRect(inner);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(0, 0, 0, 30), 1));

    p.drawLine(inner.topLeft() + QPointF(0.5, 0.5), inner.topRight() + QPointF(-0.5, 0.5));
    p.drawLine(inner.topLeft() + QPointF(0.5, 0.5), inner.bottomLeft() + QPointF(0.5, -0.5));
    p.drawLine(inner.topRight() + QPointF(-0.5, 0.5), inner.bottomRight() + QPointF(-0.5, -0.5));
    p.drawLine(inner.bottomLeft() + QPointF(0.5, -0.5), inner.bottomRight() + QPointF(-0.5, -0.5));

    if (m_cancelEnabled) {
        // Draw cancel button
        p.setOpacity(m_cancelButtonFader);

        if (value() < maximum() && !m_error) {
            m_cancelRect = QRect(rect.adjusted(rect.width() - CANCELBUTTON_WIDTH + 2, 1, 0, 0));
            const bool hover = m_cancelRect.contains(mapFromGlobal(QCursor::pos()));
            const QRectF cancelVisualRect(m_cancelRect.adjusted(0, 1, -2, -2));
            QLinearGradient grad(cancelVisualRect.topLeft(), cancelVisualRect.bottomLeft());
            int intensity = hover ? 90 : 70;
            QColor buttonColor(intensity, intensity, intensity, 255);
            grad.setColorAt(0, buttonColor.lighter(130));
            grad.setColorAt(1, buttonColor.darker(130));
            p.setPen(Qt::NoPen);
            p.setBrush(grad);
            p.drawRect(cancelVisualRect);

            p.setPen(QPen(QColor(0, 0, 0, 30)));
            p.drawLine(cancelVisualRect.topLeft() + QPointF(-0.5, 0.5), cancelVisualRect.bottomLeft() + QPointF(-0.5, -0.5));
            p.setPen(QPen(QColor(0, 0, 0, 120)));
            p.drawLine(cancelVisualRect.topLeft() + QPointF(0.5, 0.5), cancelVisualRect.bottomLeft() + QPointF(0.5, -0.5));
            p.setPen(QPen(QColor(255, 255, 255, 30)));
            p.drawLine(cancelVisualRect.topLeft() + QPointF(1.5, 0.5), cancelVisualRect.bottomLeft() + QPointF(1.5, -0.5));
            p.setPen(QPen(hover ? Qt::white : QColor(180, 180, 180), 1.2, Qt::SolidLine, Qt::FlatCap));
            p.setRenderHint(QPainter::Antialiasing, true);
            p.drawLine(cancelVisualRect.topLeft() + QPointF(4.0, 2.0), cancelVisualRect.bottomRight() + QPointF(-3.0, -2.0));
            p.drawLine(cancelVisualRect.bottomLeft() + QPointF(4.0, -2.0), cancelVisualRect.topRight() + QPointF(-3.0, 2.0));
        }
    }
}
