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

#include <QHash>
#include <QSortFilterProxyModel>

class TableFilterProxyModel final : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit TableFilterProxyModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setFilterKeyColumn(-1);
        setDynamicSortFilter(true);
    }

    bool HasFilters() const { return !filters_.isEmpty(); }

public slots:
    void RSortRequested(int column, Qt::SortOrder order) { sort(column, order); }
    void RFilterChanged(int column, const QVariant& value);
    void RFiltersCleared();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    using Filter = std::variant<int, QString>;

    static bool Match(const QVariant& data, const Filter& filter)
    {
        if (const auto* number { std::get_if<int>(&filter) })
            return data.toInt() == *number;

        return data.toString().contains(*std::get_if<QString>(&filter), Qt::CaseInsensitive);
    }

private:
    QHash<int, Filter> filters_ {};
};