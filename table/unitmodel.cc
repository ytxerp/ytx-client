#include "unitmodel.h"

UnitModel::UnitModel(QObject* parent)
    : QAbstractTableModel { parent }
{
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
