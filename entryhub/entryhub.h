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

#pragma once

#include <QJsonObject>
#include <QObject>

#include "enum/bindingmode.h"
#include "enum/entryenum.h"
#include "enum/section.h"
#include "table/entry.h"

class EntryHub : public QObject {
    Q_OBJECT

public:
    ~EntryHub() override;

protected:
    explicit EntryHub(const Section section, QObject* parent = nullptr);

signals:
    // send to LeafSStation
    void SDeleteEntryHash(const QHash<QUuid, QSet<QUuid>>& entry_hash);
    void SDeleteEntries(QUuid node_id, const QSet<QUuid>& entry_id_set);
    void SAppendEntries(QUuid node_id, const EntryList& entry_list);

    void SAttachEntry(QUuid node_id, Entry* entry);
    void SDetachEntry(QUuid node_id, QUuid entry_id, QUuid counter_node_id = {});

    void SRefreshField(QUuid node_id, QUuid entry_id, int start, int end);
    void SRefreshBatchEntries(const QSet<QUuid>& affected_node, int column);

    void SUpdateBalance(QUuid node_id, QUuid entry_id);

public slots:
    // receive from TableModel
    void RTransferEntry(Entry* entry) { entry_cache_.insert(entry->id, entry); }

public:
    void Reset();

    virtual void InsertEntry(const QJsonObject& data);
    virtual void DeleteEntry(QUuid entry_id);
    virtual void UpdateEntry(QUuid id, const QJsonObject& update, int version);
    virtual void UpdateEntryRate(QUuid entry_id, const QJsonObject& update, InputSide input_side, int version);
    virtual void UpdateEntryNumeric(QUuid entry_id, const QJsonObject& update, int version);

    void UpdateVersion(QUuid id, int version);
    void UpdateEntryLinkedNode(QUuid id, const QJsonObject& update, InputSide input_side);
    void UpdateEntryLocked(QUuid entry_id, int version, bool is_locked);
    void RestoreSyncState(QUuid entry_id);

    void AckTable(QUuid node_id, const QJsonArray& array);
    void PeriodClose(const QJsonArray& array);

    void MarkEntries(QUuid node_id, MarkOperation operation);
    void LockEntries(QUuid node_id, bool is_locked);
    void DeleteDoubleLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry);

protected:
    virtual EntryList ProcessEntryArray(const QJsonArray& array);

    void MarkEntry(Entry* entry, MarkOperation operation);

protected:
    QHash<QUuid, Entry*> entry_cache_ {};
    const Section section_ {};
};
