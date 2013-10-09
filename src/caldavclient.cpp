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

#include "caldavclient.h"
#include "report.h"
#include "put.h"
#include "delete.h"
#include "reader.h"

#include <extendedcalendar.h>
#include <extendedstorage.h>
#include <notebook.h>

#include<QNetworkReply>

#include <PluginCbInterface.h>
#include <LogMacros.h>
#include <ProfileEngineDefs.h>
#include <ProfileManager.h>

extern "C" CalDavClient* createPlugin(const QString& aPluginName,
                                         const Buteo::SyncProfile& aProfile,
                                         Buteo::PluginCbInterface *aCbInterface) {
    return new CalDavClient(aPluginName, aProfile, aCbInterface);
}

extern "C" void destroyPlugin(CalDavClient *aClient) {
    delete aClient;
}

CalDavClient::CalDavClient(const QString& aPluginName,
                            const Buteo::SyncProfile& aProfile,
                            Buteo::PluginCbInterface *aCbInterface) :
    ClientPlugin(aPluginName, aProfile, aCbInterface), mSlowSync (true), mOAuth(NULL)
{
    FUNCTION_CALL_TRACE;
}

CalDavClient::~CalDavClient() {
    FUNCTION_CALL_TRACE;
}

bool CalDavClient::init() {
    FUNCTION_CALL_TRACE;

    if (lastSyncTime().isNull ())
        mSlowSync = true;
    else
        mSlowSync = false;

    mNAManager = new QNetworkAccessManager;
    connect(mNAManager, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(replyFinished(QNetworkReply*)));

    if (initConfig ()) {
        return true;
    } else {
        // Uninitialize everything that was initialized before failure.
        uninit();
        return false;
    }
}

bool CalDavClient::uninit() {
    FUNCTION_CALL_TRACE;

    return true;
}

bool CalDavClient::startSync() {
    FUNCTION_CALL_TRACE;

    if (!mOAuth)
        return false;

    connect(this, SIGNAL(stateChanged(Sync::SyncProgressDetail)),
            this, SLOT(receiveStateChanged(Sync::SyncProgressDetail)));
    connect(this, SIGNAL(syncFinished(Sync::SyncStatus)),
            this, SLOT(receiveSyncFinished(Sync::SyncStatus)));

    mOAuth->authenticate();

    LOG_DEBUG ("Init done. Continuing with sync");

    return true;
}

void CalDavClient::abortSync(Sync::SyncStatus aStatus) {
    FUNCTION_CALL_TRACE;
    Sync::SyncStatus state = Sync::SYNC_ABORTED;

    if (aStatus == Sync::SYNC_ERROR) {
        state = Sync::SYNC_CONNECTION_ERROR;
    }

    if( !this->abort (state)) {
        LOG_DEBUG( "Agent not active, aborting immediately" );
        syncFinished(Sync::SYNC_ABORTED);

    }
    else
    {
        LOG_DEBUG( "Agent active, abort event posted" );
    }
}

bool CalDavClient::start() {
    FUNCTION_CALL_TRACE;

    mSettings.setAuthToken(mOAuth->token());

    switch (mSyncDirection)
    {
    case Buteo::SyncProfile::SYNC_DIRECTION_TWO_WAY:
        if (mSlowSync) {
            Report *report = new Report(mNAManager, &mSettings);
            report->getAllEvents();
        } else {
            Report *report = new Report(mNAManager, &mSettings);
            LOG_DEBUG("Report OBject created -------");
            report->getAllETags();
        }

        break;
    case Buteo::SyncProfile::SYNC_DIRECTION_FROM_REMOTE:
        // Not required
        break;
    case Buteo::SyncProfile::SYNC_DIRECTION_TO_REMOTE:
        // Not required
        break;
    case Buteo::SyncProfile::SYNC_DIRECTION_UNDEFINED:
        // Not required
    default:
        // throw configuration error
        break;
    };

    return true;
}

bool CalDavClient::abort(Sync::SyncStatus status) {
    Q_UNUSED(status)
    emit syncFinished (Sync::SYNC_ABORTED);
    return true;
}

bool CalDavClient::cleanUp() {
    FUNCTION_CALL_TRACE;
    return true;
}

void CalDavClient::connectivityStateChanged(Sync::ConnectivityType aType, bool aState) {
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Received connectivity change event:" << aType << " changed to " << aState);
}

