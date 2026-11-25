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

// Abstract base of all table models. Shared by all 7 sections.
//
// Owns the state common to every section: header, node id and section.
// It does not know whether a section is single-entry or double-entry, nor whether it
// syncs in real time; that knowledge lives in the intermediate classes below.
//
// pending_updates_ is only used by sections that hold local changes before submitting
// them (the double-entry sections and Partner). It stays empty in all other sections.
//
// Subclasses must implement insertRows() and removeRows().

#pragma once

#include <QAbstractItemModel>

#include "component/arg/tablemodelarg.h"
#include "table/entry.h"
#include "utils/templateutils.h"

using utils::DerivedPtr;

class TableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ~TableModel() override = default;
    TableModel() = delete;

protected:
    explicit TableModel(CTableModelArg& arg, QObject* parent = nullptr);

signals:
    // send to entryhub, FIPT
    void STransferEntry(Entry* entry);

    // send to MainWindow
    void SEntryLocked();

public slots:
    virtual void RAppendEntries(const EntryList& entry_list) = 0;
    virtual void RDetachEntry(QUuid entry_id, QUuid counter_node_id = {})
    {
        Q_UNUSED(entry_id)
        Q_UNUSED(counter_node_id)
    };
    virtual void RAttachEntry(Entry* entry) { Q_UNUSED(entry) };

    void RRefreshField(QUuid entry_id, int start, int end);

public:
    // implemented functions
    QModelIndex parent(const QModelIndex& /*index*/) const override { return QModelIndex(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : header_.size(); }
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool removeRows(int row, int count = 1, const QModelIndex& parent = QModelIndex()) override = 0;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override = 0;

    virtual QModelIndex GetIndex(QUuid entry_id) const = 0;
    virtual Entry* GetEntry(const QModelIndex& index) const = 0;

    void FlushTimers();

protected:
    void RestartTimer(QUuid id);
    void FlushTimer(QUuid id);

    void EmitDataChanged(int start_row, int end_row, int start_column, int end_column, const QModelIndex& parent = QModelIndex());
    void CancelPendingUpdate(QUuid entry_id);

protected:
    const QStringList header_ {};
    const QUuid node_id_ {};
    const Section section_ {};

    QHash<QUuid, PendingEntryUpdate> pending_updates_ {};
};
