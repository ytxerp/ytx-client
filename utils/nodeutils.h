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

#ifndef NODEUTILS_H
#define NODEUTILS_H

#include "component/constant.h"
#include "component/enumclass.h"
#include "component/using.h"
#include "tree/itemmodel.h"
#include "tree/node.h"

namespace NodeUtils {

constexpr int KindColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
        return std::to_underlying(NodeEnumF::kKind);
    case Section::kTask:
        return std::to_underlying(NodeEnumT::kKind);
    case Section::kPartner:
        return std::to_underlying(NodeEnumP::kKind);
    case Section::kInventory:
        return std::to_underlying(NodeEnumI::kKind);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(NodeEnumO::kKind);
    default:
        return -1;
    }
}

constexpr int UnitColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
        return std::to_underlying(NodeEnumF::kUnit);
    case Section::kTask:
        return std::to_underlying(NodeEnumT::kUnit);
    case Section::kPartner:
        return std::to_underlying(NodeEnumP::kUnit);
    case Section::kInventory:
        return std::to_underlying(NodeEnumI::kUnit);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(NodeEnumO::kUnit);
    default:
        return -1;
    }
}

bool IsDescendant(const Node* lhs, const Node* rhs);

void SortIterative(Node* node, std::function<bool(const Node*, const Node*)> Compare);

QString ConstructPath(const Node* root, const Node* node, CString& separator);
void UpdatePath(UuidString& leaf, UuidString& branch, const Node* root, const Node* node, CString& separator);

void LeafPathBranchPathModel(CUuidString& leaf, CUuidString& branch, ItemModel* model);

void RemoveItem(ItemModel* model, const QUuid& node_id);

void UpdateModel(CUuidString& leaf, ItemModel* leaf_model, const Node* node);
void UpdatePathSeparator(CString& old_separator, CString& new_separator, UuidString& source_path);

void UpdateModelFunction(ItemModel* model, CUuidSet& update_range, CUuidString& source_path);

template <typename Field, typename Node> const Field& Value(CNodeHash& hash, const QUuid& node_id, Field Node::* member)
{
    if (auto it = hash.constFind(node_id); it != hash.constEnd()) {
        if (auto derived = dynamic_cast<Node*>(it.value())) {
            return derived->*member;
        }
    }

    // If the node_id does not exist, return a static empty object to ensure a safe default value
    // Examples:
    // double InitialTotal(QUuid node_id) const { return GetValue(node_id, &Node::initial_total); }
    // double FinalTotal(QUuid node_id) const { return GetValue(node_id, &Node::final_total); }
    // Note: In the SetStatus() function of TreeWidget,
    // a node_id of 0 may be passed, so empty{} is needed to prevent illegal access

    static const Field empty {};
    return empty;
}

// Update a QString or int field of an object and cache the change in a QJsonObject.
// Returns true if the value was changed.
template <typename Field, typename T>
bool UpdateField(QJsonObject& cache, T* object, CString& field, const Field& value, Field T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    Field& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    cache.insert(field, value);

    if (restart_timer)
        restart_timer();

    return true;
}

template <typename Field, typename T>
bool UpdateDouble(QJsonObject& cache, T* object, CString& field, const Field& value, Field T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    Field& current_value { object->*member };

    if (std::abs(current_value - value) < kTolerance)
        return false;

    current_value = value;
    cache.insert(field, QString::number(value, 'f', kMaxNumericScale_4));

    if (restart_timer)
        restart_timer();

    return true;
}

template <typename T>
bool UpdateDocument(
    QJsonObject& cache, T* object, CString& field, const QStringList& value, QStringList T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    QStringList& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    cache.insert(field, value.join(kSemicolon));

    if (restart_timer)
        restart_timer();

    return true;
}

template <typename T>
bool UpdateUuid(QJsonObject& cache, T* object, CString& field, const QUuid& value, QUuid T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    if (object->kind != kLeaf) {
        return false;
    }

    QUuid& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    cache.insert(field, value.toString(QUuid::WithoutBraces));

    if (restart_timer)
        restart_timer();

    return true;
}

template <typename T>
bool UpdateIssuedTime(
    QJsonObject& cache, T* object, CString& field, const QDateTime& value, QDateTime T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    QDateTime& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    cache.insert(field, value.toString(Qt::ISODate));

    if (restart_timer)
        restart_timer();

    return true;
}
};

#endif // NODEUTILS_H
