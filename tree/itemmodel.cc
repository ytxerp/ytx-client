#include "itemmodel.h"

ItemModel::ItemModel(QObject* parent)
    : QAbstractItemModel { parent }
{
}

QModelIndex ItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || row < 0 || row >= items_.size() || column != 0)
        return {};

    return createIndex(row, column);
}

QVariant ItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    return ItemData(index.row(), role);
}

void ItemModel::sort(int column, Qt::SortOrder order)
{
    if (column != 0) {
        return;
    }

    const auto compare
        = [order](const Item& lhs, const Item& rhs) { return (order == Qt::AscendingOrder) ? (lhs.display < rhs.display) : (lhs.display > rhs.display); };

    emit layoutAboutToBeChanged();
    std::sort(items_.begin(), items_.end(), compare);
    emit layoutChanged();
}

QVariant ItemModel::ItemData(int row, int role) const
{
    if (row < 0 || row >= items_.size())
        return {};

    const Item& item { items_.at(row) };

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

void ItemModel::AppendItem(const QString& display, const QVariant& user)
{
    const long long row { items_.size() };
    beginInsertRows(QModelIndex(), row, row);
    items_.emplace_back(Item { display, user });
    endInsertRows();
}

void ItemModel::RemoveItem(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    items_.removeAt(row);
    endRemoveRows();
}

void ItemModel::SetDisplay(int row, const QString& display)
{
    if (row < 0 || row >= items_.size() || items_[row].display == display)
        return;

    items_[row].display = display;
    emit dataChanged(index(row, 0), index(row, 0), { Qt::DisplayRole });
}

void ItemModel::Clear()
{
    beginResetModel();
    items_.clear();
    endResetModel();
}

void ItemModel::UpdateSeparator(const QString& old_separator, const QString& new_separator)
{
    if (old_separator == new_separator || old_separator.isEmpty() || new_separator.isEmpty())
        return;

    for (auto& item : items_) {
        item.display.replace(old_separator, new_separator);
    }

    if (!items_.empty()) {
        const QModelIndex top = index(0, 0);
        const QModelIndex bottom = index(rowCount() - 1, 0);
        emit dataChanged(top, bottom, { Qt::DisplayRole, Qt::EditRole });
    }
}
