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
    if (row < 0 || row >= list_.size())
        return {};

    const Item& item { list_[row] };

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return item.display;

    case Qt::UserRole:
        return item.unit;

    default:
        return {};
    }
}
