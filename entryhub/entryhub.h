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

#include "component/sectioninfo.h"
#include "enum/bindingmode.h"
#include "enum/entryenum.h"
#include "table/entry.h"

class EntryHub : public QObject {
    Q_OBJECT

public:
    ~EntryHub() override;

protected:
    explicit EntryHub(CSectionInfo& info, QObject* parent = nullptr);

signals:
    // send to LeafSStation
    void SDeleteEntryHash(const QHash<QUuid, QSet<QUuid>>& entry_hash);
    void SDeleteEntries(const QUuid& node_id, const QSet<QUuid>& entry_id_set);
    void SAppendEntries(const QUuid& node_id, const EntryList& entry_list);

    void SAttachEntry(const QUuid& node_id, Entry* entry);
    void SDetachEntry(const QUuid& node_id, const QUuid& entry_id, const QUuid& counter_node_id = {});

    void SRefreshField(const QUuid& node_id, const QUuid& entry_id, int start, int end);
    void SRefreshStatus(const QSet<QUuid>& affected_node);

    void SUpdateBalance(const QUuid& node_id, const QUuid& entry_id);

    // send to SearchEntryModel
    void SSearchEntry(const EntryList& entry_list);

public slots:
    // receive from TableModel
    void RTransferEntry(Entry* entry) { entry_cache_.insert(entry->id, entry); }

public:
    void Reset();

    virtual void InsertEntry(const QJsonObject& data);
    virtual void DeleteEntry(const QUuid& entry_id);
    virtual void UpdateEntry(const QUuid& id, const QJsonObject& update, int version);
    virtual void UpdateEntryRate(const QUuid& entry_id, const QJsonObject& update, InputSide input_side, int version);
    virtual void UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& update, int version);

    void UpdateVersion(const QUuid& id, int version);
    void UpdateEntryLinkedNode(const QUuid& id, const QJsonObject& update, InputSide input_side);

    void AckTable(const QUuid& node_id, const QJsonArray& array);
    void SearchEntry(const QJsonArray& array);
    void PeriodClose(const QJsonArray& array);

    void MarkEntries(const QUuid& node_id, MarkOperation operation);
    void DeleteDoubleLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry);

protected:
    virtual EntryList ProcessEntryArray(const QJsonArray& array);

    void MarkEntry(Entry* entry, MarkOperation operation);

protected:
    QHash<QUuid, Entry*> entry_cache_ {};
    const Section section_ {};

    CSectionInfo& info_;
};
