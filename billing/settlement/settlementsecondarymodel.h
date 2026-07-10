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

#ifndef SETTLEMENTSECONDARYMODEL_H
#define SETTLEMENTSECONDARYMODEL_H

#include <QAbstractItemModel>

#include "settlement.h"

namespace settlement {

class SecondaryModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit SecondaryModel(const QStringList& header, SettlementStatus status, QObject* parent = nullptr);
    ~SecondaryModel() override;

signals:
    void SSyncAmount(double amount);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

    void ResetModel(const QJsonArray& array);
    void UpdateStatus(SettlementStatus status);
    void Finalize(QJsonObject& message);
    bool HasPendingUpdate() const { return !pending_selected_.isEmpty(); }

private:
    const QStringList& header_;

    SettlementStatus status_ {};

    QList<SecondaryRow*> list_ {};
    QList<SecondaryRow*> list_cache_ {};

    QSet<QUuid> pending_selected_ {};
};

}

#endif // SETTLEMENTSECONDARYMODEL_H
