#include "tablesummarymodel.h"

TableSummaryModel::TableSummaryModel(const QStringList& header, QObject* parent)
    : QAbstractTableModel(parent)
    , header_ { header }
{
}

void TableSummaryModel::RSummaryChanged(const QList<QVariant>& values)
{
    if (values_ == values)
        return;

    values_ = values;

    if (const int columns { columnCount() }; columns > 0)
        emit dataChanged(index(0, 0), index(0, columns - 1), { Qt::DisplayRole });
}

QVariant TableSummaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < header_.size())
        return header_.at(section);

    return QVariant();
}

int TableSummaryModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 1; }

int TableSummaryModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : header_.size(); }

QVariant TableSummaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    if (role != Qt::DisplayRole)
        return {};

    if (index.row() != 0 || index.column() < 0 || index.column() >= values_.size())
        return {};

    return values_.at(index.column());
}

Qt::ItemFlags TableSummaryModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
}
