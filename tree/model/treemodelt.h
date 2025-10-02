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

#ifndef TREEMODELT_H
#define TREEMODELT_H

#include "treemodel.h"

class TreeModelT final : public TreeModel {
    Q_OBJECT

public:
    TreeModelT(CSectionInfo& info, CString& separator, int default_unit, QObject* parent = nullptr);
    ~TreeModelT() override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void ResetColor(const QModelIndex& index) override;
    void AckTree(const QJsonObject& obj) override;
    Node* GetNode(const QUuid& node_id) const override;

protected:
    std::pair<int, int> CacheColumnRange() const override { return { std::to_underlying(NodeEnumT::kColor), std::to_underlying(NodeEnumT::kIssuedTime) }; }
    void UpdateStatus(NodeT* node, bool value);
    void RegisterNode(Node* node) override
    {
        node_hash_.insert(node->id, node);
        node_cache_.insert(node->id, node);
    }
    void ResetBranch(Node* node) override;
    void ResetModel() override;

private:
    NodeHash node_cache_ {};
};

#endif // TREEMODELT_H
