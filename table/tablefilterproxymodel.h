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

#include <QDateTime>
#include <QHash>
#include <QSortFilterProxyModel>

#include "component/constantstring.h"

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
    static bool Match(const QVariant& data, const QVariant& filter)
    {
        if (filter.userType() != QMetaType::QString)
            return data == filter;

        QString text {};

        switch (data.userType()) {
        case QMetaType::QDateTime:
            text = data.toDateTime().toString(datetime_format::kDashedDate);
            break;
        default:
            text = data.toString();
            break;
        }

        return text.contains(filter.toString(), Qt::CaseInsensitive);
    }

private:
    QHash<int, QVariant> filters_ {};
};