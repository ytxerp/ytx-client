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

#ifndef LEAFMODEL_H
#define LEAFMODEL_H

#include <QAbstractItemModel>
#include <QTimer>

#include "component/arg/entrymodelarg.h"
#include "entryhub/entryhub.h"
#include "utils/castutils.h"
#include "utils/entryutils.h"

using CastUtils::DerivedPtr;

class LeafModel : public QAbstractItemModel {
    Q_OBJECT

public:
    virtual ~LeafModel();
    LeafModel() = delete;

protected:
    LeafModel(CLeafModelArg& arg, QObject* parent = nullptr);

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

public slots:
    void RRemoveMultiEntry(const QSet<QUuid>& entry_id_set);
    void RAppendMultiEntry(const EntryList& entry_list);

    void RAppendOneEntry(Entry* entry);
    void RRemoveOneEntry(const QUuid& entry_id);

    void RUpdateBalance(const QUuid& entry_id);
    void RMarkAction();

    void RRefreshField(const QUuid& entry_id, int start, int end);
    void RSyncRule(bool value);

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

    void AccumulateBalance(int start);
    void RestartTimer(const QUuid& id);

protected:
    EntryHub* entry_hub_ {};
    CSectionInfo& info_;
    bool direction_rule_ {};
    const QUuid lhs_id_ {};
    const Section section_ {};

    QList<EntryShadow*> shadow_list_ {};

    QHash<QUuid, QJsonObject> caches_ {};
    QHash<QUuid, QTimer*> timers_ {};
};

#endif // LEAFMODEL_H
