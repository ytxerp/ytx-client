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

#include <QAbstractItemModel>
#include <QString>
#include <QUuid>

class UnitModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit UnitModel(QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QModelIndex parent(const QModelIndex&) const override { return {}; }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : list_.size(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 1;
    }

    void Rebuild(const QMap<int, QString>& map)
    {
        QList<Item> new_list {};
        new_list.reserve(map.size());

        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            new_list.emplace_back(Item {
                .display = it.value(),
                .unit = it.key(),
            });
        }

        beginResetModel();
        list_ = std::move(new_list);
        endResetModel();
    }

protected:
    struct Item {
        QString display {};
        int unit {};
    };

private:
    QList<Item> list_ {};
};
