#include "nodeutils.h"

#include <QApplication>

namespace Utils {

void UpdatePath(QHash<QUuid, QString>& leaf, QHash<QUuid, QString>& branch, const Node* root, const Node* node, CString& separator)
{
    Q_ASSERT(root != nullptr);
    Q_ASSERT(node != nullptr);

    QQueue<const Node*> queue {};
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        const auto* current { queue.dequeue() };
        const auto path { ConstructPath(root, current, separator) };
        const NodeKind kind { current->kind };

        switch (kind) {
        case NodeKind::kBranch:
            for (const auto* child : current->children)
                queue.enqueue(child);

            branch.insert(current->id, path);
            break;
        case NodeKind::kLeaf:
            leaf.insert(current->id, path);
            break;
        default:
            break;
        }
    }
}

bool IsDescendant(const Node* lhs, const Node* rhs)
{
    Q_ASSERT(lhs != nullptr);
    Q_ASSERT(rhs != nullptr);

    if (lhs == rhs)
        return false;

    while (lhs && lhs != rhs)
        lhs = lhs->parent;

    return lhs == rhs;
}

QString ConstructPath(const Node* root, const Node* node, CString& separator)
{
    Q_ASSERT(root != nullptr);
    Q_ASSERT(node != nullptr);

    if (node == root)
        return QString();

    QStringList tmp {};

    while (node && node != root) {
        tmp.prepend(node->name);
        node = node->parent;
    }

    return tmp.join(separator);
}

void LeafPathBranchPathModel(CUuidString& leaf, CUuidString& branch, ItemModel* model)
{
    Q_ASSERT(model != nullptr);

    if (leaf.isEmpty() && branch.isEmpty())
        return;

    model->Reset();

    QVector<std::pair<QUuid, QString>> items {};
    items.reserve(leaf.size() + branch.size());

    for (const auto& [id, path] : leaf.asKeyValueRange()) {
        items.emplaceBack(id, path);
    }

    for (const auto& [id, path] : branch.asKeyValueRange()) {
        items.emplaceBack(id, path);
    }

    for (const auto& item : std::as_const(items)) {
        model->AppendItem(item.second, item.first);
    }

    model->sort(0);
}

void RemoveItem(ItemModel* model, const QUuid& node_id)
{
    Q_ASSERT(model != nullptr);
    Q_ASSERT(!node_id.isNull());

    for (int row = 0; row != model->rowCount(); ++row) {
        if (model->ItemData(row, Qt::UserRole).toUuid() == node_id) {
            model->RemoveItem(row);
            return;
        }
    }
}

void UpdateModel(const QHash<QUuid, QString>& leaf_path, ItemModel* leaf_path_model, const Node* node)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(leaf_path_model != nullptr);

    if (leaf_path.isEmpty())
        return;

    QQueue<const Node*> queue {};
    queue.enqueue(node);

    QSet<QUuid> leaf_range {};

    while (!queue.isEmpty()) {
        const auto* current { queue.dequeue() };
        const NodeKind kind { current->kind };

        switch (kind) {
        case NodeKind::kBranch:
            for (const auto* child : current->children)
                queue.enqueue(child);

            break;
        case NodeKind::kLeaf:
            leaf_range.insert(current->id);
            break;
        default:
            break;
        }
    }

    UpdateModelFunction(leaf_path_model, leaf_range, leaf_path);
}

void UpdatePathSeparator(CString& old_separator, CString& new_separator, QHash<QUuid, QString>& source_path)
{
    Q_ASSERT(!new_separator.isEmpty());
    Q_ASSERT(!old_separator.isEmpty());

    if (old_separator == new_separator || source_path.isEmpty())
        return;

    for (auto& path : source_path)
        path.replace(old_separator, new_separator);
}

void UpdateModelFunction(ItemModel* model, const QSet<QUuid>& update_range, CUuidString& source_path)
{
    Q_ASSERT(model != nullptr);

    if (update_range.isEmpty() || source_path.isEmpty())
        return;

    for (int row = 0; row != model->rowCount(); ++row) {
        auto id { model->ItemData(row, Qt::UserRole).toUuid() };

        if (update_range.contains(id)) {
            model->SetDisplay(row, source_path.value(id, QString {}));
        }
    }
}

}
