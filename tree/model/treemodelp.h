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

#ifndef TREEMODELP_H
#define TREEMODELP_H

#include "tree/model/treemodel.h"

class TreeModelP final : public TreeModel {
    Q_OBJECT

public:
    TreeModelP(CSectionInfo& info, CString& separator, QObject* parent = nullptr);
    ~TreeModelP() override = default;

public slots:
    void RUpdateAmount(const QUuid& node_id, double initial_delta);

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QSortFilterProxyModel* IncludeUnitModel(int unit, QObject* parent) override;

protected:
    const QSet<QUuid>* UnitSet(int unit) const override;
    void RemoveUnitSet(const QUuid& node_id, int unit) override;
    void InsertUnitSet(const QUuid& node_id, int unit) override;
    QSet<QUuid> UpdateAncestorTotal(Node* node, double initial_delta, double final_delta) override;

private:
    QSet<QUuid> cset_ {}; // Set of all nodes that are customer unit
    QSet<QUuid> vset_ {}; // Set of all nodes that are vendor unit
    QSet<QUuid> eset_ { QUuid() }; // Set of all nodes that are employee unit
};

#endif // TREEMODELP_H
