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

#include "report.h"
#include "reader.h"
#include "put.h"
#include "settings.h"

#include <QNetworkAccessManager>
#include <QBuffer>
#include <QDebug>

#include <icalformat.h>


Report::Report(QNetworkAccessManager *manager, Settings *settings) :
    Request(manager, settings, "REPORT")
{
    FUNCTION_CALL_TRACE;
}

void Report::getAllEvents() {
    QNetworkRequest request;
    request.setUrl(mSettings->makeUrl());
    LOG_DEBUG(mSettings->makeUrl());
    if (!mSettings->authToken().isEmpty()) {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer " + mSettings->authToken()).toLatin1());
    }

    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");

    QBuffer *buffer = new QBuffer;
    buffer->setData("<c:calendar-query xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">" \
                    "<d:prop> <d:getetag /> <c:calendar-data /> </d:prop>"       \
                    "<c:filter> <c:comp-filter name=\"VCALENDAR\" /> </c:filter>" \
                    "</c:calendar-query>");
    mNReply = mNAManager->sendCustomRequest(request, REQUEST_TYPE.toLatin1(), buffer);

    connect(mNReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(mNReply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void Report::getAllETags() {
    QNetworkRequest request;
    LOG_DEBUG(mSettings->makeUrl());
    request.setUrl(mSettings->makeUrl());
    if (!mSettings->authToken().isEmpty()) {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer " + mSettings->authToken()).toLatin1());
    }
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");

    QBuffer *buffer = new QBuffer;
    buffer->setData("<c:calendar-query xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">" \
                    "<d:prop> <d:getetag /> </d:prop>" \
                    "<c:filter><c:comp-filter name=\"VCALENDAR\"><c:comp-filter name=\"VTODO\" /></c:comp-filter></c:filter>" \
                    "</c:calendar-query>");
    mNReply = mNAManager->sendCustomRequest(request, REQUEST_TYPE.toLatin1(), buffer);

    connect(mNReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(mNReply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void Report::multiGetEvents(const QStringList eventIdList) {
    if (eventIdList.isEmpty()) return;

    QNetworkRequest request;
    request.setUrl(mSettings->makeUrl());
    if (!mSettings->authToken().isEmpty()) {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer " + mSettings->authToken()).toLatin1());
    }
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");

    QString multiGetRequest = "<c:calendar-multiget xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">" \
                              "<d:prop><d:getetag /><c:calendar-data /></d:prop>";
    foreach (QString eventId , eventIdList) {
        multiGetRequest.append("<d:href>");
        multiGetRequest.append(eventId);
        multiGetRequest.append("</d:href>");
    }
    multiGetRequest.append("</c:calendar-multiget>");

    QBuffer *buffer = new QBuffer;
    buffer->setData(multiGetRequest.toLatin1());
    mNReply = mNAManager->sendCustomRequest(request, REQUEST_TYPE.toLatin1(), buffer);

    connect(mNReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(mNReply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void Report::slotError(QNetworkReply::NetworkError error) {
    qDebug() << "Error # " << error;
}

void Report::slotSslErrors(QList<QSslError> errors) {
    qDebug() << "SSL Error";
    if (mSettings->ignoreSSLErrors()) {
        mNReply->ignoreSslErrors(errors);
    }
}
