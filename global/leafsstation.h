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

#include "component/enumclass.h"
#include "table/model/transmodel.h"
#include "table/trans.h"

// leaf node signal station

class LeafSStation final : public QObject {
    Q_OBJECT

public:
    static LeafSStation& Instance();
    void RegisterModel(Section section, const QUuid& node_id, const TransModel* model);
    void DeregisterModel(Section section, const QUuid& node_id);

    LeafSStation(const LeafSStation&) = delete;
    LeafSStation& operator=(const LeafSStation&) = delete;
    LeafSStation(LeafSStation&&) = delete;
    LeafSStation& operator=(LeafSStation&&) = delete;

signals:
    // send to TableModel
    void SAppendOneTransL(const TransShadow* trans_shadow);
    void SRemoveOneTransL(const QUuid& node_id, const QUuid& trans_id);
    void SSyncBalance(const QUuid& node_id, const QUuid& trans_id);
    void SSyncRule(const QUuid& node_id, bool rule);
    void SAppendMultiTrans(const QUuid& node_id, const TransShadowList& trans_shadow_list);
    void SRemoveMultiTransL(const QUuid& node_id, const QSet<QUuid>& trans_id_set);
    void SAppendMultiTransL(const QUuid& node_id, const QSet<QUuid>& trans_id_set);

public slots:
    // receive from TableModel
    void RAppendOneTransL(Section section, const TransShadow* trans_shadow);
    void RRemoveOneTransL(Section section, const QUuid& node_id, const QUuid& trans_id);
    void RSyncBalance(Section section, const QUuid& node_id, const QUuid& trans_id);

    // receive from SqliteStakeholder

    // receive from TreeModel
    void RSyncRule(Section section, const QUuid& node_id, bool rule);

    // receive from Sqlite
    void RRemoveMultiTransL(Section section, const QMultiHash<QUuid, QUuid>& leaf_trans);
    void RMoveMultiTransL(Section section, const QUuid& old_node_id, const QUuid& new_node_id, const QSet<QUuid>& trans_id_set);
    void RAppendMultiTrans(Section section, const QUuid& node_id, const TransShadowList& trans_shadow);

private:
    LeafSStation() = default;
    ~LeafSStation() { };

    const TransModel* FindModel(Section section, const QUuid& node_id) const
    {
        auto it = model_hash_.constFind({ section, node_id });
        if (it == model_hash_.constEnd())
            return nullptr;

        return it.value();
    }

private:
    QHash<std::pair<Section, QUuid>, const TransModel*> model_hash_ {};
};

#endif // LEAFSSTATION_H
