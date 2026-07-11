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

#ifndef CASHFLOWSTATEMENTMODEL_H
#define CASHFLOWSTATEMENTMODEL_H

#include <QAbstractItemModel>

#include "cashflowstatementrow.h"
#include "component/using.h"

namespace cash_flow {

class Model final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit Model(const QStringList& header, QObject* parent = nullptr);
    ~Model() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void Rebuild(CJsonArray& node_array);

private:
    Row* GetNodeByIndex(const QModelIndex& index) const;
    Row* CreateBranchNode(const QString& name, finance::CashKind cash_kind, bool direction_rule) const;

    void InitFixedNodes();
    QList<Row*> AddRowsList(const CJsonArray& node_array);

    void BuildHierarchy() const;

    Row* FindNodeGroup(finance::CashKind kind) const;

    void UpdateAncestorTotal(Row* node, double final_delta) const;

private:
    const QStringList& header_;
    Row* root_ {};

    QList<Row*> first_level_nodes_ {};
    QList<Row*> second_level_nodes_ {};

    Row* operating_in_ {};
    Row* operating_out_ {};
    Row* investing_in_ {};
    Row* investing_out_ {};
    Row* financing_in_ {};
    Row* financing_out_ {};

    QList<Row*> node_list_ {};
};
}

#endif // CASHFLOWSTATEMENTMODEL_H