bool CalDavClient::initConfig () {

    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Initiating config...");

    mAccountId = 0;
    QString scope = "";
    QStringList accountList = iProfile.keyValues(Buteo::KEY_ACCOUNT_ID);
    QStringList scopeList   = iProfile.keyValues(Buteo::KEY_REMOTE_DATABASE);
    if (!accountList.isEmpty()) {
        QString aId = accountList.first();
        if (aId != NULL) {
            mAccountId = aId.toInt();
        }
    } else {
        return false;
    }

    if (!scopeList.isEmpty()) {
        scope = scopeList.first();
    }
    mOAuth = new OAuthHandler(mAccountId, scope);
    if (!mOAuth->init()) {
        return false;
    }
    connect(mOAuth, SIGNAL(success()), this, SLOT(start()));
    connect(mOAuth, SIGNAL(failed()), this, SLOT(authenticationError()));

    mSettings.setIgnoreSSLErrors(true);
    mSettings.setUrl("https://apidata.googleusercontent.com/caldav/v2/mobilitas123@gmail.com/events/");
    mSettings.setUsername("");
    mSettings.setPassword("");

    mSyncDirection = iProfile.syncDirection();
    mConflictResPolicy = iProfile.conflictResolutionPolicy();

    return true;
}

void CalDavClient::receiveStateChanged(Sync::SyncProgressDetail aState)
{
    FUNCTION_CALL_TRACE;

    switch(aState) {
    case Sync::SYNC_PROGRESS_SENDING_ITEMS: {
        emit syncProgressDetail (getProfileName(), Sync::SYNC_PROGRESS_SENDING_ITEMS);
        break;
    }
    case Sync::SYNC_PROGRESS_RECEIVING_ITEMS: {
        emit syncProgressDetail (getProfileName(), Sync::SYNC_PROGRESS_RECEIVING_ITEMS);
        break;
    }
    case Sync::SYNC_PROGRESS_FINALISING: {
        emit syncProgressDetail (getProfileName(),Sync::SYNC_PROGRESS_FINALISING);
        break;
    }
    default:
        //do nothing
        break;
    };
}

void CalDavClient::receiveSyncFinished(Sync::SyncStatus aState) {
    FUNCTION_CALL_TRACE;

    switch(aState)
    {
        case Sync::SYNC_ERROR:
        case Sync::SYNC_AUTHENTICATION_FAILURE:
        case Sync::SYNC_DATABASE_FAILURE:
        case Sync::SYNC_CONNECTION_ERROR:
        case Sync::SYNC_NOTPOSSIBLE:
        {
            emit error( getProfileName(), "", aState);
            break;
        }
        case Sync::SYNC_ABORTED:
        case Sync::SYNC_DONE:
        {
            emit success( getProfileName(), QString::number(aState));
            break;
        }
        case Sync::SYNC_QUEUED:
        case Sync::SYNC_STARTED:
        case Sync::SYNC_PROGRESS:
        default:
        {
            emit error( getProfileName(), "", aState);
            break;
        }
    }
}

void CalDavClient::authenticationError() {
    emit syncFinished (Sync::SYNC_AUTHENTICATION_FAILURE);
}

const QDateTime CalDavClient::lastSyncTime() {
    FUNCTION_CALL_TRACE;

    Buteo::ProfileManager pm;
    Buteo::SyncProfile* sp = pm.syncProfile (iProfile.name());
    // Without the hack (adding 5 secs), the qttracker engine contact storage
    // time is greater than the sync finish time
    // Because of this, the already added contacts are being sync'd again
    // for consecutive sync's
    if (!sp->lastSuccessfulSyncTime().isNull ())
        return sp->lastSuccessfulSyncTime().addSecs(5);
    else
        return sp->lastSuccessfulSyncTime();
}

Buteo::SyncProfile::SyncDirection CalDavClient::syncDirection ()
{
    FUNCTION_CALL_TRACE;
    return mSyncDirection;
}

Buteo::SyncProfile::ConflictResolutionPolicy CalDavClient::conflictResolutionPolicy ()
{
    FUNCTION_CALL_TRACE;
    return mConflictResPolicy;
}

Buteo::SyncResults
CalDavClient::getSyncResults() const
{
    FUNCTION_CALL_TRACE;

    return mResults;
}

void CalDavClient::replyFinished(QNetworkReply *reply) {
    FUNCTION_CALL_TRACE;

    QByteArray data = reply->readAll();
    if (!data.isNull() && !data.isEmpty()) {
        Reader reader;
        reader.read(data);
        LOG_DEBUG("Total content length of the data = " << data.length());
        QList<CDItem *> items = reader.getIncidenceList();
        mKCal::ExtendedCalendar::Ptr calendar = mKCal::ExtendedCalendar::Ptr (new mKCal::ExtendedCalendar(KDateTime::Spec::LocalZone()));
        mKCal::ExtendedStorage::Ptr storage = calendar->defaultStorage(calendar);
        KCalCore::Incidence::List incidenceList;
        foreach(CDItem *item, items) {
            incidenceList.append(item->incidencePtr());
        }

        KCalCore::Incidence::List list = calendar->addIncidences(&incidenceList, "buteo-cal-" + QString::number(mAccountId));
        calendar->save();

        storage->save();
        storage->close();
        LOG_DEBUG("Total Indicies from Remote = " << incidenceList.count() << "         Total Incidences Saved =  " << list.count() << "\n");
    }
}
