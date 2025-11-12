#include "searchnodemodel.h"

#include <QJsonArray>

SearchNodeModel::SearchNodeModel(CSectionInfo& info, CTreeModel* tree_model, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , tree_model_ { tree_model }
    , section_ { info.section }
{
}

QModelIndex SearchNodeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SearchNodeModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SearchNodeModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return node_list_.size();
}

int SearchNodeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.node_header.size();
}

QVariant SearchNodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.node_header.at(section);

    return QVariant();
}

void SearchNodeModel::Search(const QString& text)
{
    beginResetModel();

    node_list_.clear();

    if (!text.isEmpty())
        tree_model_->SearchNode(node_list_, text);

    endResetModel();
}
