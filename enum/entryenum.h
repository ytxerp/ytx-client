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

#ifndef ENTRYENUM_H
#define ENTRYENUM_H

/** @brief Operation */
enum class Mark { kClear = 0, kSelect, kToggle };

/** @brief Status of an entry. */
enum class Status { kUnmarked = 0, kMarked };

// defining entry column
enum class EntryEnum {
    kId = 0,
    kVersion,
    kIssuedTime,
    kLhsNode,
    kLhsRate,
    kCode,
    kDescription,
    kTag,
    kDocument,
    kStatus,
    kRhsNode,
    kDebit,
    kCredit,
    kBalance,
};

// FullEntryEnumP is same as EntryEnumP
enum class EntryEnumP {
    kId = 0,
    kVersion,
    kIssuedTime,
    kLhsNode,
    kCode,
    kDescription,
    kTag,
    kDocument,
    kStatus,
    kRhsNode,
    kUnitPrice,
    kExternalSku,
};

// FullEntryEnumO is same as EntryEnumO
enum class EntryEnumO {
    kId = 0,
    kVersion,
    kLhsNode,
    kRhsNode,
    kDescription,
    kTag,
    kCount,
    kMeasure,
    kUnitPrice,
    kExternalSku,
    kUnitDiscount,
    kInitial,
    kDiscount,
    kFinal,
};

enum class FullEntryEnum {
    kId = 0,
    kVersion,
    kIssuedTime,
    kCode,
    kLhsNode,
    kLhsRate,
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kTag,
    kDocument,
    kStatus,
    kRhsCredit,
    kRhsDebit,
    kRhsRate,
    kRhsNode,
};

#endif // ENTRYENUM_H
