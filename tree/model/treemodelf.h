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

#ifndef TREEMODELF_H
#define TREEMODELF_H

#include "tree/model/treemodel.h"

class TreeModelF final : public TreeModel {
    Q_OBJECT

public:
    TreeModelF(CSectionInfo& info, CString& separator, int default_unit, QObject* parent = nullptr);
    ~TreeModelF() override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:
    QSet<QUuid> UpdateAncestorValue(
        Node* node, double initial_delta, double final_delta, double first_delta = 0.0, double second_delta = 0.0, double discount_delta = 0.0) override;
    std::pair<int, int> CacheColumnRange() const override { return { std::to_underlying(NodeEnumF::kCode), std::to_underlying(NodeEnumF::kNote) }; }
};

#endif // TREEMODELF_H
