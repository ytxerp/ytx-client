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

#ifndef ENTRYHUB_H
#define ENTRYHUB_H

#include <QObject>

#include "component/enumclass.h"
#include "component/info.h"
#include "report/entryref.h"
#include "table/entry.h"
#include "table/entryshadow.h"

class EntryHub : public QObject {
    Q_OBJECT

public:
    virtual ~EntryHub();

protected:
    EntryHub(CSectionInfo& info, QObject* parent = nullptr);

signals:
    // send to LeafSStation; Stakeholder and order entries are removed directly
    void SRemoveEntryHash(const QHash<QUuid, QSet<QUuid>>& entry_hash);
    void SRemoveMultiEntry(const QUuid& leaf_id, const QSet<QUuid>& entry_id_set);
    void SAppendMultiEntry(const QUuid& leaf_id, const EntryList& entry_list);

    void SAppendOneEntry(const QUuid& leaf_id, Entry* entry);
    void SRemoveOneEntry(const QUuid& leaf_id, const QUuid& entry_id);

    void SRefreshField(const QUuid& leaf_id, const QUuid& entry_id, int start, int end);

    void SUpdateBalance(const QUuid& node_id, const QUuid& entry_id);
    void SCheckAction(const QUuid& leaf_id);

    // send to SearchEntryModel
    void SSearchEntry(const EntryList& entry_list);

public:
    // tree
    void ApplyEntryInsert(const QJsonObject& data);
    void ApplyEntryRemove(const QUuid& entry_id);
    void ApplyEntryUpdate(const QUuid& entry_id, const QJsonObject& data);

    void ApplyMetaInsert(const QUuid& entry_id, const QJsonObject& data);
    void ApplyMetaUpdate(const QUuid& entry_id, const QJsonObject& data);

    void ApplyEntryRhsNode(const QUuid& entry_id, const QUuid& old_rhs_id, const QUuid& new_rhs_id, const QJsonObject& data);

    // table
    void AckLeafTable(const QUuid& leaf_id, const QJsonArray& array);
    void AckEntrySearch(const QJsonArray& array);

    void ApplyCheckAction(const QUuid& leaf_id, Check check, const QJsonObject& meta);
    void ApplyCheckActionMeta(const QUuid& leaf_id, const QJsonObject& meta);
    void ApplyLeafReplace(const QUuid& old_leaf_id, const QUuid& new_leaf_id);

    EntryShadow* AllocateEntryShadow();

    bool ReadTransRef(EntryRefList& list, const QUuid& node_id, int unit, const QDateTime& start, const QDateTime& end) const;

    virtual void ApplyEntryRate(const QUuid& entry_id, const QJsonObject& data, bool is_parallel)
    {
        Q_UNUSED(entry_id);
        Q_UNUSED(data);
        Q_UNUSED(is_parallel);
    }

    virtual void ApplyEntryNumeric(const QUuid& entry_id, const QJsonObject& data, bool is_parallel)
    {
        Q_UNUSED(entry_id);
        Q_UNUSED(data);
        Q_UNUSED(is_parallel);
    }

    virtual void ApplyItemReplace(const QUuid& old_item_id, const QUuid& new_item_id) const
    {
        Q_UNUSED(old_item_id);
        Q_UNUSED(new_item_id);
    }
    virtual void ApplyLeafRemove(const QHash<QUuid, QSet<QUuid>>& leaf_entry);

protected:
    // QS means QueryString
    virtual QString QSReadTransRef(int unit) const
    {
        Q_UNUSED(unit);
        return {};
    };

    virtual std::pair<int, int> CacheColumnRange() const { return { std::to_underlying(EntryEnum::kCode), std::to_underlying(EntryEnum::kIsChecked) }; }
    virtual std::pair<int, int> NumericColumnRange() const { return { std::to_underlying(EntryEnum::kDebit), std::to_underlying(EntryEnum::kBalance) }; }

    void ReplaceLeafFunction(QSet<QUuid>& entry_id_set, EntryList& entry_list, const QUuid& old_node_id, const QUuid& new_node_id) const;
    void RemoveLeafFunction(const QHash<QUuid, QSet<QUuid>>& leaf_entry);

    EntryList ProcessEntryArray(const QJsonArray& array);

protected:
    QHash<QUuid, Entry*> entry_cache_ {};
    const Section section_ {};

    CSectionInfo& info_;
};

#endif // ENTRYHUB_H
