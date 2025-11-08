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

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QTimer>

#include "component/arg/tablemodelarg.h"
#include "table/entryshadow.h"
#include "utils/castutils.h"

using CastUtils::DerivedPtr;

class TableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    virtual ~TableModel();
    TableModel() = delete;

protected:
    TableModel(CTableModelArg& arg, QObject* parent = nullptr);

signals:
    // send to TreeModel
    void SSyncDelta(
        const QUuid& node_id, double initial_delta, double final_delta, double first_delta = 0.0, double second_delta = 0.0, double discount_delta = 0.0);

    // send to LeafSStation
    void SAppendOneEntry(const QUuid& node_id, Entry* entry);
    void SRemoveOneEntry(const QUuid& node_id, const QUuid& entry_id);
    void SUpdateBalance(const QUuid& node_id, const QUuid& entry_id);

    // send to its table view
    void SResizeColumnToContents(int column);

    // send to entryhub, FIPT
    void SInsertEntry(Entry* entry);
    void SRemoveEntry(const QUuid& entry_id);

public slots:
    void RRemoveMultiEntry(const QSet<QUuid>& entry_id_set);
    virtual void RAppendMultiEntry(const EntryList& entry_list);

    void RUpdateBalance(const QUuid& entry_id);
    void RRefreshStatus();

    void RRefreshField(const QUuid& entry_id, int start, int end);
    void RDirectionRule(bool value);

    void RAppendOneEntry(Entry* entry);
    void RRemoveOneEntry(const QUuid& entry_id);

public:
    // implemented functions
    inline int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return shadow_list_.size(); }
    inline int columnCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return info_.entry_header.size(); }
    inline QModelIndex parent(const QModelIndex& /*index*/) const override { return QModelIndex(); }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    virtual int GetRhsRow(const QUuid& rhs_id) const;
    QModelIndex GetIndex(const QUuid& entry_id) const;

    void ActionEntry(EntryAction action);

protected:
    virtual bool UpdateNumeric(EntryShadow* entry_shadow, double value, int row, bool is_debit)
    {
        Q_UNUSED(entry_shadow)
        Q_UNUSED(value)
        Q_UNUSED(is_debit)
        Q_UNUSED(row)
        return false;
    }

#if 0
    virtual bool UpdateDebit(EntryShadow* entry_shadow, double value, int row)
    {
        Q_UNUSED(value)
        Q_UNUSED(entry_shadow)
        Q_UNUSED(row)
        return false;
    }

    virtual bool UpdateCredit(EntryShadow* entry_shadow, double value, int row)
    {
        Q_UNUSED(entry_shadow)
        Q_UNUSED(value)
        Q_UNUSED(row)
        return false;
    }
#endif

    virtual bool UpdateRate(EntryShadow* entry_shadow, double value)
    {
        Q_UNUSED(entry_shadow)
        Q_UNUSED(value)
        return false;
    }

    virtual double CalculateBalance(EntryShadow* entry_shadow)
    {
        Q_UNUSED(entry_shadow)
        return {};
    }

    virtual bool UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row)
    {
        Q_UNUSED(entry_shadow)
        Q_UNUSED(value)
        Q_UNUSED(row)
        return false;
    }

    virtual void AccumulateBalance(int start);

    EntryShadow* InsertRowsImpl(int row, const QModelIndex& parent = QModelIndex());

    void RestartTimer(const QUuid& id);
    void FlushCaches();

protected:
    CSectionInfo& info_;
    bool direction_rule_ {};
    const QUuid lhs_id_ {};
    const Section section_ {};

    QList<EntryShadow*> shadow_list_ {};

    QHash<QUuid, QJsonObject> entry_caches_ {};
    QHash<QUuid, QTimer*> entry_timers_ {};
};

#endif // TABLEMODEL_H
