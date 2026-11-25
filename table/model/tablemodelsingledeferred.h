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

// Single-entry table models that do NOT sync in real time (Sale, Purchase, Reimbursement).
//
// Local changes, such as pending deletions, are held in memory until the user commits
// them. On commit, the changes are validated against the version stored in the database;
// if the version has changed, the commit is rejected as a conflict and the local data
// should be reloaded.
//
// Real-time sections must not inherit from this class, since they have nothing to defer.
//
// This class is an intermediate base and cannot be instantiated directly.

#pragma once

#include "tablemodelsingle.h"
#include "utils/delta.h"

class TableModelSingleDeferred : public TableModelSingle {
    Q_OBJECT

public:
    ~TableModelSingleDeferred() override;

protected:
    using TableModelSingle::TableModelSingle;

signals:
    // Notify the owning widget that table data has changed.
    void SDataChanged();
    // Notify the owning widget of entry value changes.
    void SSyncDelta(QUuid node_id, const entry::Delta& delta);

public:
    virtual void Purify() = 0;
    virtual bool HasZeroValue() const = 0;

    const QList<Entry*>& GetEntryList() const { return entry_list_; }

    bool HasPendingUpdate() const;
    void Finalize(QJsonObject& message);
    void SyncSucceeded();

protected:
    bool UpdateDescription(Entry* entry, const QString& value);
    bool UpdateDocument(Entry* entry, const QStringList& value);
    bool UpdateTag(Entry* entry, const QStringList& value);

protected:
    // Only used by sections that defer deletion (Reimbursement, Order).
    QSet<QUuid> pending_delete_ {};
};
