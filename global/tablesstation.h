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

#ifndef TABLESSTATION_H
#define TABLESSTATION_H

#include "table/model/tablemodel.h"

// table model signal station

class TableSStation final : public QObject {
    Q_OBJECT

public:
    static TableSStation* Instance();
    void RegisterModel(const QUuid& node_id, const TableModel* model);
    void DeregisterModel(const QUuid& node_id);

    void Reset() { model_hash_.clear(); }

    TableSStation(const TableSStation&) = delete;
    TableSStation& operator=(const TableSStation&) = delete;
    TableSStation(TableSStation&&) = delete;
    TableSStation& operator=(TableSStation&&) = delete;

signals:
    // send to TableModel
    void SAppendOneEntry(Entry* entry);
    void SRemoveOneEntry(const QUuid& entry_id);

    void SUpdateBalance(const QUuid& entry_id);
    void SDirectionRule(bool rule);
    void SRefreshStatus();

    void SAppendMultiEntry(const EntryList& entry_list);
    void SRemoveMultiEntry(const QSet<QUuid>& entry_id_set);

    void SRefreshField(const QUuid& entry_id, int start, int end);

public slots:
    void RAppendOneEntry(const QUuid& node_id, Entry* entry);
    void RRemoveOneEntry(const QUuid& node_id, const QUuid& entry_id);

    void RDirectionRule(const QUuid& node_id, bool rule);
    void RRefreshStatus(const QSet<QUuid>& affected_node);
    void RUpdateBalance(const QUuid& node_id, const QUuid& entry_id);

    // receive from EntryHub
    void RRemoveEntryHash(const QHash<QUuid, QSet<QUuid>>& entry_hash);
    void RRemoveMultiEntry(const QUuid& node_id, const QSet<QUuid>& entry_id_set);
    void RAppendMultiEntry(const QUuid& node_id, const EntryList& entry_list);

    void RRefreshField(const QUuid& node_id, const QUuid& entry_id, int start, int end);

private:
    explicit TableSStation(QObject* parent = nullptr);
    ~TableSStation() override { };

    const TableModel* FindModel(const QUuid& node_id) const
    {
        auto it = model_hash_.constFind(node_id);
        if (it == model_hash_.constEnd())
            return nullptr;

        return it.value();
    }

private:
    QHash<QUuid, const TableModel*> model_hash_ {};
};

#endif // TABLESSTATION_H
