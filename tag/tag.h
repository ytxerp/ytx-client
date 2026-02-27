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

#ifndef TAG_H
#define TAG_H

#include <QJsonObject>
#include <QString>
#include <QUuid>

#include "component/constant.h"
#include "enum/syncenum.h"

struct Tag final {
    QUuid id {};
    QString name {};
    QString color {};
    int version {};

    // state is local only, not serialized
    SyncState state { SyncState::kNew };

    void Reset();
    QJsonObject WriteJson() const;
    void ReadJson(const QJsonObject& object);
};

inline void Tag::Reset() { *this = Tag {}; }

inline QJsonObject Tag::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kName, name);
    obj.insert(kColor, color);

    // version is managed by the server, not written to json
    return obj;
}

inline void Tag::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kName); val.isString())
        name = val.toString();
    if (const auto val = object.value(kColor); val.isString())
        color = val.toString();
    if (const auto val = object.value(kVersion); val.isDouble())
        version = val.toInt();
}

#endif // TAG_H
