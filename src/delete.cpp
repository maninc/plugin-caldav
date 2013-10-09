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

#include "delete.h"
#include "settings.h"

#include <QNetworkAccessManager>
#include <QBuffer>
#include <QDebug>

Delete::Delete(QNetworkAccessManager *manager, Settings *settings) :
    Request(manager, settings, "DELETE")
{
}


void Delete::deleteEvent() {
    QNetworkRequest request;
    request.setUrl(mSettings->makeUrl());
    if (!mSettings->authToken().isEmpty()) {
        request.setRawHeader(QString("Authorization").toLatin1(),
                             QString("Bearer " + mSettings->authToken()).toLatin1());
    }

    mNReply = mNAManager->sendCustomRequest(request, REQUEST_TYPE.toLatin1());

    connect(mNReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(mNReply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));

}
void Delete::slotError(QNetworkReply::NetworkError error) {
    qDebug() << "Error # " << error;
}

void Delete::slotSslErrors(QList<QSslError> errors) {
    qDebug() << "SSL Error";
    if (mSettings->ignoreSSLErrors()) {
        mNReply->ignoreSslErrors(errors);
    }
}
