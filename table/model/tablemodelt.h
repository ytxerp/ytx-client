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
#include "tree/node.h"

class TableModelT final : public TableModel {
    Q_OBJECT

public:
    TableModelT(CTableModelArg& arg, const Node* node, QObject* parent = nullptr);
    ~TableModelT() override = default;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

protected:
    bool UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row) override;
    bool UpdateNumeric(EntryShadow* entry_shadow, double value, int row, bool is_debit) override;
    // bool UpdateDebit(EntryShadow* entry_shadow, double value, int row) override;
    // bool UpdateCredit(EntryShadow* entry_shadow, double value, int row) override;
    bool UpdateRate(EntryShadow* entry_shadow, double value) override;

    double CalculateBalance(EntryShadow* entry_shadow) override;

private:
    const NodeT* d_node_ {};
};

#endif // TABLEMODELT_H
