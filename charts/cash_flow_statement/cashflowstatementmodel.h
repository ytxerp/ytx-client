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

    void ResetModel(CJsonArray& operating_node, CJsonArray& operating_path, CJsonArray& investing_node, CJsonArray& investing_path, CJsonArray& finance_node,
        CJsonArray& finance_path);

private:
    CashFlowStatementRow* GetNodeByIndex(const QModelIndex& index) const;
    CashFlowStatementRow* CreateBranchNode(const QString& name, finance::CashKind cash_kind, bool direction_rule);

    void InitFixedNodes();
    QHash<QUuid, CashFlowStatementRow*> AddServerRows(const CJsonArray& node_array, const CJsonArray& path_array);
    void BuildHierarchy(const QHash<QUuid, CashFlowStatementRow*>& hash, CJsonArray& path_array);
    CashFlowStatementRow* FindGroupNode(finance::CashKind kind) const;

private:
    const QStringList& header_;
    CashFlowStatementRow* root_ {};

    QList<CashFlowStatementRow*> fixed_nodes_ {};
    QList<CashFlowStatementRow*> cash_flow_group_nodes_ {};
    QHash<QUuid, CashFlowStatementRow*> node_hash_ {};
};

#endif // CASHFLOWSTATEMENTMODEL_H
