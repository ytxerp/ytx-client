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

#include "entryhub.h"

class EntryHubP final : public EntryHub {
    Q_OBJECT

public:
    EntryHubP(CSectionInfo& info, QObject* parent = nullptr);

public:
    bool CrossSearch(EntryShadowO* order_entry_shadow, const QUuid& partner_id, const QUuid& item_id, bool is_internal) const;
    void RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) override;
    // void ApplyEntryRate(const QUuid& entry_id, const QJsonObject& data, bool is_parallel) override;

protected:
    // table
    void ApplyInventoryReplace(const QUuid& old_item_id, const QUuid& new_item_id) const override;
    QString QSReadTransRef(int unit) const override;

private:
    // void ReadTransS(QSqlQuery& query);
    bool ReadTransRange(const QSet<QUuid>& set);
};

#endif // ENTRYHUBP_H
