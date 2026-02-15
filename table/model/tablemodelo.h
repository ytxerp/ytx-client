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

/*
 * TableModelO design notes:
 *
 * 1. New entries (inserted into a draft order) are cached locally in `pending_inserts_`.
 *    - These entries are NOT immediately sent to the server.
 *    - When the user clicks "Save" or "Release", all pending inserts are processed:
 *        a) Entries with null rhs_node (internal SKU not selected) are removed.
 *        b) Remaining entries are serialized into JSON and sent as a batch.
 *    - This ensures that incomplete or invalid entries are never sent to the server.
 *
 * 2. Updates to existing entries (already persisted) are cached in `pending_updates_`.
 *    - These updates use a debounced timer to delay sending changes, preventing excessive network traffic.
 *    - The timer restarts on subsequent edits to the same entry, ensuring that only the latest state is sent.
 *
 * 3. Deletions:
 *    - If the entry is in `pending_inserts_`, deletion is local only (no server message needed).
 *    - If the entry has already been persisted, the debounced timer is stopped, and a delete message is sent to the server.
 *
 * 4. Save button behavior:
 *    - Only affects new, unpersisted entries (drafts).
 *    - Existing entries are synchronized automatically via the debounced timer; no manual save required.
 *
 * 5. PurifyEntry ensures that only valid new entries are saved.
 *
 * This design cleanly separates new inserts from updates, ensures safe multi-client synchronization,
 * and prevents sending incomplete or redundant data to the server.
 */

class TableModelO final : public TableModel {
    Q_OBJECT

public:
    TableModelO(CTableModelArg& arg, TreeModel* tree_model_inventory, EntryHub* entry_hub_partner, QObject* parent = nullptr);
    ~TableModelO() override;

signals:
    // send to TableWidgetO
    void SSyncDeltaO(const QUuid& node_id, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta);

public slots:
    void RAppendMultiEntries(const EntryList& entry_list) override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    inline int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return entry_list_.size(); }
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    bool insertRows(int row, int /*count*/, const QModelIndex& parent) override;
    bool removeRows(int row, int, const QModelIndex& parent = QModelIndex()) override;

    const QList<Entry*>& GetEntryList() const { return entry_list_; }
    void Finalize(QJsonObject& message);
    bool HasPendingUpdate() const { return !pending_insert_.isEmpty() || !pending_delete_.isEmpty() || !pending_update_.isEmpty(); }
    void SetNode(const NodeO* node) { d_node_ = node; }
    inline Entry* GetEntry(const QModelIndex& index) const override { return entry_list_.at(index.row()); }

private:
    bool UpdateInternalSku(EntryO* entry, int row, const QUuid& value, bool is_persisted);
    bool UpdateUnitPrice(EntryO* entry, int row, double value, bool is_persisted);
    bool UpdateUnitDiscount(EntryO* entry, int row, double value, bool is_persisted);
    bool UpdateMeasure(EntryO* entry, int row, double value, bool is_persisted);
    bool UpdateCount(EntryO* entry, double value, bool is_persisted);
    bool UpdateDescription(EntryO* entry, const QString& value, bool is_persisted);

    void ResolveFromInternal(EntryO* entry, const QUuid& internal_sku) const;
    void RecalculateAmount(EntryO* entry) const;

    void PurifyEntry();
    void NormalizeEntryBuffer();

private:
    TreeModelI* tree_model_i_ {};
    EntryHubP* entry_hub_p_ {};
    const NodeO* d_node_ {};

    QList<Entry*> entry_list_ {};

    QSet<QUuid> pending_delete_ {};
    QHash<QUuid, Entry*> pending_insert_ {};
    QHash<QUuid, Entry*> pending_update_ {};
};

#endif // TABLEMODELO_H
