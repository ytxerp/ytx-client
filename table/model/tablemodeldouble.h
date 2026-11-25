/*
 * Copyright (C) 2026 YTX
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

// Double-entry table models (Finance, Inventory, Task).
//
// Each row is an EntryShadow, which represents one side of an entry that is shared
// between two nodes (lhs / rhs). Changes are synchronized with the server in real time.
//
// This class is an intermediate base and cannot be instantiated directly; concrete
// sections inherit from it and provide their own public constructor.

#pragma once

#include "component/constantbool.h"
#include "enum/entryenum.h"
#include "table/entryshadow.h"
#include "tablemodel.h"

class TableModelDouble : public TableModel {
    Q_OBJECT

public:
    ~TableModelDouble() override;

protected:
    explicit TableModelDouble(CTableModelArg& arg, QObject* parent = nullptr);

signals:
    // send to LeafSStation
    void SAttachEntry(QUuid node_id, Entry* entry);
    void SDetachEntry(QUuid node_id, QUuid entry_id, QUuid extra_value = {});
    void SUpdateBalance(QUuid node_id, QUuid entry_id);

public slots:
    void RAppendEntries(const EntryList& entry_list) override;
    void RDetachEntry(QUuid entry_id, QUuid counter_node_id = {}) override;
    void RAttachEntry(Entry* entry) override;

    void RUpdateBalance(QUuid entry_id);
    void RDeleteEntries(const QSet<QUuid>& entry_id_set);
    void RDirectionRule(bool value);
    void RRefreshBatchEntries(int column);

public:
    // implemented functions
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : shadow_list_.size(); }
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count = 1, const QModelIndex& parent = QModelIndex()) override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QModelIndex GetIndex(QUuid entry_id) const override;
    Entry* GetEntry(const QModelIndex& index) const override { return shadow_list_.at(index.row())->entry; }

    void MarkEntries(MarkOperation operation);
    void LockEntries(bool locked);

protected:
    virtual bool UpdateNumeric(EntryShadow* shadow, double value, int row, NumericSide side);
    virtual bool UpdateRate(EntryShadow* shadow, double value);

    bool UpdateLocked(EntryShadow* shadow, bool value);
    bool UpdateLinkedNode(EntryShadow* shadow, QUuid value, int row);
    void AccumulateBalance(int start);
    double CalculateBalance(EntryShadow* shadow) const
    {
        return (direction_rule_ == direction_rule::kDICD ? 1 : -1) * (*shadow->lhs_debit - *shadow->lhs_credit);
    }

protected:
    QList<EntryShadow*> shadow_list_ {};
    bool direction_rule_ {};
};
