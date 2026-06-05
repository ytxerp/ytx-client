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

#ifndef CASHFLOWSPECIALMODEL_H
#define CASHFLOWSPECIALMODEL_H

#include <QAbstractItemModel>

#include "cashflowstatementrow.h"
#include "component/using.h"

class CashFlowSpecialModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit CashFlowSpecialModel(const QStringList& header, QObject* parent = nullptr);
    ~CashFlowSpecialModel() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void ResetModel(CJsonArray& special_array);

private:
    CashFlowStatementRow* GetNodeByIndex(const QModelIndex& index) const;
    CashFlowStatementRow* CreateBranchNode(const QString& name, finance::Roles roles, bool direction_rule) const;

    void InitFixedNodes();
    QList<CashFlowStatementRow*> AddRowsList(const CJsonArray& node_array);

    void BuildCounterPartHierarchy() const;

private:
    const QStringList& header_;
    CashFlowStatementRow* root_ {};
    CashFlowStatementRow* special_ {};

    QList<CashFlowStatementRow*> special_list_ {};
};

#endif // CASHFLOWSPECIALMODEL_H
