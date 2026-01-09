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

#include <QQueue>

#include "component/constant.h"
#include "component/using.h"
#include "enum/nodeenum.h"
#include "enum/section.h"
#include "tree/itemmodel.h"
#include "tree/node.h"

namespace Utils {

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
    }

    Q_UNREACHABLE();
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
    }

    Q_UNREACHABLE();
}

constexpr int DirectionRuleColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
        return std::to_underlying(NodeEnumF::kDirectionRule);
    case Section::kTask:
        return std::to_underlying(NodeEnumT::kDirectionRule);
    case Section::kPartner:
        return -1;
    case Section::kInventory:
        return std::to_underlying(NodeEnumI::kDirectionRule);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(NodeEnumO::kDirectionRule);
    }

    Q_UNREACHABLE();
}

constexpr int NodeDescriptionColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
        return std::to_underlying(NodeEnumF::kDescription);
    case Section::kTask:
        return std::to_underlying(NodeEnumT::kDescription);
    case Section::kPartner:
        return std::to_underlying(NodeEnumP::kDescription);
    case Section::kInventory:
        return std::to_underlying(NodeEnumI::kDescription);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(NodeEnumO::kDescription);
    }

    Q_UNREACHABLE();
}

constexpr std::pair<int, int> NodeNumericColumnRange(Section section)
{
    switch (section) {
    case Section::kFinance:
        return { std::to_underlying(NodeEnumF::kInitialTotal), std::to_underlying(NodeEnumF::kFinalTotal) };
    case Section::kTask:
        return { std::to_underlying(NodeEnumT::kInitialTotal), std::to_underlying(NodeEnumT::kFinalTotal) };
    case Section::kPartner:
        return { std::to_underlying(NodeEnumP::kInitialTotal), std::to_underlying(NodeEnumP::kInitialTotal) };
    case Section::kInventory:
        return { std::to_underlying(NodeEnumI::kInitialTotal), std::to_underlying(NodeEnumI::kFinalTotal) };
    case Section::kSale:
    case Section::kPurchase:
        return { std::to_underlying(NodeEnumO::kCountTotal), std::to_underlying(NodeEnumO::kFinalTotal) };
    }

    Q_UNREACHABLE();
}

constexpr std::pair<int, int> NodeCacheColumnRange(Section section)
{
    switch (section) {
    case Section::kFinance:
        return { std::to_underlying(NodeEnumF::kCode), std::to_underlying(NodeEnumF::kNote) };
    case Section::kTask:
        return { std::to_underlying(NodeEnumT::kCode), std::to_underlying(NodeEnumT::kDocument) };
    case Section::kPartner:
        return { std::to_underlying(NodeEnumP::kCode), std::to_underlying(NodeEnumP::kPaymentTerm) };
    case Section::kInventory:
        return { std::to_underlying(NodeEnumI::kCode), std::to_underlying(NodeEnumI::kCommission) };
    case Section::kSale:
    case Section::kPurchase:
        return { std::to_underlying(NodeEnumO::kPartner), std::to_underlying(NodeEnumO::kFinalTotal) };
    }

    Q_UNREACHABLE();
}

inline QString UnitString(UnitO unit)
{
    switch (unit) {
    case UnitO::kMonthly:
        return QObject::tr("MS");
    case UnitO::kImmediate:
        return QObject::tr("IS");
    case UnitO::kPending:
        return QObject::tr("PEND");
    }

    Q_UNREACHABLE();
}

bool IsDescendant(const Node* lhs, const Node* rhs);

template <typename F> void SortIterative(Node* node, F&& Compare)
{
    if (!node)
        return;

    QQueue<Node*> queue {};
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        Node* current = queue.dequeue();

        if (current->children.isEmpty())
            continue;

        std::sort(current->children.begin(), current->children.end(), std::forward<F>(Compare));

        for (Node* child : std::as_const(current->children)) {
            queue.enqueue(child);
        }
    }
}

QString ConstructPath(const Node* root, const Node* node, CString& separator);
void UpdatePath(QHash<QUuid, QString>& leaf, QHash<QUuid, QString>& branch, const Node* root, const Node* node, CString& separator);

void LeafPathBranchPathModel(CUuidString& leaf, CUuidString& branch, ItemModel* model);

void RemoveItem(ItemModel* model, const QUuid& node_id);

void UpdateModel(const QHash<QUuid, QString>& leaf_path, ItemModel* leaf_path_model, const Node* node);
void UpdatePathSeparator(CString& old_separator, CString& new_separator, QHash<QUuid, QString>& source_path);

void UpdateModelFunction(ItemModel* model, const QSet<QUuid>& update_range, CUuidString& source_path);

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

// Update a QString or int field of an object and update the change in a QJsonObject.
// Returns true if the value was changed.
template <typename Field, typename T, typename F = std::nullptr_t>
bool UpdateField(QJsonObject& update, T* object, CString& field, const Field& value, Field T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    Field& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    update.insert(field, value);

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}

template <typename Field, typename T, typename F = std::nullptr_t>
bool UpdateDouble(QJsonObject& update, T* object, CString& field, const Field& value, Field T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    Field& current_value { object->*member };

    if (FloatEqual(current_value, value))
        return false;

    current_value = value;
    update.insert(field, QString::number(value, 'f', kMaxNumericScale_8));

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}

template <typename T, typename F = std::nullptr_t>
bool UpdateDocument(QJsonObject& update, T* object, CString& field, const QStringList& value, QStringList T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    QStringList& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    update.insert(field, value.join(kSemicolon));

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}

template <typename T, typename F = std::nullptr_t>
bool UpdateUuid(QJsonObject& update, T* object, CString& field, const QUuid& value, QUuid T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    QUuid& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    update.insert(field, value.toString(QUuid::WithoutBraces));

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}

template <typename T, typename F = std::nullptr_t>
bool UpdateIssuedTime(QJsonObject& update, T* object, CString& field, const QDateTime& value, QDateTime T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    QDateTime& current_value { object->*member };

    if (current_value == value)
        return false;

    current_value = value;
    update.insert(field, value.toString(Qt::ISODate));

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}
};

#endif // NODEUTILS_H
