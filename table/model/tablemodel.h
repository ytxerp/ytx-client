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
#include "enum/entryenum.h"
#include "table/entryshadow.h"
#include "utils/castutils.h"

using Utils::DerivedPtr;

class TableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ~TableModel() override;
    TableModel() = delete;

protected:
    explicit TableModel(CTableModelArg& arg, QObject* parent = nullptr);

signals:
    // send to LeafSStation
    void SAttachOneEntry(const QUuid& node_id, Entry* entry);
    void SDetachOneEntry(const QUuid& node_id, const QUuid& entry_id);
    void SUpdateBalance(const QUuid& node_id, const QUuid& entry_id);

    // send to its table view
    void SResizeColumnToContents(int column);

    // send to entryhub, FIPT
    void SAppendOneEntry(Entry* entry);
    void SDeleteOneEntry(const QUuid& node_id, const QUuid& entry_id);

public slots:
    virtual void RAppendMultiEntry(const EntryList& entry_list);
    virtual void RDeleteOneEntry(const QUuid& entry_id);
    virtual void RAppendOneEntry(Entry* entry);

    void RDeleteMultiEntry(const QSet<QUuid>& entry_id_set);

    void RUpdateBalance(const QUuid& entry_id);
    void RRefreshStatus();

    void RRefreshField(const QUuid& entry_id, int start, int end);
    void RDirectionRule(bool value);

public:
    // implemented functions
    inline int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return shadow_list_.size(); }
    inline int columnCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return info_.entry_header.size(); }
    inline QModelIndex parent(const QModelIndex& /*index*/) const override { return QModelIndex(); }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QModelIndex GetIndex(const QUuid& entry_id) const;
    virtual void ActionEntry(EntryAction action);

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

    virtual bool UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row)
    {
        Q_UNUSED(entry_shadow)
        Q_UNUSED(value)
        Q_UNUSED(row)
        return false;
    }

    virtual void AccumulateBalance(int start);
    virtual bool IsFinished(const QUuid& lhs_node, const QUuid& rhs_node) const
    {
        Q_UNUSED(lhs_node)
        Q_UNUSED(rhs_node)
        return false;
    };

    void RestartTimer(const QUuid& id);
    void FlushCaches();
    double CalculateBalance(EntryShadow* shadow) const { return (direction_rule_ == Rule::kDICD ? 1 : -1) * (*shadow->lhs_debit - *shadow->lhs_credit); }

    EntryShadow* InsertRowsImpl(int row, const QModelIndex& parent = QModelIndex());

protected:
    CSectionInfo& info_;
    bool direction_rule_ {};
    const QUuid lhs_id_ {};
    const Section section_ {};

    QList<EntryShadow*> shadow_list_ {};
    QDateTime last_issued_ {};

    QHash<QUuid, QJsonObject> pending_updates_ {};
    QHash<QUuid, QTimer*> pending_timers_ {};
};

#endif // TABLEMODEL_H
