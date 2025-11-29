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

// defining entry column
enum class EntryEnum {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kLhsNode,
    kLhsRate,
    kCode,
    kDescription,
    kDocument,
    kStatus,
    kRhsNode,
    kDebit,
    kCredit,
    kBalance,
};

enum class EntryEnumF {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kLhsNode,
    kLhsRate,
    kCode,
    kDescription,
    kDocument,
    kStatus,
    kRhsNode,
    kDebit,
    kCredit,
    kBalance,
};

enum class EntryEnumI {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kLhsNode,
    kUnitCost,
    kCode,
    kDescription,
    kDocument,
    kStatus,
    kRhsNode,
    kDebit,
    kCredit,
    kBalance,
};

enum class EntryEnumT {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kLhsNode,
    kUnitCost,
    kCode,
    kDescription,
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
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kLhsNode,
    kCode,
    kDescription,
    kDocument,
    kStatus,
    kExternalSku,
    kUnitPrice,
    kRhsNode,
};

// FullEntryEnumO is same as EntryEnumO
enum class EntryEnumO {
    kId = 0,
    kRhsNode,
    kCount,
    kMeasure,
    kUnitPrice,
    kDescription,
    kExternalSku,
    kLhsNode,
    kUnitDiscount,
    kInitial,
    kDiscount,
    kFinal,
};

enum class FullEntryEnumF {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kCode,
    kLhsNode,
    kLhsRate,
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kDocument,
    kStatus,
    kRhsCredit,
    kRhsDebit,
    kRhsRate,
    kRhsNode,
};

enum class FullEntryEnumI {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kCode,
    kLhsNode,
    kLhsRate,
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kDocument,
    kStatus,
    kRhsCredit,
    kRhsDebit,
    kRhsRate,
    kRhsNode,
};

enum class FullEntryEnumT {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kCode,
    kLhsNode,
    kLhsRate,
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kDocument,
    kStatus,
    kRhsCredit,
    kRhsDebit,
    kRhsRate,
    kRhsNode,
};

// kPIId: kPartner or kInventory
enum class EntryRefEnum {
    kIssuedTime = 0,
    kOrderId,
    kSection,
    kPIId,
    kExternalSku,
    kkCount,
    kkMeasure,
    kUnitPrice,
    kUnitDiscount,
    kDescription,
    kInitial,
};

#endif // ENTRYENUM_H
