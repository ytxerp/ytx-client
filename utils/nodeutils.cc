#include "nodeutils.h"

#include <QApplication>
#include <QQueue>

#include "component/constant.h"
#include "component/enumclass.h"

namespace NodeUtils {

void UpdatePath(UuidString& leaf, UuidString& branch, const Node* root, const Node* node, CString& separator)
{
    QQueue<const Node*> queue {};
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        const auto* current { queue.dequeue() };
        const auto path { ConstructPath(root, current, separator) };

        switch (current->kind) {
        case kBranch:
            for (const auto* child : current->children)
                queue.enqueue(child);

            branch.insert(current->id, path);
            break;
        case kLeaf:
            leaf.insert(current->id, path);
            break;
        default:
            break;
        }
    }
}

bool IsDescendant(const Node* lhs, const Node* rhs)
{
    if (!lhs || !rhs || lhs == rhs)
        return false;

    while (lhs && lhs != rhs)
        lhs = lhs->parent;

    return lhs == rhs;
}

void SortIterative(Node* node, std::function<bool(const Node*, const Node*)> Compare)
{
    if (!node)
        return;

    QQueue<Node*> queue {};
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        auto* current { queue.dequeue() };

        if (current->children.isEmpty())
            continue;

        std::sort(current->children.begin(), current->children.end(), Compare);
        for (auto* child : std::as_const(current->children)) {
            queue.enqueue(child);
        }
    }
}

QString ConstructPath(const Node* root, const Node* node, CString& separator)
{
    if (!node || node == root)
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
    if (!model || (leaf.isEmpty() && branch.isEmpty()))
        return;

    model->Clear();

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
    if (!model || node_id.isNull())
        return;

    for (int row = 0; row != model->rowCount(); ++row) {
        if (model->ItemData(row, Qt::UserRole).toUuid() == node_id) {
            model->RemoveItem(row);
            return;
        }
    }
}

void UpdateModel(CUuidString& leaf, ItemModel* leaf_model, const Node* node)
{
    if (!node)
        return;

    QQueue<const Node*> queue {};
    queue.enqueue(node);

    QSet<QUuid> leaf_range {};

    while (!queue.isEmpty()) {
        const auto* current { queue.dequeue() };

        switch (current->kind) {
        case kBranch:
            for (const auto* child : current->children)
                queue.enqueue(child);

            break;
        case kLeaf:
            leaf_range.insert(current->id);
            break;
        default:
            break;
        }
    }

    UpdateModelFunction(leaf_model, leaf_range, leaf);
}

void UpdatePathSeparator(CString& old_separator, CString& new_separator, UuidString& source_path)
{
    if (old_separator == new_separator || new_separator.isEmpty() || source_path.isEmpty())
        return;

    for (auto& path : source_path)
        path.replace(old_separator, new_separator);
}

void UpdateModelFunction(ItemModel* model, CUuidSet& update_range, CUuidString& source_path)
{
    if (!model || update_range.isEmpty() || source_path.isEmpty())
        return;

    for (int row = 0; row != model->rowCount(); ++row) {
        auto id = model->ItemData(row, Qt::UserRole).toUuid();

        if (update_range.contains(id)) {
            model->SetDisplay(row, source_path.value(id, QString {}));
        }
    }
}

}
