#include "tablesummarymodel.h"

TableSummaryModel::TableSummaryModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
}

void TableSummaryModel::RSummaryChanged(const QList<QVariant>& values)
{
    beginResetModel();

    values_ = values;

    endResetModel();
}

QVariant TableSummaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < header_.size())
        return header_.at(section);

    return QVariant();
}

QModelIndex TableSummaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TableSummaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return QModelIndex();
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

Qt::ItemFlags TableSummaryModel::flags(const QModelIndex& index) const { return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags; }
