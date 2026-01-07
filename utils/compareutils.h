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

#ifndef COMPAREUTILS_H
#define COMPAREUTILS_H

#include <QtCore/qnamespace.h>

#include "global/collator.h"

namespace Utils {

template <typename Obj, typename T> inline bool CompareMember(const Obj* lhs, const Obj* rhs, const T Obj::* member, Qt::SortOrder order)
{
    if constexpr (std::is_same_v<T, QString>) {
        // QString comparison using collator
        const auto& collator = Collator::Instance();
        const int r = collator.compare(lhs->*member, rhs->*member);
        return (order == Qt::AscendingOrder) ? (r < 0) : (r > 0);
    } else {
        // Regular comparison for other types
        return (order == Qt::AscendingOrder) ? (lhs->*member < rhs->*member) : (lhs->*member > rhs->*member);
    }
}

} // namespace Utils

#endif // COMPAREUTILS_H
