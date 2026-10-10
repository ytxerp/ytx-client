#include "filterinputmodel.h"

FilterInputModel::FilterInputModel(const QStringList& header, int ignored_column, QObject* parent)
    : QAbstractTableModel(parent)
    , header_ { header }
    , ignored_column_ { ignored_column }
{
}

int FilterInputModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 1; }

int FilterInputModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : header_.size(); }

QVariant FilterInputModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() != 0 || (role != Qt::DisplayRole && role != Qt::EditRole))
        return {};

    return filters_.value(index.column());
}

bool FilterInputModel::setData(const QModelIndex& index, const QVariant& value, int role)
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

QVariant FilterInputModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < header_.size())
        return header_.at(section);

    return QVariant();
}

Qt::ItemFlags FilterInputModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = QAbstractTableModel::flags(index);

    if (index.column() != ignored_column_)
        f |= Qt::ItemIsEditable;

    return f;
}

void FilterInputModel::sort(int column, Qt::SortOrder order) { emit SSortRequested(column, order); }

QVariant FilterInputModel::Filter(int column) const { return filters_.value(column); }

void FilterInputModel::ClearFilters()
{
    if (filters_.isEmpty())
        return;

    filters_.clear();

    const int columns { columnCount() };

    if (columns > 0)
        emit dataChanged(index(0, 0), index(0, columns - 1), { Qt::DisplayRole, Qt::EditRole });

    emit SFiltersCleared();
}
