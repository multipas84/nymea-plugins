/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 - 2019 Simon Stürz <simon.stuerz@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef HUEDEVICE_H
#define HUEDEVICE_H

#include <QObject>
#include <QDebug>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QJsonDocument>

class HueDevice : public QObject
{
    Q_OBJECT
public:
    explicit HueDevice(QObject *parent = 0);

    int id() const;
    void setId(const int &id);

    QString name() const;
    void setName(const QString &name);

    QHostAddress hostAddress() const;
    void setHostAddress(const QHostAddress hostAddress);

    QString uuid();
    void setUuid(const QString &uuid);

    QString apiKey() const;
    void setApiKey(const QString &apiKey);

    QString modelId() const;
    void setModelId(const QString &modelId);

    QString type() const;
    void setType(const QString &type);

    QString softwareVersion() const;
    void setSoftwareVersion(const QString &softwareVersion);

    bool reachable() const;
    void setReachable(const bool &reachable);

    static QString getBaseUuid(const QString &uuid);

private:
    int m_id;
    QString m_name;
    QHostAddress m_hostAddress;
    QString m_apiKey;
    QString m_modelId;
    QString m_uuid;
    QString m_type;
    QString m_softwareVersion;

    bool m_reachable;

signals:
    void reachableChanged(bool reachable);

};

#endif // HUEDEVICE_H
