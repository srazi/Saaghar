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

#include <QProgressBar>
#include <QMessageBox>
#include <QtNetwork>
#include <QEventLoop>

#include "downloader.h"
#include "ui_authenticationdialog.h"

Downloader::Downloader(QObject* parent, QProgressBar* progressBar, QLabel* statusLabel)
    : QObject(parent)
    , m_hasError(false)
{
    downloadTitle = "";
    loop = new QEventLoop(parent);
    _progressBar = progressBar;
    _statusLabel = statusLabel;

    connect(&qnam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(&qnam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif
}

void Downloader::startRequest(QUrl url)
{
    m_hasError = false;
    //loop->quit();
    QNetworkRequest request(url);//QNetworkRequest(url)
    //request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows; U; Windows NT 6.0; en-US; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7 (.NET CLR 3.5.30729)");
    reply = qnam.get(request);

    connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(replyReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDataReadProgress(qint64,qint64)));
    connect(reply, SIGNAL(finished()), loop, SLOT(quit()));
    qDebug() << "looooooooooop started";

    if (_progressBar) {
        _progressBar->setValue(0);
        _progressBar->show();
    }
    if (_statusLabel) {
        _statusLabel->show();
        if (downloadTitle.isEmpty()) {
            _statusLabel->setText(tr("Downloading..."));
        }
        else {
            _statusLabel->setText(downloadTitle + tr(" is downloading..."));
        }
    }

    loop->exec();
}

void Downloader::downloadFile(const QUrl &downloadUrl, const QString &path, const QString &title)
{
    qDebug() << "downloadFile=url=" << url << "Before" << "downloadUrl=" << downloadUrl << "path==" << path;
    downloadTitle = title;
    url = downloadUrl;
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty()) {
        fileName = "index.html";
    }

    QString filePath = fileName;
    if (!path.isEmpty()) {
        filePath = path + "/" + fileName;
    }
    qDebug() << "downloadFile=filePath=" << filePath;
    if (QFile::exists(filePath)) {
        QMessageBox question(QMessageBox::Question, tr("Downloader"), tr("There already exists a file called %1 in %2. Overwrite?")
                            .arg(fileName).arg(QDir(filePath).path()), QMessageBox::Yes | QMessageBox::No
                            , 0, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
        question.setDefaultButton(QMessageBox::No);

        if (question.exec() == QMessageBox::No) {
            m_hasError = true;
            loop->quit();
            emit downloadStopped();
            return;
        }
        QFile::remove(filePath);
    }

    file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(0, tr("Downloader"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        m_hasError = true;
        delete file;
        file = 0;
        loop->quit();
        emit downloadStopped();
        return;
    }

    // schedule the request
    requestAborted = false;
    startRequest(url);
}

void Downloader::cancelDownload()
{
    if (_statusLabel) {
        _statusLabel->setText(tr("Download canceled."));
    }
    loop->quit();
    requestAborted = true;
    if (reply) {
        reply->abort();
    }
    //downloadButton->setEnabled(true);
}

void Downloader::requestFinished()
{
    dataText = "";
    qDebug() << "URL==" << url;
    if (requestAborted) {
        if (file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        reply->deleteLater();
        loop->quit();
        if (_progressBar) {
            _progressBar->hide();
        }
        emit downloadStopped();

        m_hasError = true;
        return;
    }

    if (_progressBar) {
        _progressBar->hide();
    }
//#ifndef Q_OS_MAEMO_5
//  progressDialog->hide();
//#endif
    file->flush();
    file->close();


    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        file->remove();
        QMessageBox::information(0, tr("Downloader"),
                                 tr("Download failed: %1.")
                                 .arg(reply->errorString()));
        m_hasError = true;
    }
    else if (!redirectionTarget.isNull()) {
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        qDebug() << "redirect-URL==" << url << "newURl=" << newUrl;
        redirectTo(newUrl);
        return;
    }
    else {
        //QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
        if (_statusLabel) {
            _statusLabel->setText(tr("Download Completed."));
            //  _statusLabel->setText(tr("Download Completed."));//.arg(fileName).arg(QDir::currentPath()));
        }
        //downloadButton->setEnabled(true);
    }

    loop->quit();
    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;
    emit downloadStopped();
}

void Downloader::replyReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    QByteArray dataToWrite = reply->readAll();
    QRegExp htmlRedirect("<meta\\s+http-equiv\\s*=\\s*\"refresh\"\\s+content\\s*=\\s*\"\\s*[0-9]+\\s*;\\s*url\\s*=\\s*([^\"]+)\\s*\"\\s*>", Qt::CaseInsensitive);
    //<meta\s+http-equiv\s*=\s*"refresh"\s+content\s*=\s*"\s*[0-9]+\s*;\s*url\s*=\s*([^"]+)\s*"\s*>
    dataText += dataToWrite;
    dataText.indexOf(htmlRedirect);
//  qDebug()<<"====replyReadyRead==================================================";
//  qDebug()<<dataText;
//  qDebug()<<"cap_0="<< htmlRedirect.cap(0)<<"cap_1" <<htmlRedirect.cap(1);
//  qDebug()<<"====replyReadyRead==================================================";
    QString reirectUrl = htmlRedirect.cap(1);
    if (!reirectUrl.isEmpty()) {
        //qDebug() <<"============================\ndataText=\n"<<dataText<<"\n========================\n";
        //qDebug()<<"====reirectUrl="<<reirectUrl;
        dataText = "";
        QUrl newUrl(reirectUrl);
        if (reirectUrl.contains("sourceforge") && reirectUrl.contains("use_mirror")) {
            //unfortunately this redirection uses cookies and we need QWebPage for tracking this type of redirections
            //this is an ugly trick for creating direct link for sourceforge!
            //http://ignum.dl.sourceforge.net/project/ganjoor/gdb/ahmd-prvin.zip?r=&amp;ts=1337765789&amp;use_mirror=garr
            QRegExp mirror(".*use_mirror=(.*)");
            reirectUrl.indexOf(mirror);
            QString mirrorStr = mirror.cap(1);
            if (mirrorStr.isEmpty()) {
                mirrorStr = "mesh";
            }
            reirectUrl.replace("downloads", mirrorStr + ".dl");
            qDebug() << "triiiiiiiiicky==" << reirectUrl << "----" << newUrl.allQueryItemValues("use_mirror");
        }
        redirectTo(QUrl(reirectUrl));
        return;
    }
    if (file) {
        file->write(dataToWrite);
    }
}

void Downloader::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (requestAborted) {
        return;
    }
    if (_progressBar) {
        _progressBar->setMaximum(totalBytes);
        _progressBar->setValue(bytesRead);
    }
}

