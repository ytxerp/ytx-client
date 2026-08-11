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

#pragma once

#include "component/sectioninfo.h"
#include "ordermodel.h"
#include "tree/model/treemodel.h"

namespace history {

class OrderModelP final : public OrderModel {
    Q_OBJECT

public:
    explicit OrderModelP(CSectionInfo& info, const QUuid& partner_id, TreeModel* tree_model_i, QObject* parent = nullptr);
    ~OrderModelP() override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

private:
    TreeModel* tree_model_i_ {};
    const QUuid partner_id_ {};
};
}
