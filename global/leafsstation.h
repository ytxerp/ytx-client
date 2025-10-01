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

#ifndef LEAFSSTATION_H
#define LEAFSSTATION_H

#include "table/model/leafmodel.h"

// leaf model signal station

class LeafSStation final : public QObject {
    Q_OBJECT

public:
    static LeafSStation* Instance();
    void RegisterModel(const QUuid& node_id, const LeafModel* model);
    void DeregisterModel(const QUuid& node_id);

    void Clear() { model_hash_.clear(); }

    LeafSStation(const LeafSStation&) = delete;
    LeafSStation& operator=(const LeafSStation&) = delete;
    LeafSStation(LeafSStation&&) = delete;
    LeafSStation& operator=(LeafSStation&&) = delete;

signals:
    // send to TableModel
    void SAppendOneEntry(Entry* entry);
    void SRemoveOneEntry(const QUuid& entry_id);

    void SUpdateBalance(const QUuid& entry_id);
    void SSyncRule(bool rule);
    void SCheckAction();

    void SAppendMultiEntry(const EntryList& entry_list);
    void SRemoveMultiEntry(const QSet<QUuid>& entry_id_set);

    void SRefreshField(const QUuid& entry_id, int start, int end);

public slots:
    void RAppendOneEntry(const QUuid& node_id, Entry* entry);
    void RRemoveOneEntry(const QUuid& node_id, const QUuid& entry_id);

    void RSyncRule(const QUuid& node_id, bool rule);
    void RCheckAction(const QUuid& node_id);
    void RUpdateBalance(const QUuid& node_id, const QUuid& entry_id);

    // receive from EntryHub
    void RRemoveEntryHash(const QHash<QUuid, QSet<QUuid>>& leaf_entry);
    void RRemoveMultiEntry(const QUuid& node_id, const QSet<QUuid>& entry_id_set);
    void RAppendMultiEntry(const QUuid& node_id, const EntryList& entry_list);

    void RRefreshField(const QUuid& node_id, const QUuid& entry_id, int start, int end);

private:
    explicit LeafSStation(QObject* parent = nullptr);
    ~LeafSStation() = default;

    const LeafModel* FindModel(const QUuid& node_id) const
    {
        auto it = model_hash_.constFind(node_id);
        if (it == model_hash_.constEnd())
            return nullptr;

        return it.value();
    }

private:
    QHash<QUuid, const LeafModel*> model_hash_ {};
};

#endif // LEAFSSTATION_H