//void Downloader::enableDownloadButton()
//{
//  //downloadButton->setEnabled(!urlLineEdit->text().isEmpty());
//}

void Downloader::slotAuthenticationRequired(QNetworkReply*, QAuthenticator* authenticator)
{
    QDialog dlg;
    Ui::Dialog ui;
    ui.setupUi(&dlg);
    dlg.adjustSize();
    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm()).arg(url.host()));

    // Did the URL have information? Fill the UI
    // This is only relevant if the URL-supplied credentials were wrong
    ui.userEdit->setText(url.userName());
    ui.passwordEdit->setText(url.password());

    if (dlg.exec() == QDialog::Accepted) {
        authenticator->setUser(ui.userEdit->text());
        authenticator->setPassword(ui.passwordEdit->text());
    }
}

void Downloader::redirectTo(const QUrl &newUrl)
{
    url = newUrl;
    disconnect(reply, SIGNAL(finished()), loop, SLOT(quit()));
    disconnect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    disconnect(reply, SIGNAL(readyRead()), this, SLOT(replyReadyRead()));
    disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDataReadProgress(qint64,qint64)));
    if (reply) {
        reply->deleteLater();
    }
    if (file) {
        file->open(QIODevice::WriteOnly);
        file->resize(0);
    }

    //we can save file with new name by calling downloadFile()
    startRequest(url);
}

//QProgressBar *Downloader::downloadProgressBar(QWidget *parent)
//{
//  progressBar->setParent(parent);
//  return progressBar;
//}

#ifndef QT_NO_OPENSSL
void Downloader::sslErrors(QNetworkReply*, const QList<QSslError> &errors)
{
    QString errorString;
    foreach (const QSslError &error, errors) {
        if (!errorString.isEmpty()) {
            errorString += ", ";
        }
        errorString += error.errorString();
    }

    if (QMessageBox::warning(0, tr("Downloader"),
                             tr("One or more SSL errors has occurred: %1").arg(errorString),
                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
        reply->ignoreSslErrors();
    }
}
#endif
