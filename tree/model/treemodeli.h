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

#ifndef TREEMODELI_H
#define TREEMODELI_H

#include "tree/model/treemodel.h"

class TreeModelI final : public TreeModel {
    Q_OBJECT

public:
    TreeModelI(CSectionInfo& info, CString& separator, int default_unit, QObject* parent = nullptr);
    ~TreeModelI() override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    const QString& Color(const QUuid& node_id) const { return NodeUtils::Value(node_hash_, node_id, &NodeI::color); }
    double UnitPrice(const QUuid& node_id) const { return NodeUtils::Value(node_hash_, node_id, &NodeI::unit_price); }

    QSortFilterProxyModel* IncludeUnitModel(int unit, QObject* parent) override;
    QSortFilterProxyModel* ExcludeMultipleModel(const QUuid& node_id, int unit, QObject* parent) override;
    void ResetColor(const QModelIndex& index) override;

protected:
    const QSet<QUuid>* UnitSet(int unit) const override;
    void RemoveUnitSet(const QUuid& node_id, int unit) override;
    void InsertUnitSet(const QUuid& node_id, int unit) override;

private:
    QSet<QUuid> pos_set_ {}; // Set of all nodes that are position-unit
    QSet<QUuid> int_set_ {}; // Set of all nodes that are internal-unit
    QSet<QUuid> ext_set_ { QUuid() }; // Set of all nodes that are external-unit
};

#endif // TREEMODELI_H
