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

#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

#include "component/constant.h"

namespace settlement_view {

struct Row final {
    QUuid partner_id {};
    QList<double> values {};

    void Reset() { *this = Row {}; }
    void ReadJson(const QJsonObject& object);
};

struct Column final {
    QString title {};
    int value_index { -1 };
};

inline void Row::ReadJson(const QJsonObject& object)
{
    partner_id = QUuid { object.value(kPartnerId).toString() };

    const auto array { object.value(kValues).toArray() };

    values.reserve(array.size());

    for (const auto& value : array)
        values.append(value.toDouble());
}

}