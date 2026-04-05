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

#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QString>

#include "workspace_member/workspacerole.h"

class UserProfile {
public:
    static UserProfile& Instance()
    {
        static UserProfile instance;
        return instance;
    }

    const QString& Username() const { return username_; }
    const QString& Name() const { return name_; }
    WorkspaceRole GetWorkspaceRole() const { return workspacer_role_; }

    void SetUsername(const QString& value) { username_ = value; }
    void SetName(const QString& value) { name_ = value; }
    void SetWorkspaceRole(WorkspaceRole value) { workspacer_role_ = value; }

    void Reset()
    {
        username_.clear();
        name_.clear();
        workspacer_role_ = WorkspaceRole::kGuest;
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
    WorkspaceRole workspacer_role_ { WorkspaceRole::kGuest };
};

#endif // USERPROFILE_H
