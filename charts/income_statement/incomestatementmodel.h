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

#ifndef INCOMESTATEMENTMODEL_H
#define INCOMESTATEMENTMODEL_H

#include <QAbstractItemModel>

#include "incomestatementrow.h"

class IncomeStatementModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit IncomeStatementModel(const QStringList& header, QObject* parent = nullptr);
    ~IncomeStatementModel() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void ResetModel(const QJsonArray& node_array, const QJsonArray& path_array, double net_profit, double yoy_net_profit, double mom_net_profit);
    void UpdateHeaderTooltip(const QString& yoy_tooltip, const QString& mom_tooltip);

private:
    IncomeStatementRow* GetNodeByIndex(const QModelIndex& index) const;
    void BuildHierarchy(const QJsonArray& path_array);

    void InitFixedNodes();
    IncomeStatementRow* CreateBranchNode(const QString& name, bool direction_rule) const;

private:
    QStringList header_;
    IncomeStatementRow* root_ {};
    IncomeStatementRow* net_profit_ {};
    QHash<QUuid, IncomeStatementRow*> node_hash_ {};
    QString yoy_tooltip_ {};
    QString mom_tooltip_ {};
};

#endif // INCOMESTATEMENTMODEL_H
