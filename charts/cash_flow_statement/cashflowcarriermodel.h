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

#ifndef CASHFLOWCARRIERMODEL_H
#define CASHFLOWCARRIERMODEL_H

#include <QAbstractItemModel>

#include "cashflowstatementrow.h"
#include "component/using.h"

namespace cash_flow {

class CarrierModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit CarrierModel(const QStringList& header, QObject* parent = nullptr);
    ~CarrierModel() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void ResetModel(CJsonArray& carrier_array, CJsonArray& counterpart_array);

private:
    Row* GetNodeByIndex(const QModelIndex& index) const;
    Row* CreateBranchNode(const QString& name, finance::Roles roles, bool direction_rule) const;

    void InitFixedNodes();
    QList<Row*> AddRowsList(const CJsonArray& node_array);

    void BuildCarrierHierarchy() const;
    void BuildCounterPartHierarchy() const;

    Row* FindCarrierGroup(finance::Roles roles) const;

private:
    const QStringList& header_;
    Row* root_ {};
    Row* counterpart_ {};

    QList<Row*> first_level_nodes_ {};
    QList<Row*> second_level_nodes_ {};

    Row* cash_ {};
    Row* bank_ {};
    Row* wallet_ {};

    QList<Row*> carrier_list_ {};
    QList<Row*> counterpart_list_ {};
};
}

#endif // CASHFLOWCARRIERMODEL_H
