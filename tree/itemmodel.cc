#include "itemmodel.h"

#include "global/resourcepool.h"
#include "utils/templateutils.h"

ItemModel::ItemModel(QObject* parent)
    : QAbstractItemModel { parent }
{
}

ItemModel::~ItemModel() { ResourcePool<Item>::Instance().Recycle(list_); }

QModelIndex ItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant ItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return {};

    const int row { index.row() };
    if (row < 0 || row >= list_.size())
        return {};

    const Item* item { list_[row] };

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return item->display;

    case Qt::UserRole:
        return item->id;

    default:
        return {};
    }
}

void ItemModel::sort(int column, Qt::SortOrder order)
{
    if (column != 0) {
        return;
    }

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, [order](const Item* lhs, const Item* rhs) { return utils::CompareString(lhs->display, rhs->display, order); });
    emit layoutChanged();
}

void ItemModel::AppendItem(const QString& display, const QUuid& id)
{
    if (hash_.contains(id))
        return;

    auto* item { ResourcePool<Item>::Instance().Allocate() };

    item->display = display;
    item->id = id;

    auto it = std::lower_bound(list_.cbegin(), list_.cend(), item,
        [](const Item* lhs, const Item* rhs) { return utils::CompareString(lhs->display, rhs->display, Qt::AscendingOrder); });

    const int row { static_cast<int>(it - list_.cbegin()) };

    beginInsertRows(QModelIndex(), row, row);
    list_.emplace(list_.cbegin() + row, item);
    hash_.insert(id, item);
    endInsertRows();
}

bool ItemModel::RemoveItem(const QUuid& id)
{
    for (int row = 0; row != list_.size(); ++row) {
        auto* item { list_.at(row) };

        if (item->id != id)
            continue;

        beginRemoveRows({}, row, row);

        list_.removeAt(row);
        hash_.remove(id);

        endRemoveRows();

        ResourcePool<Item>::Instance().Recycle(item);

        return true;
    }

    return false;
}

void ItemModel::Reset()
{
    beginResetModel();
    ResourcePool<Item>::Instance().Recycle(list_);
    hash_.clear();
    endResetModel();
}

void ItemModel::SetSeparator(const QString& old_separator, const QString& new_separator)
{
    Q_ASSERT(!new_separator.isEmpty());
    Q_ASSERT(!old_separator.isEmpty());

    if (old_separator == new_separator)
        return;

    for (auto* item : std::as_const(list_)) {
        item->display.replace(old_separator, new_separator);
    }
}

void ItemModel::SetDisplay(const QUuid& id, const QString& display)
{
    auto* item { hash_.value(id, nullptr) };
    if (!item || item->display == display)
        return;

    item->display = display;
}

void ItemModel::Rebuild(const QHash<QUuid, QString>& leaf_path)
{
    QList<Item*> new_list {};
    QHash<QUuid, Item*> new_hash {};

    new_list.reserve(leaf_path.size());
    new_hash.reserve(leaf_path.size());

    for (auto it = leaf_path.cbegin(); it != leaf_path.cend(); ++it) {
        auto* item { ResourcePool<Item>::Instance().Allocate() };

        item->id = it.key();
        item->display = it.value();

        new_list.append(item);
        new_hash.insert(item->id, item);
    }

    std::ranges::sort(new_list, [](const Item* lhs, const Item* rhs) { return utils::CompareString(lhs->display, rhs->display, Qt::AscendingOrder); });

    beginResetModel();

    ResourcePool<Item>::Instance().Recycle(list_);
    hash_.clear();

    list_ = std::move(new_list);
    hash_ = std::move(new_hash);

    endResetModel();
}
