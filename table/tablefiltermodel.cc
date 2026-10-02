#include "tablefiltermodel.h"

TableFilterModel::TableFilterModel(const QStringList& header, int ignored_column, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
    , ignored_column_ { ignored_column }
{
}

int TableFilterModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 1; }

int TableFilterModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : header_.size(); }

QVariant TableFilterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() != 0 || (role != Qt::DisplayRole && role != Qt::EditRole))
        return {};

    return filters_.value(index.column());
}

bool TableFilterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() != 0 || role != Qt::EditRole)
        return false;

    const int column { index.column() };
    if (column < 0 || column >= columnCount() || column == ignored_column_)
        return false;

    QVariant v { value };
    if (v.userType() == QMetaType::QString && v.toString().isEmpty())
        v.clear();

    if (Filter(column) == v)
        return false;

    if (v.isValid())
        filters_.insert(column, v);
    else
        filters_.remove(column);

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    emit SFilterChanged(column, v);
    return true;
}

QVariant TableFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

Qt::ItemFlags TableFilterModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QModelIndex TableFilterModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TableFilterModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

void TableFilterModel::sort(int column, Qt::SortOrder order) { emit SSortRequested(column, order); }

QVariant TableFilterModel::Filter(int column) const { return filters_.value(column); }

void TableFilterModel::ClearFilters()
{
    if (filters_.isEmpty())
        return;

    filters_.clear();

    const int columns { columnCount() };

    if (columns > 0) {
        emit dataChanged(index(0, 0), index(0, columns - 1), { Qt::DisplayRole, Qt::EditRole });
    }

    emit SFiltersCleared();
}
