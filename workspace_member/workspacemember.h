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
#include "workspacerole.h"

struct WorkspaceMember final {
    QUuid id {};
    QString email {};
    QString username {};
    QString name {};

    WorkspaceRole workspace_role {};
    QString database_role {};
    QDateTime created_time {};

    void Reset();
    void ReadJson(const QJsonObject& object);
};

inline void WorkspaceMember::Reset()
{
    id = QUuid();
    name.clear();
    username.clear();
    email.clear();
    workspace_role = WorkspaceRole::kGuest;
    database_role.clear();
    created_time = QDateTime();
}

inline void WorkspaceMember::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());

    if (const auto val = object.value(kName); val.isString())
        name = val.toString();

    if (const auto val = object.value(kUsername); val.isString())
        username = val.toString();

    if (const auto val = object.value(kEmail); val.isString())
        email = val.toString();

    if (const auto val = object.value(kWorkspaceRole); val.isDouble())
        workspace_role = static_cast<WorkspaceRole>(val.toInt());

    if (const auto val = object.value(kDatabaseRole); val.isString())
        database_role = val.toString();

    if (const auto val = object.value(kCreatedTime); val.isString())
        created_time = QDateTime::fromString(val.toString(), Qt::ISODate);
}

#endif // WORKSPACEMEMBER_H
