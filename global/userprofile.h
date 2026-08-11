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

#pragma once

#include <QString>

#include "workspace/sectionpermissions.h"
#include "workspace/workspacerole.h"

class UserProfile {
public:
    static UserProfile& Instance()
    {
        static UserProfile instance;
        return instance;
    }

    const QString& Username() const { return username_; }
    const QString& Name() const { return name_; }
    workspace::Role WorkspaceRole() const { return role_; }
    section::Permissions SectionPermissions() const { return permissions_; }

    void SetUsername(const QString& value) { username_ = value; }
    void SetName(const QString& value) { name_ = value; }
    void SetWorkspaceRole(workspace::Role value) { role_ = value; }
    void SetSectionPermissions(section::Permissions value) { permissions_ = value; }

    void Reset()
    {
        username_.clear();
        name_.clear();
        role_ = workspace::Role::kGuest;
        permissions_ = {};
    }

    UserProfile(const UserProfile&) = delete;
    UserProfile& operator=(const UserProfile&) = delete;
    UserProfile(UserProfile&&) = delete;
    UserProfile& operator=(UserProfile&&) = delete;

private:
    UserProfile() = default;
    ~UserProfile() = default;

private:
    QString username_ {};
    QString name_ {};
    workspace::Role role_ { workspace::Role::kGuest };
    section::Permissions permissions_ {};
};
