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

#include "settings.h"

Settings::Settings()
    : mIgnoreSSLErrors(false), mOAuthToken(""),
      mUrl(""), mUsername(""), mPassword("")
{
}

QString Settings::authToken() {
    return mOAuthToken;
}

void Settings::setAuthToken(QString token) {
    mOAuthToken = token;
}

bool Settings::ignoreSSLErrors() {
    return mIgnoreSSLErrors;
}

void Settings::setIgnoreSSLErrors(bool ignore) {
    mIgnoreSSLErrors = ignore;
}

QString Settings::password() {
    return mPassword;
}

void Settings::setPassword(QString password) {
    mPassword = password;
}

QString Settings::username() {
    return mUsername;
}

void Settings::setUsername(QString username) {
    mUsername = username;
}

void Settings::setUrl(QString url) {
    mUrlString = url;
    mUrl.setUrl(url);
}

QString Settings::url() {
    return mUrlString;
}

QUrl Settings::makeUrl() {
    if (!mPassword.isEmpty() && !mUsername.isEmpty()) {
        mUrl.setUserName(mUsername);
        mUrl.setPassword(mPassword);
    }

    return mUrl;
}
