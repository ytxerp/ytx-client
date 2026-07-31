#ifndef PATHUTILS_H
#define PATHUTILS_H

#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

namespace path {

struct Dto {
    QUuid ancestor_id {};
    QUuid descendant_id {};

    void ReadJson(const QJsonObject& object);
};

QList<Dto> Parse(const QJsonArray& array);

template <typename Row> void BuildHierarchy(const QHash<QUuid, Row*>& node_hash, const QList<Dto>& paths)
{
    for (const auto& path : paths) {
        auto* ancestor = node_hash.value(path.ancestor_id, nullptr);
        auto* descendant = node_hash.value(path.descendant_id, nullptr);

        Q_ASSERT_X(ancestor != nullptr, Q_FUNC_INFO, "Ancestor not found");
        Q_ASSERT_X(descendant != nullptr, Q_FUNC_INFO, "Descendant not found");

        if (!ancestor || !descendant)
            continue;

        ancestor->children.emplaceBack(descendant);
        descendant->parent = ancestor;
    }
}

template <typename Row> void AttachRootNodes(const QHash<QUuid, Row*>& node_hash, Row* root)
{
    Q_ASSERT_X(root != nullptr, Q_FUNC_INFO, "Root node cannot be null");

    // Attach nodes without parent to the virtual root.
    for (auto* node : std::as_const(node_hash)) {
        if (node->parent == nullptr) {
            root->children.emplaceBack(node);
            node->parent = root;
        }
    }
}

} // namespace path

#endif // PATHUTILS_H
