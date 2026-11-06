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

#pragma once

#include <QAbstractItemModel>

#include "reimbursementrow.h"

namespace reimbursement {

class SecondaryModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit SecondaryModel(const QStringList& header, Status status, int finance_permission, QObject* parent = nullptr);
    ~SecondaryModel() override;

signals:
    void SSyncInitial(double amount);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& /*index*/) const override { return QModelIndex(); }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : list_.size(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : header_.size(); }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void Rebuild(const QJsonArray& array);
    void UpdateStatus(Status status);

    void Purify();
    void Finalize(QJsonObject& message);
    void SyncSucceeded();
    bool HasPendingUpdate() const;

private:
    bool UpdateDescription(SecondaryRow* row, const QString& value);
    bool UpdateDocument(SecondaryRow* row, const QStringList& value);
    bool UpdateInitial(SecondaryRow* row, double value);
    bool UpdateDebit(SecondaryRow* row, QUuid value);
    bool UpdateCredit(SecondaryRow* row, QUuid value);

private:
    const QStringList header_ {};
    Status status_ {};
    const int finance_permission_ {};

    QList<SecondaryRow*> list_ {};
    QSet<QUuid> pending_delete_ {};
};

}
