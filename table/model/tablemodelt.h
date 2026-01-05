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

#ifndef TABLEMODELT_H
#define TABLEMODELT_H

#include "tablemodel.h"
#include "tree/model/treemodelt.h"

class TableModelT final : public TableModel {
    Q_OBJECT

public:
    TableModelT(CTableModelArg& arg, TreeModelT* tree_model_t, QObject* parent = nullptr);
    ~TableModelT() override = default;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

protected:
    bool UpdateLinkedNode(EntryShadow* shadow, const QUuid& value, int row) override;
    bool UpdateNumeric(EntryShadow* shadow, double value, int row, bool is_debit) override;
    bool UpdateRate(EntryShadow* shadow, double value) override;
    bool IsReleased(const QUuid& lhs_node, const QUuid& rhs_node) const override
    {
        return tree_model_t_->Status(lhs_node) == std::to_underlying(NodeStatus::kReleased)
            || tree_model_t_->Status(rhs_node) == std::to_underlying(NodeStatus::kReleased);
    };

private:
    TreeModelT* tree_model_t_ {};
};

#endif // TABLEMODELT_H
