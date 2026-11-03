/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

#include "tree/node.h"

namespace path {

struct Dto final {
    QUuid ancestor_id {};
    QUuid descendant_id {};

    void ReadJson(const QJsonObject& object);
};

QList<Dto> Parse(const QJsonArray& array);

QString Build(const Node* node, const QString& separator);
QString Build(const Node* node, const Node* root, const QString& separator);

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
