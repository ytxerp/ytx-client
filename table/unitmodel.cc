#include "unitmodel.h"

UnitModel::UnitModel(QObject* parent)
    : QAbstractItemModel { parent }
{
}

QModelIndex UnitModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QVariant UnitModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return {};

    const int row { index.row() };
    if (row < 0 || row >= items_.size())
        return {};

    const Item& item { items_[row] };

    switch (role) {
    case Qt::DisplayRole:
        return item.display;

    case Qt::UserRole:
        return item.user;

    default:
        return {};
    }
}

void UnitModel::sort(int column, Qt::SortOrder order)
{
    if (column != 0) {
        return;
    }

    const auto compare
        = [order](const Item& lhs, const Item& rhs) { return (order == Qt::AscendingOrder) ? (lhs.display < rhs.display) : (lhs.display > rhs.display); };

    emit layoutAboutToBeChanged();
    std::ranges::sort(items_, compare);
    emit layoutChanged();
}

void UnitModel::AppendItem(const QString& display, const int& user)
{
    const long long row { items_.size() };
    beginInsertRows(QModelIndex(), row, row);
    items_.emplace_back(Item { display, user });
    endInsertRows();
}
