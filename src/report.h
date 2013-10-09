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

#ifndef REPORT_H
#define REPORT_H

#include "request.h"

#include <QObject>

class QNetworkAccessManager;
class Settings;

class Report : public Request
{
public:
    explicit Report(QNetworkAccessManager *manager, Settings *settings);

    void getAllEvents();
    void getAllETags();
    void multiGetEvents(const QStringList eventIdList);
    
public slots:
    void slotError(QNetworkReply::NetworkError error);
    void slotSslErrors(QList<QSslError> errors);
};

#endif // REPORT_H
