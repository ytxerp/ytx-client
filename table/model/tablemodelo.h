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

#ifndef TABLEMODELO_H
#define TABLEMODELO_H

#include "entryhub/entryhubp.h"
#include "tablemodel.h"
#include "tree/model/treemodel.h"
#include "tree/model/treemodeli.h"

class TableModelO final : public TableModel {
    Q_OBJECT

public:
    TableModelO(CTableModelArg& arg, TreeModel* tree_model_inventory, EntryHub* entry_hub_partner, QObject* parent = nullptr);
    ~TableModelO() override = default;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool removeRows(int row, int, const QModelIndex& parent = QModelIndex()) override;

    const QList<EntryShadow*>& GetEntryShadowList() { return shadow_list_; }
    void SaveOrder(QJsonObject& order_cache);
    bool HasUnsavedData() const;
    void SetNode(const NodeO* node) { d_node_ = node; }

private:
    bool UpdateRate(EntryShadow* entry_shadow, double value) override;
    bool UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row) override;
    bool CanInsertRows() const override { return d_node_->status == std::to_underlying(NodeStatus::kDraft); }
    void InitShadow(EntryShadow* entry_shadow) const override
    {
        assert(entry_shadow->lhs_node != nullptr);
        *entry_shadow->lhs_node = lhs_id_;
    }

    bool UpdateExternalSku(EntryShadowO* entry_shadow, const QUuid& value);
    bool UpdateUnitDiscount(EntryShadowO* entry_shadow, double value);
    bool UpdateMeasure(EntryShadowO* entry_shadow, double value);
    bool UpdateCount(EntryShadowO* entry_shadow, double value);
    bool UpdateDescription(EntryShadowO* entry_shadow, const QString& value);

    void ResolveFromInternal(EntryShadowO* shadow, const QUuid& internal_sku) const;
    void ResolveFromExternal(EntryShadowO* shadow, const QUuid& external_sku) const;
    void RecalculateAmount(EntryShadowO* shadow) const;

    void PurifyEntryShadow();
    void NormalizeEntryBuffer();

private:
    TreeModelI* tree_model_i_ {};
    EntryHubP* entry_hub_partner_ {};
    const NodeO* d_node_ {};

    QSet<QUuid> deleted_entries_ {};
    QHash<QUuid, EntryShadow*> inserted_entries_ {};
};

#endif // TABLEMODELO_H
