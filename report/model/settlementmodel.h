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

#ifndef SETTLEMENTMODEL_H
#define SETTLEMENTMODEL_H

#include <QAbstractItemModel>
#include <QJsonObject>

#include "component/info.h"
#include "entryhub/entryhubo.h"
#include "report/settlement.h"

class SettlementModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    SettlementModel(EntryHub* dbhub, CSectionInfo& info, QObject* parent = nullptr);
    ~SettlementModel();

signals:
    void SResetModel(const QUuid& party_id, const QUuid& settlement_id, bool settlement_finished);
    void SSyncFinished(const QUuid& party_id, const QUuid& settlement_id, bool settlement_finished);

    // send to its table view
    void SResizeColumnToContents(int column);

    // send to NodeModelS
    void SUpdateAmount(const QUuid& settlement_id, double initial_delta, double final_delta);

public slots:
    void RSyncDouble(const QUuid& settlement_id, int column, double delta1);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void sort(int column, Qt::SortOrder order) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    void ResetModel(const QDateTime& start, const QDateTime& end);

private:
    bool UpdateParty(Settlement* settlement, const QUuid& party_id);
    bool UpdateFinished(Settlement* settlement, bool finished);

private:
    EntryHubO* dbhub_ {};
    QJsonObject update_cache_ {};

    CSectionInfo& info_;

    SettlementList settlement_list_ {};
};

#endif // SETTLEMENTMODEL_H
