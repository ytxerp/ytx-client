#include "filterproxymodel.h"

void FilterProxyModel::RFilterChanged(int column, const QVariant& value)
{
    if (column < 0)
        return;

    std::optional<Filter> filter {};

    switch (value.userType()) {
    case QMetaType::Int:
        filter = Filter { value.toInt() };
        break;
    case QMetaType::QString:
        if (const auto text { value.toString() }; !text.isEmpty())
            filter = Filter { text };
        break;
    default:
        break;
    }

    const auto it { filters_.constFind(column) };

    // No change: the column already has an equal filter, or has none and the new value is empty
    if (it == filters_.cend() ? !filter.has_value() : (filter && *it == *filter))
        return;

    beginFilterChange();

    if (filter)
        filters_.insert(column, *filter);
    else
        filters_.remove(column);

    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void FilterProxyModel::RFiltersCleared()
{
    if (filters_.isEmpty())
        return;

    beginFilterChange();

    filters_.clear();

    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (filters_.isEmpty())
        return true;

    const auto* source { sourceModel() };
    if (!source)
        return true;

    const int column_count { source->columnCount(source_parent) };
    const int role { filterRole() };

    for (auto it = filters_.cbegin(); it != filters_.cend(); ++it) {
        const int column { it.key() };

        if (column >= column_count)
            continue;

        if (!Match(source->index(source_row, column, source_parent).data(role), it.value()))
            return false;
    }

    return true;
}
