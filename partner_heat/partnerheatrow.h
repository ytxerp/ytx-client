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

#ifndef PARTNERHEATROW_H
#define PARTNERHEATROW_H

#include <QJsonObject>
#include <QUuid>

struct PartnerHeatRow final {
    QUuid partner_node {};
    qint64 order_count {};
    qint64 inventory_diversity {};
    qint64 active_months {};
    qint64 active_days {};
    double total_quantity {};
    double heat_score {};

    void ReadJson(const QJsonObject& obj);
    void Reset();
};

inline void PartnerHeatRow::ReadJson(const QJsonObject& obj)
{
    if (const auto val = obj.value("partner_node"); val.isString())
        partner_node = QUuid::fromString(val.toString());

    if (const auto val = obj.value("order_count"); val.isDouble())
        order_count = val.toInteger();

    if (const auto val = obj.value("inventory_diversity"); val.isDouble())
        inventory_diversity = val.toInteger();

    if (const auto val = obj.value("active_months"); val.isDouble())
        active_months = val.toInteger();

    if (const auto val = obj.value("active_days"); val.isDouble())
        active_days = val.toInteger();

    if (const auto val = obj.value("total_quantity"); val.isDouble())
        total_quantity = val.toDouble();

    if (const auto val = obj.value("heat_score"); val.isDouble())
        heat_score = val.toDouble();
}

inline void PartnerHeatRow::Reset() { *this = PartnerHeatRow {}; }

#endif // PARTNERHEATROW_H
