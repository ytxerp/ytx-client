#include "dynamicsummarymodel.h"

DynamicSummaryModel::DynamicSummaryModel(int column_count, QObject* parent)
    : QAbstractTableModel(parent)
    , column_count_ { column_count }
{
    values_.resize(column_count_);
}

int DynamicSummaryModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 1; }

int DynamicSummaryModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : column_count_; }

QVariant DynamicSummaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    if (role != Qt::DisplayRole)
        return {};

    if (index.row() != 0 || index.column() < 0 || index.column() >= values_.size())
        return {};

    return values_.at(index.column());
}

void DynamicSummaryModel::RebuildHeader(int column_count)
{
    if (column_count_ == column_count)
        return;

    beginResetModel();

    column_count_ = column_count;
    values_.clear();

    endResetModel();
}

void DynamicSummaryModel::RSummaryChanged(const QList<QVariant>& values)
{
    Q_ASSERT(values.size() == column_count_);

    if (values_ == values)
        return;

    values_ = values;

    if (const int columns { columnCount() }; columns > 0)
        emit dataChanged(index(0, 0), index(0, columns - 1), { Qt::DisplayRole });
}
