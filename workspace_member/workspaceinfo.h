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

#ifndef WORKSPACEINFO_H
#define WORKSPACEINFO_H

#include <QHash>
#include <QList>
#include <QString>

struct WorkspaceInfo {
    QList<QPair<int, QString>> role_list {};
    QHash<int, QString> role_hash {};
    QList<QPair<QString, QString>> database_role_list {};
    QHash<QString, QString> database_role_hash {};
    QStringList header {};
};

#endif // WORKSPACEINFO_H
