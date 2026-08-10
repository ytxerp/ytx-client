#include "unitmodel.h"

#include "utils/templateutils.h"

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
    if (row < 0 || row >= list_.size())
        return {};

    const Item& item { list_[row] };

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
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

    const auto compare = [order](const Item& lhs, const Item& rhs) { return utils::CompareString(lhs.display, rhs.display, order); };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, compare);
    emit layoutChanged();
}

void UnitModel::AppendItem(const QString& display, int user)
{
    const long long row { list_.size() };
    beginInsertRows(QModelIndex(), row, row);
    list_.emplace_back(Item { display, user });
    endInsertRows();
}
