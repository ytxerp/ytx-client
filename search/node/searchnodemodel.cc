#include "searchnodemodel.h"

#include <QJsonArray>

#include "utils/tagutils.h"

namespace search {

NodeModel::NodeModel(Section section, const QStringList& header, CTreeModel* tree_model, const QHash<QUuid, tag::Row>& tag_hash, QObject* parent)
    : QAbstractItemModel { parent }
    , section_ { section }
    , header_ { header }
    , tree_model_ { tree_model }
    , tag_hash_ { tag_hash }
{
}

QModelIndex NodeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex NodeModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int NodeModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int NodeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant NodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void NodeModel::Search(const QString& text)
{
    // Prepare the result list (do not modify the model yet)
    QList<Node*> results {};

    const QString trimmed { text.trimmed() };

    if (!trimmed.isEmpty()) {
        // Parse search input into text and tag set
        const auto query { tag::ParseSearchQuery(trimmed, tag_hash_) };

        if (!query.tags.isEmpty()) {
            // Tag search has higher priority
            tree_model_->SearchTag(results, query.tags);
        } else if (!query.text.isEmpty()) {
            // Search by description text if no tags
            tree_model_->SearchName(results, query.text);
        }
    }

    // Update the model in one step
    beginResetModel();
    list_ = std::move(results); // Move results to avoid copying
    endResetModel();
}
}
