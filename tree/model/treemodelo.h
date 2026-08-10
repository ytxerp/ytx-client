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

#ifndef TREEMODELO_H
#define TREEMODELO_H

#include <QDate>

#include "tree/model/treemodel.h"

class TreeModelO final : public TreeModel {
    Q_OBJECT

public:
    TreeModelO(CSectionInfo& info, CString& separator, QObject* parent = nullptr);
    ~TreeModelO() override = default;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

    void ApplyName(const QUuid& node_id, const QString& name, int version) override;

    void InsertSettlement(const QSet<QUuid>& settled_set, const QUuid& settlement_id);
    void RecallSettlement(const QUuid& settlement_id);

    QUuid Partner(QUuid node_id) const { return Value(node_id, &NodeO::partner_id); };
    void HandleStatusChanged(const QUuid& node_id, NodeStatus value);

protected:
    void RegisterNode(Node* node) override;
    void UnregisterNode(Node* node, Node* parent_node) override;

    void InitLeafData() override { };
    void InitTreeData(const QHash<QUuid, Node*>& node_hash, QHash<QUuid, QString>& leaf_path, QHash<QUuid, QString>& branch_path) override;

    QSet<QUuid> UpdateAncestorTotal(Node* node, const node::Delta& delta) const override;
    void InitAncestorTotal(Node* node, const node::Delta& delta) const override;
};

#endif // TREEMODELO_H
