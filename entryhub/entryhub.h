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

#include <QJsonObject>
#include <QObject>

#include "component/info.h"
#include "enum/entryenum.h"
#include "table/entry.h"

class EntryHub : public QObject {
    Q_OBJECT

public:
    virtual ~EntryHub();

protected:
    EntryHub(CSectionInfo& info, QObject* parent = nullptr);

signals:
    // send to LeafSStation; partner and order entries are removed directly
    void SRemoveEntryHash(const QHash<QUuid, QSet<QUuid>>& entry_hash);
    void SRemoveMultiEntry(const QUuid& node_id, const QSet<QUuid>& entry_id_set);
    void SAppendMultiEntry(const QUuid& node_id, const EntryList& entry_list);

    void SAppendOneEntry(const QUuid& node_id, Entry* entry);
    void SRemoveOneEntry(const QUuid& node_id, const QUuid& entry_id);

    void SRefreshField(const QUuid& node_id, const QUuid& entry_id, int start, int end);
    void SRefreshStatus(const QSet<QUuid>& affected_node);

    void SUpdateBalance(const QUuid& node_id, const QUuid& entry_id);

    // send to SearchEntryModel
    void SSearchEntry(const EntryList& entry_list);

public slots:
    // receive from TableModel
    void RInsertEntry(Entry* entry) { entry_cache_.insert(entry->id, entry); }
    void RRemoveEntry(const QUuid& entry_id);

public:
    // tree
    virtual void InsertEntry(const QJsonObject& data);
    virtual void RemoveEntry(const QUuid& entry_id);
    virtual void UpdateEntry(const QUuid& entry_id, const QJsonObject& update);

    void InsertMeta(const QUuid& entry_id, const QJsonObject& meta);
    void UpdateMeta(const QUuid& entry_id, const QJsonObject& meta);

    void UpdateEntryLinkedNode(const QUuid& entry_id, const QJsonObject& update, bool is_parallel);

    // table
    void AckTable(const QUuid& node_id, const QJsonArray& array);
    void ApplyPartnerEntry(const QJsonArray& array);
    void SearchEntry(const QJsonArray& array);

    void ActionEntry(const QUuid& node_id, EntryAction action);
    void ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id);

    virtual void UpdateEntryRate(const QUuid& entry_id, const QJsonObject& update, bool is_parallel)
    {
        Q_UNUSED(entry_id);
        Q_UNUSED(update);
        Q_UNUSED(is_parallel);
    }

    virtual void UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& update)
    {
        Q_UNUSED(entry_id);
        Q_UNUSED(update);
    }

    virtual void ApplyInventoryReplace(const QUuid& old_item_id, const QUuid& new_item_id) const
    {
        Q_UNUSED(old_item_id);
        Q_UNUSED(new_item_id);
    }
    virtual void RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry);

protected:
    virtual EntryList ProcessEntryArray(const QJsonArray& array);

    void ReplaceLeafFunction(QSet<QUuid>& entry_id_set, EntryList& entry_list, const QUuid& old_node_id, const QUuid& new_node_id) const;
    void RemoveLeafFunction(const QHash<QUuid, QSet<QUuid>>& leaf_entry);

    void EntryActionImpl(Entry* entry, EntryAction action);

protected:
    QHash<QUuid, Entry*> entry_cache_ {};
    const Section section_ {};

    CSectionInfo& info_;
};

#endif // ENTRYHUB_H
