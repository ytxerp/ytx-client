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

#include <QAbstractItemModel>
#include <QJsonObject>

#include "tree/model/treemodel.h"

class TreeModelR final : public TreeModel {
    Q_OBJECT

public:
    explicit TreeModelR(Section section, const QStringList& header, CString& separator, QObject* parent = nullptr);
    ~TreeModelR() override = default;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

    void SyncTree(const QJsonObject& data) override;
    void UpdateName(QUuid node_id, const QString& name, int version) override;
    void HandleStatusChanged(QUuid node_id, ReimbursementStatus value);

protected:
    void RegisterNode(Node* node) override;
    void UnregisterNode(Node* node, Node* parent_node) override;
    void UpdateSubtreePath(const Node* node) override;

    void InitTreeData(const QHash<QUuid, Node*>& node_hash, QHash<QUuid, QString>& leaf_path, QHash<QUuid, QString>& branch_path) override;
    void ClearTree() override;

    QSet<QUuid> UpdateAncestorTotal(Node* node, const node::Delta& delta) const override;
    void InitAncestorTotal(Node* node, const node::Delta& delta) const override;
};
