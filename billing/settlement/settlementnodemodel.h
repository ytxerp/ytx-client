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

#ifndef SETTLEMENTNODEMODEL_H
#define SETTLEMENTNODEMODEL_H

#include <QAbstractItemModel>

#include "component/info.h"
#include "component/using.h"
#include "settlement.h"

class SettlementNodeModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    SettlementNodeModel(CSectionInfo& info, int status, CUuid& settlement_id, QObject* parent = nullptr);
    ~SettlementNodeModel();

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

    void ResetModel(const QJsonArray& entry_array);
    void UpdateStatus(int status);

private:
    void NormalizeBuffer();

private:
    CSectionInfo& info_;

    const QUuid settlement_id_ {};
    int status_ {};

    SettlementNodeList list_ {};
    SettlementNodeList list_cache_ {};

    QSet<QUuid> pending_delete_ {};
    QSet<QUuid> pending_insert_ {};
};

#endif // SETTLEMENTNODEMODEL_H
