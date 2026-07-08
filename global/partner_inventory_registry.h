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

#ifndef PARTNER_INVENTORY_REGISTRY_H
#define PARTNER_INVENTORY_REGISTRY_H

#include <QHash>
#include <QUuid>
#include <optional>

// NOTE: This is a pure lookup service for "partner + inventory (product)"
// price data, used by Order (TableModelO) to quickly resolve a unit price
// and external SKU when a product is selected on an order line, and
// updated by Partner (TableModelP) whenever a customer-specific price is
// created, edited, or removed.
//
// This class holds no object lifetime responsibilities (unlike EntryHub,
// which owns Entry* objects and their recycling) — it only caches plain
// value data for fast lookup. As such it is implemented as a singleton:
// the application is single-account/single-workspace, so there is no need
// to keep multiple isolated instances of this data.
//
// Call Clear() on logout / full data reload to avoid stale data leaking
// into a new session.
class PartnerInventoryRegistry {
public:
    static PartnerInventoryRegistry& Instance()
    {
        static PartnerInventoryRegistry instance;
        return instance;
    }

    std::optional<double> UnitPrice(const QUuid& partner_id, const QUuid& inventory_id) const
    {
        const auto it { map_.constFind({ partner_id, inventory_id }) };
        if (it == map_.constEnd())
            return std::nullopt;

        return it->unit_price;
    }

    QUuid ExternalSku(const QUuid& partner_id, const QUuid& inventory_id) const
    {
        const auto it { map_.constFind({ partner_id, inventory_id }) };
        return it == map_.constEnd() ? QUuid() : it->external_sku;
    }

    void Insert(const QUuid& partner_id, const QUuid& inventory_id, double unit_price, const QUuid& external_sku)
    {
        map_.insert({ partner_id, inventory_id }, Value { unit_price, external_sku });
    }

    void ReplaceExternalSku(const QUuid& old_id, const QUuid& new_id)
    {
        for (auto& value : map_) {
            if (value.external_sku == old_id)
                value.external_sku = new_id;
        }
    }

    void ReplaceInternalSku(const QUuid& old_id, const QUuid& new_id)
    {
        QVector<Value> values {};
        QVector<std::pair<QUuid, QUuid>> keys {};

        for (auto it = map_.begin(); it != map_.end();) {
            if (it.key().second != old_id) {
                ++it;
                continue;
            }

            auto key = it.key();
            key.second = new_id;

            keys.push_back(key);
            values.push_back(it.value());

            it = map_.erase(it);
        }

        for (int i = 0; i < keys.size(); ++i)
            map_.insert(keys[i], values[i]);
    }

    void Remove(const QUuid& partner_id, const QUuid& inventory_id) { map_.remove({ partner_id, inventory_id }); }
    void Reset() { map_.clear(); }

    PartnerInventoryRegistry(const PartnerInventoryRegistry&) = delete;
    PartnerInventoryRegistry& operator=(const PartnerInventoryRegistry&) = delete;
    PartnerInventoryRegistry(PartnerInventoryRegistry&&) = delete;
    PartnerInventoryRegistry& operator=(PartnerInventoryRegistry&&) = delete;

private:
    PartnerInventoryRegistry() = default;
    ~PartnerInventoryRegistry() = default;

    struct Value {
        double unit_price { 0.0 };
        QUuid external_sku {};
    };

private:
    QHash<std::pair<QUuid, QUuid>, Value> map_ {};
};

#endif // PARTNER_INVENTORY_REGISTRY_H