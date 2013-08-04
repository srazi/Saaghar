/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2013 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).       *
 * All rights reserved.                                                    *
 * Contact: Nokia Corporation (qt-info@nokia.com)                          *
 *                                                                         *
 * This file is part of the examples of the Qt Toolkit.                    *
 *                                                                         *
 * $QT_BEGIN_LICENSE:BSD$                                                  *
 * You may use this file under the terms of the BSD license as follows:    *
 *                                                                         *
 * "Redistribution and use in source and binary forms, with or without     *
 * modification, are permitted provided that the following conditions are  *
 * met:                                                                    *
 *   * Redistributions of source code must retain the above copyright      *
 *     notice, this list of conditions and the following disclaimer.       *
 *   * Redistributions in binary form must reproduce the above copyright   *
 *     notice, this list of conditions and the following disclaimer in     *
 *     the documentation and/or other materials provided with the          *
 *     distribution.                                                       *
 *   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor  *
 *     the names of its contributors may be used to endorse or promote     *
 *     products derived from this software without specific prior written  *
 *     permission.                                                         *
 *                                                                         *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   *
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."   *
 * $QT_END_LICENSE$                                                        *
 *                                                                         *
 ***************************************************************************/

#ifndef DOWNLOADER_H
#define DOWNLOADER_H
#include <QWidget>
#include <QNetworkAccessManager>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QFile;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSslError;
class QAuthenticator;
class QNetworkReply;
class QEventLoop;
QT_END_NAMESPACE


class Downloader : public QObject
{
    Q_OBJECT

public:
    Downloader(QObject* parent = 0, QProgressBar* progressBar = 0, QLabel* statusLabel = 0);

    void startRequest(QUrl url);
    inline bool hasError() { return m_hasError; }
    //QProgressBar *downloadProgressBar(QWidget *parent = 0);
    QEventLoop* loop;

public slots:
    void downloadFile(const QUrl &downloadUrl, const QString &path, const QString &title = "");
    void cancelDownload();

private slots:
    void redirectTo(const QUrl &newUrl);
    void requestFinished();
    void replyReadyRead();
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
    //void enableDownloadButton();
    void slotAuthenticationRequired(QNetworkReply*, QAuthenticator*);
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply*, const QList<QSslError> &errors);
#endif

private:
    QString dataText;
    QString downloadTitle;
    QUrl url;
    QLabel* _statusLabel;
    QProgressBar* _progressBar;

    QNetworkAccessManager qnam;
    QNetworkReply* reply;
    QFile* file;
    int httpGetId;
    bool requestAborted;
    bool m_hasError;

signals:
    void downloadStopped();
};

#endif
