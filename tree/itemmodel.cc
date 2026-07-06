#include "itemmodel.h"

ItemModel::ItemModel(QObject* parent)
    : QAbstractItemModel { parent }
{
}

QModelIndex ItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QVariant ItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return {};

    const int row { index.row() };
    if (row < 0 || row >= items_.size())
        return {};

    const Item& item { items_[row] };

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

bool ItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != 0)
        return false;

    if (role != Qt::EditRole)
        return false;

    const int row { index.row() };
    if (row < 0 || row >= items_.size())
        return false;

    Item& item { items_[row] };

    const QString new_value { value.toString() };
    if (item.display == new_value)
        return false;

    item.display = new_value;

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void ItemModel::sort(int column, Qt::SortOrder order)
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

bool ItemModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid())
        return false;

    if (count != 1)
        return false;

    if (row < 0 || row >= items_.size())
        return false;

    beginRemoveRows(parent, row, row);

    items_.removeAt(row);

    endRemoveRows();

    return true;
}

void ItemModel::AppendItem(const QString& display, const QUuid& id)
{
    const long long row { items_.size() };
    beginInsertRows(QModelIndex(), row, row);
    items_.emplace_back(Item { display, id });
    endInsertRows();
}

bool ItemModel::RemoveItem(const QUuid& id)
{
    if (const int row { FindRow(id) }; row != -1)
        return removeRows(row);

    return false;
}

void ItemModel::Reset()
{
    beginResetModel();
    items_.clear();
    endResetModel();
}

void ItemModel::UpdateSeparator(const QString& old_separator, const QString& new_separator)
{
    Q_ASSERT(!new_separator.isEmpty());
    Q_ASSERT(!old_separator.isEmpty());

    if (old_separator == new_separator)
        return;

    for (auto& item : items_) {
        item.display.replace(old_separator, new_separator);
    }

    if (!items_.empty()) {
        const QModelIndex top { index(0, 0) };
        const QModelIndex bottom { index(rowCount() - 1, 0) };
        emit dataChanged(top, bottom, { Qt::DisplayRole, Qt::EditRole });
    }
}

int ItemModel::FindRow(const QUuid& id) const
{
    for (int row = 0; row != items_.size(); ++row) {
        if (items_[row].user == id)
            return row;
    }

    return -1;
}
