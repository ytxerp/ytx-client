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

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "component/constant.h"
#include "enum/nodeenum.h"
#include "enum/statusenum.h"

namespace order_overview {

enum class RowField : int {
    kIssuedTime = 0,
    kPartner,
    kCode,
    kInventory,
    kDirectionRule,
    kUnit,
    kStatus,
    kPlaceholder,
    kCount,
    kMeasure,
    kUnitPrice,
    kAmount,
};

struct Row final {
    QDateTime issued_time {};
    QString partner {};
    QString code {};
    QString inventory {};
    bool direction_rule {};
    double count {};
    double measure {};
    double unit_price {};
    double amount {};
    NodeUnit unit {};
    OrderStatus status {};

    inline void Reset() { *this = Row {}; }

    inline void ReadJson(const QJsonObject& object)
    {
        if (const auto val = object.value(kIssuedTime); val.isString())
            issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);

        if (const auto val = object.value(kPartner); val.isString())
            partner = val.toString();

        if (const auto val = object.value(kCode); val.isString())
            code = val.toString();

        if (const auto val = object.value(kInventory); val.isString())
            inventory = val.toString();

        if (const auto val = object.value(kDirectionRule); val.isBool())
            direction_rule = val.toBool();

        if (const auto val = object.value(kCount); val.isString())
            count = val.toString().toDouble();

        if (const auto val = object.value(kMeasure); val.isString())
            measure = val.toString().toDouble();

        if (const auto val = object.value(kUnitPrice); val.isString())
            unit_price = val.toString().toDouble();

        if (const auto val = object.value(kAmount); val.isString())
            amount = val.toString().toDouble();

        if (const auto val = object.value(kUnit); val.isDouble())
            unit = static_cast<NodeUnit>(val.toInt());

        if (const auto val = object.value(kStatus); val.isDouble())
            status = static_cast<OrderStatus>(val.toInt());
    }
};

}