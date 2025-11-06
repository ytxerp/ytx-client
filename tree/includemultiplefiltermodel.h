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

#ifndef INCLUDEMULTIPLEFILTERMODEL_H
#define INCLUDEMULTIPLEFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <QUuid>

#include "tree/itemmodel.h"

class IncludeMultipleFilterModel final : public QSortFilterProxyModel {
public:
    explicit IncludeMultipleFilterModel(const QSet<QUuid>* set, QObject* parent = nullptr)
        : QSortFilterProxyModel { parent }
        , set_ { set }
    {
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& /*source_parent*/) const override
    {
        assert(dynamic_cast<ItemModel*>(sourceModel()));
        auto id { static_cast<ItemModel*>(sourceModel())->ItemData(source_row, Qt::UserRole).toUuid() };

        return set_->contains(id);
    }

private:
    const QSet<QUuid>* set_ {};
};

#endif // INCLUDEMULTIPLEFILTERMODEL_H
