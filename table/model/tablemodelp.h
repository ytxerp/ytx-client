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

#ifndef TABLEMODELP_H
#define TABLEMODELP_H

#include "tablemodel.h"

class TableModelP final : public TableModel {
    Q_OBJECT

public:
    explicit TableModelP(CTableModelArg& arg, QObject* parent = nullptr);
    ~TableModelP() override = default;

public slots:
    void RAppendMultiEntry(const EntryList& entry_list) override;
    void RRemoveOneEntry(const QUuid& entry_id) override;
    void RAppendOneEntry(Entry* entry) override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    inline int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return entry_list_.size(); }

    bool insertRows(int row, int /*count*/, const QModelIndex& parent) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QModelIndex GetIndex(const QUuid& entry_id) const override;

protected:
    bool UpdateInternalSku(EntryP* entry, const QUuid& value);

private:
    QSet<QUuid> internal_sku_ {};
    QList<Entry*> entry_list_ {};
};

#endif // TABLEMODELP_H
