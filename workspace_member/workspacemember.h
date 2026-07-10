/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WORKSPACEMEMBER_H
#define WORKSPACEMEMBER_H

#include <QJsonObject>
#include <QString>
#include <QUuid>

#include "component/constant.h"
#include "databaserole.h"
#include "workspacerole.h"

namespace workspace {

struct Member final {
    QUuid id {};
    int version {};

    QString email {};
    QString username {};
    QString name {};

    Role workspace_role { Role::kGuest };
    database::Roles database_roles {};
    QDateTime created_time {};

    void Reset();
    void ReadJson(const QJsonObject& object);
};

inline void Member::Reset() { *this = Member {}; }

inline void Member::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());

    if (const auto val = object.value(kVersion); val.isDouble())
        version = val.toInt();

    if (const auto val = object.value(kName); val.isString())
        name = val.toString();

    if (const auto val = object.value(kUsername); val.isString())
        username = val.toString();

    if (const auto val = object.value(kEmail); val.isString())
        email = val.toString();

    if (const auto val = object.value(kWorkspaceRole); val.isDouble())
        workspace_role = static_cast<workspace::Role>(val.toInt());

    if (const auto val = object.value(kDatabaseRoles); val.isDouble())
        database_roles = database::Roles(val.toInt());

    if (const auto val = object.value(kCreatedTime); val.isString())
        created_time = QDateTime::fromString(val.toString(), Qt::ISODate);
}
}

#endif // WORKSPACEMEMBER_H
