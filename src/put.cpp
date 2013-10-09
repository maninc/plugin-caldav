/*
 * This file is part of buteo-gcontact-plugin package
 *
 * Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
 *
 * Contributors: Mani Chandrasekar <maninc@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "put.h"
#include "settings.h"

#include <QNetworkAccessManager>
#include <QBuffer>
#include <QDebug>

#include <LogMacros.h>

#include <incidence.h>
#include <icalformat.h>

Put::Put(QNetworkAccessManager *manager, Settings *settings) :
    Request(manager, settings, "PUT")
{
}

void Put::updateEvent(const QString etag, const QString data) {
    QNetworkRequest request;
    request.setUrl(mSettings->makeUrl());

    if (!mSettings->authToken().isEmpty()) {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer " + mSettings->authToken()).toLatin1());
    }
    request.setRawHeader("If-Match", etag.toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/calendar; charset=utf-8");

    QBuffer *buffer = new QBuffer;
    buffer->setData(data.toLatin1());
    mNReply = mNAManager->sendCustomRequest(request, REQUEST_TYPE.toLatin1(), buffer);

    connect(mNReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(mNReply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));

}

void Put::createEvent(KCalCore::Incidence::Ptr incidence) {

    KCalCore::ICalFormat *icalFormat = new KCalCore::ICalFormat;
    QString ical = icalFormat->toICalString(incidence);
    if (ical == NULL) {
        LOG_WARNING("Error while converting iCal Object to string");
        return;
    }

    QNetworkRequest request;
    QUrl url(mSettings->url() + incidence->uid() + ".ics");
    request.setRawHeader("If-None-Match", "*");
    request.setHeader(QNetworkRequest::ContentLengthHeader, ical.length());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/calendar; charset=utf-8");
    if (!mSettings->authToken().isEmpty()) {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer " + mSettings->authToken()).toLatin1());
    } else {
        url.setUserName(mSettings->username());
        url.setPassword(mSettings->password());
    }
    request.setUrl(url);

//    qDebug() << "====================================== \n";
//    qDebug() << request.url().toString();
//    const QList<QByteArray>& rawHeaderList(request.rawHeaderList());
//    foreach (QByteArray rawHeader, rawHeaderList) {
//        qDebug() << rawHeader << " : " << request.rawHeader(rawHeader);
//    }
//    qDebug() << "========================================\n";
//    qDebug() << "\n\n" << ical;
    QBuffer *buffer = new QBuffer;
    buffer->setData(ical.toLatin1());
    mNReply = mNAManager->sendCustomRequest(request, REQUEST_TYPE.toLatin1(), buffer);

    connect(mNReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(mNReply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));

}

void Put::slotError(QNetworkReply::NetworkError error) {
    qDebug() << "Error # " << error;
}

void Put::slotSslErrors(QList<QSslError> errors) {
    qDebug() << "SSL Error";
    if (mSettings->ignoreSSLErrors()) {
        mNReply->ignoreSslErrors(errors);
    }
}
