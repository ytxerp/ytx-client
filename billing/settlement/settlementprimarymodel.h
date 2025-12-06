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

#ifndef SETTLEMENTPRIMARYMODEL_H
#define SETTLEMENTPRIMARYMODEL_H

#include <QAbstractItemModel>

#include "billing/settlement/settlement.h"
#include "component/info.h"
#include "entryhub/entryhubo.h"

class SettlementPrimaryModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    SettlementPrimaryModel(EntryHub* dbhub, CSectionInfo& info, QObject* parent = nullptr);
    ~SettlementPrimaryModel();

signals:
    void SSyncDouble(const QUuid& settlement_id, int column, double delta1);

public slots:
    void RSyncFinished(const QUuid& partner_id, const QUuid& settlement_id, bool settlement_finished);
    void RResetModel(const QUuid& partner_id, const QUuid& settlement_id, bool settlement_finished);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

private:
    void RemoveUnfinishedNode();
    void UpdateSettlementInfo(const QUuid& partner_id, const QUuid& settlement_id, bool settlement_finished);

private:
    EntryHubO* dbhub_ {};
    CSectionInfo& info_;

    QUuid partner_id_ {};
    QUuid settlement_id_ {};
    bool settlement_finished_ {};

    SettlementList settlementList_list_ {};
};

#endif // SETTLEMENTPRIMARYMODEL_H
