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

#ifndef ENTRYUTILS_H
#define ENTRYUTILS_H

#include <QJsonObject>

#include "component/constant.h"
#include "component/using.h"
#include "enum/entryenum.h"
#include "enum/section.h"

namespace Utils {

constexpr int LinkedNodeColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
    case Section::kTask:
    case Section::kInventory:
        return std::to_underlying(EntryEnum::kRhsNode);
    case Section::kPartner:
        return std::to_underlying(EntryEnumP::kRhsNode);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(EntryEnumO::kRhsNode);
    }

    Q_UNREACHABLE();
}

constexpr int EntryDescriptionColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
    case Section::kInventory:
    case Section::kTask:
        return std::to_underlying(EntryEnum::kDescription);
    case Section::kPartner:
        return std::to_underlying(EntryEnumP::kDescription);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(EntryEnumO::kDescription);
    }

    Q_UNREACHABLE();
}

constexpr int SearchEntryDescriptionColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
    case Section::kInventory:
    case Section::kTask:
        return std::to_underlying(FullEntryEnum::kDescription);
    case Section::kPartner:
        return std::to_underlying(EntryEnumP::kDescription);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(EntryEnumO::kDescription);
    }

    Q_UNREACHABLE();
}

constexpr int BalanceColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
    case Section::kTask:
    case Section::kInventory:
        return std::to_underlying(EntryEnum::kBalance);
    case Section::kPartner:
    case Section::kSale:
    case Section::kPurchase:
        return -1;
    }

    Q_UNREACHABLE();
}

constexpr std::pair<int, int> EntryCacheColumnRange(Section section)
{
    switch (section) {
    case Section::kFinance:
    case Section::kInventory:
    case Section::kTask:
        return { std::to_underlying(EntryEnum::kCode), std::to_underlying(EntryEnum::kStatus) };
    case Section::kPartner:
        return { std::to_underlying(EntryEnumP::kIssuedTime), std::to_underlying(EntryEnumP::kRhsNode) };
    case Section::kSale:
    case Section::kPurchase:
        return { std::to_underlying(EntryEnumO::kDescription), std::to_underlying(EntryEnumO::kExternalSku) };
    }

    Q_UNREACHABLE();
}

constexpr std::pair<int, int> EntryNumericColumnRange(Section section)
{
    switch (section) {
    case Section::kFinance:
    case Section::kTask:
    case Section::kInventory:
        return { std::to_underlying(EntryEnum::kDebit), std::to_underlying(EntryEnum::kCredit) };
    case Section::kPartner:
        return { -1, -1 };
    case Section::kSale:
    case Section::kPurchase:
        return { std::to_underlying(EntryEnumO::kInitial), std::to_underlying(EntryEnumO::kFinal) };
    }

    Q_UNREACHABLE();
}

template <typename T, typename F = std::nullptr_t>
bool UpdateShadowIssuedTime(QJsonObject& update, T* object, CString& field, const QDateTime& value, QDateTime* T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    QDateTime* current_value { object->*member };

    if (*current_value == value)
        return false;

    *current_value = value;

    if (!object->rhs_node || object->rhs_node->isNull()) {
        return true;
    }

    update.insert(field, value.toString(Qt::ISODate));

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}

template <typename T, typename F = std::nullptr_t>
bool UpdateShadowDocument(QJsonObject& update, T* object, CString& field, const QStringList& value, QStringList* T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    QStringList* member_ptr { object->*member };

    if (!member_ptr) {
        qWarning() << "Member pointer must not be null.";
        return false;
    }

    if (*member_ptr == value)
        return false;

    *member_ptr = value;

    // If rhs_node is invalid, skip updating
    if (!object->rhs_node || object->rhs_node->isNull()) {
        return true;
    }

    update.insert(field, value.join(kSemicolon));

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}

template <typename Field, typename T, typename F = std::nullptr_t>
bool UpdateShadowField(QJsonObject& update, T* object, CString& field, const Field& value, Field* T::* member, F&& restart_timer = nullptr)
{
    assert(object);

    Field* member_ptr { object->*member };

    if (!member_ptr) {
        qWarning() << "Member pointer must not be null.";
        return false;
    }

    if (*member_ptr == value)
        return false;

    *member_ptr = value;

    // If rhs_node is invalid, skip updating
    if (!object->rhs_node || object->rhs_node->isNull()) {
        return true;
    }

    update.insert(field, value);

    if constexpr (!std::is_same_v<std::decay_t<F>, std::nullptr_t>) {
        restart_timer();
    }

    return true;
}
};

#endif // ENTRYUTILS_H
