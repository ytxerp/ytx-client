#include "tablefilterproxymodel.h"

void TableFilterProxyModel::RFilterChanged(int column, const QVariant& value)
{
    if (column < 0 || filters_.value(column) == value)
        return;

    beginFilterChange();

    if (value.isValid())
        filters_.insert(column, value);
    else
        filters_.remove(column);

    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void TableFilterProxyModel::RFiltersCleared()
{
    if (filters_.isEmpty())
        return;

    beginFilterChange();

    filters_.clear();

    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

bool TableFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (filters_.isEmpty())
        return true;

    const auto* source { sourceModel() };
    if (!source)
        return true;

    const int column_count { source->columnCount(source_parent) };

    for (auto it = filters_.cbegin(); it != filters_.cend(); ++it) {
        const int column { it.key() };

        if (column >= column_count)
            continue;

        const auto index { source->index(source_row, column, source_parent) };

        if (!Match(index.data(filterRole()), it.value()))
            return false;
    }

    return true;
}
