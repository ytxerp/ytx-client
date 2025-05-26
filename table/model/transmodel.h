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

#ifndef TRANSMODEL_H
#define TRANSMODEL_H

#include <QAbstractItemModel>
#include <QMutex>

#include "component/arg/transmodelarg.h"
#include "database/sql/sql.h"

class TransModel : public QAbstractItemModel {
    Q_OBJECT

public:
    virtual ~TransModel();
    TransModel() = delete;

protected:
    TransModel(CTransModelArg& arg, QObject* parent = nullptr);

signals:
    // send to TreeModel
    void SSyncDouble(const QUuid& node_id, int column, double value);
    void SSyncLeafValue(const QUuid& node_id, double delta1 = 0.0, double delta2 = 0.0, double delta3 = 0.0, double delta4 = 0.0, double delta5 = 0.0);
    void SSearch();

    // send to LeafSStation
    void SAppendOneTransL(Section section, const TransShadow* trans_shadow);
    void SRemoveOneTransL(Section section, const QUuid& node_id, const QUuid& trans_id);
    void SSyncBalance(Section section, const QUuid& node_id, const QUuid& trans_id);

    // send to SupportSStation
    void SAppendOneTransS(Section section, const QUuid& support_id, const QUuid& trans_id);
    void SRemoveOneTransS(Section section, const QUuid& support_id, const QUuid& trans_id);

    // send to its table view
    void SResizeColumnToContents(int column);

public slots:
    // receive from LeafSStation
    void RRemoveMultiTransL(const QUuid& node_id, const QSet<QUuid>& trans_id_set);
    void RAppendMultiTransL(const QUuid& node_id, const QSet<QUuid>& trans_id_set);
    void RAppendMultiTrans(const QUuid& node_id, const TransShadowList& trans_shadow_list);

    void RAppendOneTransL(const TransShadow* trans_shadow);
    void RRemoveOneTransL(const QUuid& node_id, const QUuid& trans_id);
    void RSyncBalance(const QUuid& node_id, const QUuid& trans_id);
    void RSyncRule(const QUuid& node_id, bool rule);

    // receive from TreeModel
    virtual void RSyncBoolWD(const QUuid& node_id, int column, bool value)
    {
        Q_UNUSED(node_id);
        Q_UNUSED(column);
        Q_UNUSED(value);
    }

    virtual void RSyncInt(const QUuid& node_id, int column, const QUuid& value)
    {
        Q_UNUSED(node_id);
        Q_UNUSED(column);
        Q_UNUSED(value);
    }

public:
    // implemented functions
    inline int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return trans_shadow_list_.size(); }
    inline int columnCount(const QModelIndex& /*parent*/ = QModelIndex()) const override { return info_.trans_header.size(); }
    inline QModelIndex parent(const QModelIndex& /*index*/) const override { return QModelIndex(); }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    virtual int GetNodeRow(const QUuid& node_id) const;

    QModelIndex GetIndex(const QUuid& trans_id) const;
    QStringList* GetDocumentPointer(const QModelIndex& index) const;

    void UpdateAllState(Check state);

protected:
    // virtual functions
    virtual bool UpdateDebit(TransShadow* trans_shadow, double value);
    virtual bool UpdateCredit(TransShadow* trans_shadow, double value);
    virtual bool UpdateRatio(TransShadow* trans_shadow, double value);

    virtual void IniRatio(TransShadow* trans_shadow) const { Q_UNUSED(trans_shadow) }
    virtual void UpdateUnitCost(const QUuid& lhs_node, const QUuid& rhs_node, double value)
    {
        Q_UNUSED(lhs_node)
        Q_UNUSED(rhs_node)
        Q_UNUSED(value)
    }

protected:
    Sql* sql_ {};
    CInfo& info_;
    bool node_rule_ {};
    QUuid node_id_ {};
    QMutex mutex_ {};
    const Section section_ {};

    QList<TransShadow*> trans_shadow_list_ {};
};

using PTransModel = QPointer<TransModel>;

#endif // TRANSMODEL_H
