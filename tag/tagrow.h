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

#ifndef TAGROW_H
#define TAGROW_H

#include <QJsonObject>
#include <QString>
#include <QUuid>

#include "component/constant.h"
#include "enum/stateenum.h"

struct TagRow final {
    QUuid id {};
    QString name {};
    QString color {};
    int version {};

    // sync_state is local only, not serialized
    SyncState sync_state { SyncState::kCreating };

    void Reset();
    QJsonObject WriteJson() const;
    void ReadJson(const QJsonObject& object);
};

inline void TagRow::Reset() { *this = TagRow {}; }

inline QJsonObject TagRow::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kName, name);
    obj.insert(kColor, color);
    obj.insert(kVersion, version);

    // version is managed by the server, not written to json
    return obj;
}

inline void TagRow::ReadJson(const QJsonObject& object)
{
    // Data loaded from server
    sync_state = SyncState::kSynced;

    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kName); val.isString())
        name = val.toString();
    if (const auto val = object.value(kColor); val.isString())
        color = val.toString();
    if (const auto val = object.value(kVersion); val.isDouble())
        version = val.toInt();
}

#endif // TAGROW_H
