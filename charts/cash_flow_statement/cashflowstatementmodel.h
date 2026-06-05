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

class CashFlowStatementModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit CashFlowStatementModel(const QStringList& header, QObject* parent = nullptr);
    ~CashFlowStatementModel() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void ResetModel(CJsonArray& node_array);

private:
    CashFlowStatementRow* GetNodeByIndex(const QModelIndex& index) const;
    CashFlowStatementRow* CreateBranchNode(const QString& name, finance::CashKind cash_kind, bool direction_rule) const;

    void InitFixedNodes();
    QList<CashFlowStatementRow*> AddRowsList(const CJsonArray& node_array);

    void BuildHierarchy() const;

    CashFlowStatementRow* FindNodeGroup(finance::CashKind kind) const;

    void UpdateAncestorTotal(CashFlowStatementRow* node, double final_delta) const;

private:
    const QStringList& header_;
    CashFlowStatementRow* root_ {};

    QList<CashFlowStatementRow*> first_level_nodes_ {};
    QList<CashFlowStatementRow*> second_level_nodes_ {};

    CashFlowStatementRow* operating_in_ {};
    CashFlowStatementRow* operating_out_ {};
    CashFlowStatementRow* investing_in_ {};
    CashFlowStatementRow* investing_out_ {};
    CashFlowStatementRow* financing_in_ {};
    CashFlowStatementRow* financing_out_ {};

    QList<CashFlowStatementRow*> node_list_ {};
};

#endif // CASHFLOWSTATEMENTMODEL_H
