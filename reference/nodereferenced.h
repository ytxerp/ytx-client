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

#ifndef NODEREFERENCED_H
#define NODEREFERENCED_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <QUuid>

struct NodeReferenced {
    QDateTime issued_time {};
    QUuid order_id {};
    int section {};
    QUuid node_id {}; // partner or inventory uuid
    QUuid external_sku {};
    double count {};
    double measure {};
    double unit_price {};
    double unit_discount {};
    double initial {};
    QString description {};

    void ResetState();
};

inline void NodeReferenced::ResetState()
{
    issued_time = {};
    order_id = QUuid();
    section = 0;
    node_id = QUuid();
    external_sku = QUuid();
    count = 0.0;
    measure = 0.0;
    unit_price = 0.0;
    unit_discount = 0.0;
    initial = 0.0;
    description.clear();
}

using NodeReferencedList = QList<NodeReferenced*>;

#endif // NODEREFERENCED_H
