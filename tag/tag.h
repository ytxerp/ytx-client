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

struct Tag {
    QUuid id {};
    QString name {};
    QString color {};
    int version {};
    bool is_new {}; // is_new: client-only state, not persisted, not synced

    void ResetState();
    QJsonObject WriteJson() const;
};

inline void Tag::ResetState()
{
    id = QUuid();
    name = {};
    color = {};
    version = 0;
    is_new = false;
}

inline QJsonObject Tag::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kName, name);
    obj.insert(kColor, color);
    return obj;
}

#endif // TAG_H
