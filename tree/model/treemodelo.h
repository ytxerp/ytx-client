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
    ~TreeModelO() override;

signals:
    // send to NodeModelP
    void SUpdateAmount(const QUuid& node_id, double initial_delta, double final_delta);

public slots:
    void RSyncStatus(const QUuid& node_id, bool value);

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

    void AckTree(const QJsonObject& obj) override;
    void SyncNodeName(const QUuid& node_id, const QString& name, const QJsonObject& meta) override;

    int Status(QUuid node_id) const override { return NodeUtils::Value(node_model_, node_id, &NodeO::status); }
    void SyncNodeStatus(const QUuid& node_id, int status, const QJsonObject& meta) override;

    bool InsertNode(int row, const QModelIndex& parent, Node* node) override;
    QUuid Partner(QUuid node_id) const { return NodeUtils::Value(node_model_, node_id, &NodeO::partner); };

protected:
    void RegisterPath(Node* /*node*/) override { };

    void UpdateName(const QUuid& node_id, CString& new_name) override;
    void RemovePath(Node* node, Node* parent_node) override;

    QSet<QUuid> SyncAncestorTotal(
        Node* node, double initial_delta, double final_delta, double first_delta, double second_delta, double discount_delta) override;
    QSet<QUuid> SyncDeltaImpl(
        const QUuid& node_id, double initial_delta, double final_delta, double first_delta, double second_delta, double discount_delta) override;

    void HandleNode() override;
    void ResetBranch(Node* node) override;
    void ClearModel() override;
    void RegisterNode(Node* node) override
    {
        node_model_.insert(node->id, node);
        node_cache_.insert(node->id, node);
    }
};

#endif // TREEMODELO_H
