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
    TreeModelO(CSectionInfo& info, CString& separator, int default_unit, QObject* parent = nullptr);
    ~TreeModelO() override = default;

signals:
    // send to NodeModelP
    void SUpdateAmount(const QUuid& node_id, double initial_delta);

public slots:
    void RNodeStatus(const QUuid& node_id, NodeStatus value);

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

    void AckTree(const QJsonObject& obj) override;
    void AckNode(const QJsonObject& leaf_obj, const QUuid& ancestor_id) override;
    void SyncNodeName(const QUuid& node_id, const QString& name) override;

    void InsertSettlement(const QUuid& node_id, const QUuid& settlement_id);
    void RecallSettlement(const QUuid& settlement_id);
    void DeleteSettlement(const QUuid& settlement_id);

    int Status(QUuid node_id) const override { return NodeUtils::Value(node_hash_, node_id, &NodeO::status); }

    bool InsertNode(int row, const QModelIndex& parent, Node* node) override;
    QUuid Partner(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &NodeO::partner_id); };

protected:
    void RegisterPath(Node* /*node*/) override { };

    void UpdateName(const QUuid& node_id, CString& new_name) override;
    void RemovePath(Node* node, Node* parent_node) override;

    void HandleNode() override;
    void ResetBranch(Node* node) override;
    void ClearModel() override;

private:
    QSet<QUuid> UpdateAncestorTotalOrder(Node* node, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta);
};

#endif // TREEMODELO_H
