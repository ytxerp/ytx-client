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

#ifndef ENTRYREF_H
#define ENTRYREF_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <QUuid>

struct EntryRef {
    QDateTime issued_time {};
    QUuid order_id {};
    int section {};
    QUuid pi_id {}; // party or item uuid
    QUuid external_sku {};
    double count {};
    double measure {};
    double unit_price {};
    double discount_price {};
    double initial {};
    QString description {};

    void ResetState();
};

inline void EntryRef::ResetState()
{
    issued_time = {};
    order_id = QUuid();
    section = 0;
    pi_id = QUuid();
    external_sku = QUuid();
    count = 0.0;
    measure = 0.0;
    unit_price = 0.0;
    discount_price = 0.0;
    initial = 0.0;
    description.clear();
}

using EntryRefList = QList<EntryRef*>;

#endif // ENTRYREF_H
