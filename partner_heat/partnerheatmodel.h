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

#ifndef PARTNERHEATMODEL_H
#define PARTNERHEATMODEL_H

#include <QAbstractItemModel>

#include "partnerheatrow.h"

namespace partner_heat {

class Model final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit Model(const QStringList& header, QObject* parent = nullptr);
    ~Model() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override
    {
        Q_UNUSED(index)
        return QModelIndex();
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return list_.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return header_.size();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void Rebuild(const QJsonArray& array);

private:
    QList<Row*> list_ {};
    const QStringList& header_ {};
};
}

#endif // PARTNERHEATMODEL_H
