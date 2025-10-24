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

#ifndef LEAFMODELO_H
#define LEAFMODELO_H

#include "entryhub/entryhubo.h"
#include "entryhub/entryhubp.h"
#include "leafmodel.h"
#include "tree/model/treemodel.h"
#include "tree/model/treemodeli.h"

class LeafModelO final : public LeafModel {
    Q_OBJECT

public:
    LeafModelO(CLeafModelArg& arg, const Node* node, TreeModel* tree_model_inventory, EntryHub* entry_hub_partner, QObject* parent = nullptr);
    ~LeafModelO() override = default;

public slots:
    void RSyncStatus(const QUuid& node_id, bool value);
    void RSyncPartner(const QUuid& node_id, const QUuid& value);
    void RSaveOrder();

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int, const QModelIndex& parent = QModelIndex()) override;

    const QList<EntryShadow*>& GetEntryShadowList() { return shadow_list_; }

private:
    bool UpdateRate(EntryShadow* entry_shadow, double value) override;
    bool UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row) override;

    bool UpdateExternalItem(EntryShadowO* entry_shadow, const QUuid& value);
    bool UpdateDiscountPrice(EntryShadow* entry_shadow, double value);
    bool UpdateSecond(EntryShadow* entry_shadow, double value, int kCoefficient);
    bool UpdateFirst(EntryShadow* entry_shadow, double value, int kCoefficient);
    void PurifyEntryShadow();

    void CrossSearch(EntryShadow* entry_shadow, const QUuid& item_id, bool is_internal) const;

private:
    TreeModelI* tree_model_i_ {};
    EntryHubP* entry_hub_partner_ {};
    EntryHubO* entry_hub_order_ {};
    QUuid partner_id_ {};
    int node_status_ {};
};

#endif // LEAFMODELO_H
