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

#ifndef SALEREFERENCE_H
#define SALEREFERENCE_H

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUuid>

#include "component/constant.h"

struct SaleReference {
    QDateTime issued_time {};
    QUuid order_id {};
    QUuid node_id {}; // partner or inventory uuid
    double count {};
    double measure {};
    double unit_price {};
    double unit_discount {};
    double initial {};
    QString description {};

    void ResetState();
    void ReadJson(const QJsonObject& object);
};

inline void SaleReference::ResetState()
{
    issued_time = {};
    order_id = QUuid();
    node_id = QUuid();
    count = 0.0;
    measure = 0.0;
    unit_price = 0.0;
    unit_discount = 0.0;
    initial = 0.0;
    description.clear();
}

inline void SaleReference::ReadJson(const QJsonObject& object)
{
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object[kIssuedTime].toString(), Qt::ISODate);
    if (object.contains(kNodeId))
        node_id = QUuid(object[kNodeId].toString());
    if (object.contains(kOrderId))
        order_id = QUuid(object[kOrderId].toString());
    if (object.contains(kDescription))
        description = object[kDescription].toString();
    if (object.contains(kCount))
        count = object[kCount].toString().toDouble();
    if (object.contains(kMeasure))
        measure = object[kMeasure].toString().toDouble();
    if (object.contains(kInitial))
        initial = object[kInitial].toString().toDouble();
    if (object.contains(kUnitDiscount))
        unit_discount = object[kUnitDiscount].toString().toDouble();
    if (object.contains(kUnitPrice))
        unit_price = object[kUnitPrice].toString().toDouble();
}

using SaleReferenceList = QList<SaleReference*>;

#endif // SALEREFERENCE_H
