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

#ifndef TRANSMODELO_H
#define TRANSMODELO_H

#include "database/sql/sqlo.h"
#include "database/sql/sqls.h"
#include "transmodel.h"
#include "tree/model/nodemodel.h"
#include "tree/model/nodemodelp.h"

class TransModelO final : public TransModel {
    Q_OBJECT

public:
    TransModelO(CTransModelArg& arg, const Node* node, CNodeModel* product_tree, Sql* sqlite_stakeholder, QObject* parent = nullptr);
    ~TransModelO() override = default;

public slots:
    void RSyncBoolWD(const QUuid& node_id, int column, bool value) override; // kFinished, kRule
    void RSyncInt(const QUuid& node_id, int column, const QUuid& value) override; // node_id, party_id

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool removeRows(int row, int, const QModelIndex& parent = QModelIndex()) override;

    const QList<TransShadow*>& GetTransShadowList() { return trans_shadow_list_; }

private:
    bool UpdateInsideProduct(TransShadow* trans_shadow, const QUuid& value);
    bool UpdateOutsideProduct(TransShadow* trans_shadow, const QUuid& value);

    bool UpdateUnitPrice(TransShadow* trans_shadow, double value);
    bool UpdateDiscountPrice(TransShadow* trans_shadow, double value);
    bool UpdateSecond(TransShadow* trans_shadow, double value, int kCoefficient);
    bool UpdateFirst(TransShadow* trans_shadow, double value, int kCoefficient);
    void PurifyTransShadow(const QUuid& lhs_node_id = QUuid());

    void CrossSearch(TransShadow* trans_shadow, const QUuid& product_id, bool is_inside) const;

    void UpdateLhsNode(const QUuid& node_id);
    void UpdateParty(const QUuid& node_id, const QUuid& party_id);
    void UpdateRule(const QUuid& node_id, bool value);

private:
    const NodeModelP* product_tree_ {};
    SqlS* sqlite_stakeholder_ {};
    SqlO* sqlite_order_ {};
    const Node* node_ {};
    QUuid party_id_ {};
};

#endif // TRANSMODELO_H
