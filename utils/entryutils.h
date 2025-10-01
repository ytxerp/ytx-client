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
#include "component/enumclass.h"
#include "component/using.h"

namespace EntryUtils {

inline int LinkedNodeColumn(Section section)
{
    switch (section) {
    case Section::kFinance:
        return std::to_underlying(EntryEnumF::kRhsNode);
    case Section::kTask:
        return std::to_underlying(EntryEnumT::kRhsNode);
    case Section::kPartner:
        return std::to_underlying(EntryEnumP::kRhsNode);
    case Section::kInventory:
        return std::to_underlying(EntryEnumI::kRhsNode);
    case Section::kSale:
    case Section::kPurchase:
        return std::to_underlying(EntryEnumO::kRhsNode);
    default:
        return -1;
    }
}

template <typename T>
bool UpdateShadowIssuedTime(
    QJsonObject& cache, T* object, CString& field, const QDateTime& value, QDateTime* T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    QDateTime* current_value { object->*member };

    if (*current_value == value)
        return false;

    *current_value = value;

    if (!object->rhs_node || object->rhs_node->isNull()) {
        return true;
    }

    cache.insert(field, value.toString(Qt::ISODate));
    if (restart_timer)
        restart_timer();

    return true;
}

template <typename T>
bool UpdateShadowUuid(QJsonObject& cache, T* object, CString& field, const QUuid& value, QUuid* T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    QUuid* current_value { object->*member };

    if (*current_value == value)
        return false;

    *current_value = value;

    if (!object->rhs_node || object->rhs_node->isNull()) {
        return true;
    }

    cache.insert(field, value.toString(QUuid::WithoutBraces));
    if (restart_timer)
        restart_timer();

    return true;
}

template <typename T>
bool UpdateShadowDocument(
    QJsonObject& cache, T* object, CString& field, const QStringList& value, QStringList* T::* member, std::function<void()> restart_timer = nullptr)
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

    cache.insert(field, value.join(kSemicolon));
    if (restart_timer)
        restart_timer();

    return true;
}

template <typename Field, typename T>
bool UpdateShadowDouble(QJsonObject& cache, T* object, CString& field, const Field& value, Field* T::* member, std::function<void()> restart_timer = nullptr)
{
    assert(object);

    Field* member_ptr { object->*member };

    if (std::abs(*member_ptr - value) < kTolerance)
        return false;

    *member_ptr = value;

    // If rhs_node is invalid, skip updating
    if (!object->rhs_node || object->rhs_node->isNull()) {
        return true;
    }

    cache.insert(field, QString::number(value, 'f', kMaxNumericScale_4));
    if (restart_timer)
        restart_timer();

    return true;
}

template <typename Field, typename T>
bool UpdateShadowField(QJsonObject& cache, T* object, CString& field, const Field& value, Field* T::* member, std::function<void()> restart_timer = nullptr)
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

    cache.insert(field, value);
    if (restart_timer)
        restart_timer();

    return true;
}
};

#endif // ENTRYUTILS_H
