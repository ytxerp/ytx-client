#include "searchnodemodel.h"

#include <QJsonArray>

#include "utils/tagutils.h"

SearchNodeModel::SearchNodeModel(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , tree_model_ { tree_model }
    , section_ { info.section }
    , tag_hash_ { tag_hash }
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
    // 1. Prepare the result list (do not modify the model yet)
    QList<Node*> results {};

    if (!text.isEmpty()) {
        // Parse search input into text and tag set
        const SearchQuery query { Utils::ParseSearchQuery(text, tag_hash_) };

        if (!query.tags.isEmpty()) {
            // Tag search has higher priority
            tree_model_->SearchTag(results, query.tags);
        } else if (!query.text.isEmpty()) {
            // Search by description text if no tags
            tree_model_->SearchName(results, query.text);
        }
    }

    // 2. Update the model in one step
    beginResetModel();
    node_list_ = std::move(results); // Move results to avoid copying
    endResetModel();
}
