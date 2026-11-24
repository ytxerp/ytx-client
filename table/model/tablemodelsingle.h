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

// Single-entry table models (Partner, Sale, Purchase, Reimbursement).
//
// Each row is an Entry that belongs to exactly one node. This class only holds the
// state shared by all single-entry sections (entry_list_). Sections that do not sync
// in real time should inherit from TableModelSingleDeferred instead.
//
// This class is an intermediate base and cannot be instantiated directly.

#pragma once

#include "tablemodel.h"

class TableModelSingle : public TableModel {
    Q_OBJECT

public:
    ~TableModelSingle() override = default;

protected:
    using TableModel::TableModel;

public slots:
    void RAppendEntries(const EntryList& entry_list) override;

public:
    // implemented functions
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : entry_list_.size(); }
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    Entry* GetEntry(const QModelIndex& index) const override { return entry_list_.at(index.row()); }
    QModelIndex GetIndex(QUuid entry_id) const override;

protected:
    virtual bool UpdateInternalSku(Entry* entry, QUuid value, int row)
    {
        Q_UNUSED(entry)
        Q_UNUSED(value)
        Q_UNUSED(row)
        return false;
    }

protected:
    QList<Entry*> entry_list_ {};
};
