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

#ifndef ENTRYHUBP_H
#define ENTRYHUBP_H

#include "component/using.h"
#include "entryhub.h"

class EntryHubP final : public EntryHub {
    Q_OBJECT

public:
    explicit EntryHubP(CSectionInfo& info, QObject* parent = nullptr);

public:
    std::optional<double> UnitPrice(const QUuid& partner_id, const QUuid& internal_sku) const;
    QUuid ExternalSku(const QUuid& partner_id, const QUuid& internal_sku) const;

    void SearchEntry(QList<Entry*>& entry_list, CString& name) const;
    void PushEntry(const QUuid& node_id);

    void InsertEntry(const QJsonObject& data) override;
    void RemoveEntry(const QUuid& entry_id) override;
    void UpdateEntry(const QUuid& id, const QJsonObject& update) override;

    void RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) override;
    void ApplyPartnerEntry(const QJsonArray& array);
    // std::optional<std::pair<QUuid, double>> ResolveFromExternal(const QUuid& partner_id, const QUuid& external_sku) const;

protected:
    void ApplyInventoryReplace(const QUuid& old_item_id, const QUuid& new_item_id) const override;

private:
    struct EntryValue {
        double unit_price { 0.0 };
        QUuid external_sku {};
    };

    QHash<std::pair<QUuid, QUuid>, EntryValue> entry_map_ {};
};

#endif // ENTRYHUBP_H
