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

#ifndef ORDERREFERENCE_H
#define ORDERREFERENCE_H

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUuid>

#include "component/constant.h"

struct OrderReference final {
    QDateTime issued_time {};
    QUuid order_id {};
    QUuid node_id {}; // partner or inventory id
    double count {};
    double measure {};
    double unit_price {};
    double initial {};
    QString description {};

    void Reset();
    void ReadJson(const QJsonObject& object);
};

inline void OrderReference::Reset() { *this = OrderReference {}; }

inline void OrderReference::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kIssuedTime); val.isString())
        issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kNodeId); val.isString())
        node_id = QUuid(val.toString());
    if (const auto val = object.value(kOrderId); val.isString())
        order_id = QUuid(val.toString());
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (const auto val = object.value(kCount); val.isString())
        count = val.toString().toDouble();
    if (const auto val = object.value(kMeasure); val.isString())
        measure = val.toString().toDouble();
    if (const auto val = object.value(kInitial); val.isString())
        initial = val.toString().toDouble();
    if (const auto val = object.value(kUnitPrice); val.isString())
        unit_price = val.toString().toDouble();
}

#endif // ORDERREFERENCE_H
