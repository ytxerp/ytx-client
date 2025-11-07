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
    ~TableModelO() override;

signals:
    // send to entryhub
    void SInsertEntryHash(const QHash<QUuid, Entry*>& entry_hash);
    void SRemoveEntrySet(const QSet<QUuid>& entry_set);
    void SUpdateEntryHash(const QHash<QUuid, QJsonObject>& entry_caches);

public slots:
    void RAppendMultiEntry(const EntryList& entry_list) override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    inline int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return entry_list_.size(); }

    bool insertRows(int row, int /*count*/, const QModelIndex& parent) override;
    bool removeRows(int row, int, const QModelIndex& parent = QModelIndex()) override;

    const QList<Entry*>& GetEntryList() { return entry_list_; }
    void SaveOrder(QJsonObject& order_cache);
    bool HasUnsavedData() const { return !deleted_entries_.isEmpty() || !inserted_entries_.isEmpty() || !updated_entries_.isEmpty(); }
    void SetNode(const NodeO* node) { d_node_ = node; }

private:
    bool UpdateInternalSku(EntryO* entry, const QUuid& value);
    bool UpdateUnitPrice(EntryO* entry, double value);
    bool UpdateExternalSku(EntryO* entry, const QUuid& value);
    bool UpdateUnitDiscount(EntryO* entry, double value);
    bool UpdateMeasure(EntryO* entry, double value);
    bool UpdateCount(EntryO* entry, double value);
    bool UpdateDescription(EntryO* entry, const QString& value);

    void ResolveFromInternal(EntryO* entry, const QUuid& internal_sku) const;
    void ResolveFromExternal(EntryO* entry, const QUuid& external_sku) const;
    void RecalculateAmount(EntryO* entry) const;

    void PurifyEntry();
    void NormalizeEntryBuffer();

private:
    TreeModelI* tree_model_i_ {};
    EntryHubP* entry_hub_p_ {};
    const NodeO* d_node_ {};

    QList<Entry*> entry_list_ {};

    QSet<QUuid> deleted_entries_ {};
    QHash<QUuid, Entry*> inserted_entries_ {};
    QHash<QUuid, QJsonObject> updated_entries_ {};
};

#endif // TABLEMODELO_H
