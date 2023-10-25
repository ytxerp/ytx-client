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

#ifndef ENUMCLASS_H
#define ENUMCLASS_H

// TODO: 改为 enum class
enum NodeKind { kBranch = 0, kLeaf };

// - 4 corresponds to kSale
// - 5 corresponds to kPurchase
// Section values must start from 0 to maintain consistency.
// Abbreviations: Finance -> F, Item -> I, Task -> T, Stakeholder -> S, Order -> O
enum class Section { kFinance = 0, kItem, kTask, kStakeholder, kSale, kPurchase };

// defining section's unit
enum class UnitO { kIS = 0, kMS, kPEND };
enum class UnitS { kCust = 0, kEmp, kVend };
enum class UnitI { kInternal = 0, kPlaceholder, kExternal };
enum class UnitT { kExternal = 0, kPlaceholder, kInternal };

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
    kIsChecked,
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
    kIsChecked,
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
    kIsChecked,
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
    kIsChecked,
    kRhsNode,
    kDebit,
    kCredit,
    kBalance,
};

// FullEntryEnumS is same as EntryEnumS
enum class EntryEnumS {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kLhsNode,
    kUnitPrice,
    kCode,
    kDescription,
    kDocument,
    kIsChecked,
    kRhsNode,
    kExternalItem,
};

// FullEntryEnumO is same as EntryEnumO
enum class EntryEnumO {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kRhsNode,
    kFirst,
    kSecond,
    kUnitPrice,
    kDescription,
    kExternalItem,
    kLhsNode,
    kDiscountPrice,
    kColor,
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
    kIsChecked,
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
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kUnitCost,
    kDocument,
    kIsChecked,
    kRhsCredit,
    kRhsDebit,
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
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kUnitCost,
    kDocument,
    kIsChecked,
    kRhsCredit,
    kRhsDebit,
    kRhsNode,
};

// defining node column
enum class NodeEnum {
    kName = 0,
    kId,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kCode,
    kDescription,
    kNote,
    kDirectionRule,
    kKind,
    kUnit,
};

enum class NodeEnumF {
    kName = 0,
    kId,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kCode,
    kDescription,
    kNote,
    kDirectionRule,
    kKind,
    kUnit,
    kInitialTotal,
    kFinalTotal,
};

enum class NodeEnumI {
    kName = 0,
    kId,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kCode,
    kDescription,
    kNote,
    kDirectionRule,
    kKind,
    kUnit,
    kColor,
    kUnitPrice,
    kCommission,
    kInitialTotal,
    kFinalTotal,
};

enum class NodeEnumT {
    kName = 0,
    kId,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kCode,
    kDescription,
    kNote,
    kDirectionRule,
    kKind,
    kUnit,
    kColor,
    kDocument,
    kIssuedTime,
    kIsFinished,
    kInitialTotal,
    kFinalTotal,
};

enum class NodeEnumS {
    kName = 0,
    kId,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kCode,
    kDescription,
    kNote,
    kPaymentTerm,
    kKind,
    kUnit,
    kInitialTotal,
    kFinalTotal,
};

enum class NodeEnumO {
    kName = 0,
    kId,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kDescription,
    kDirectionRule,
    kKind,
    kUnit,
    kParty,
    kEmployee,
    kIssuedTime,
    kFirstTotal,
    kSecondTotal,
    kIsFinished,
    kInitialTotal,
    kDiscountTotal,
    kFinalTotal,
    kSettlementId,
};

// P:Previous, C:Current, Statement
enum class StatementEnum { kParty = 0, kPBalance, kCFirst, kCSecond, kCGrossAmount, kCBalance, kPlaceholder, kCSettlement };

enum class StatementPrimaryEnum { kIssuedTime = 0, kFirst, kSecond, kInitialTotal, kIsChecked, kDescription, kEmployee, kFinalTotal };

enum class StatementSecondaryEnum {
    kIssuedTime = 0,
    kRhsNode,
    kFirst,
    kSecond,
    kUnitPrice,
    kInitialTotal,
    kIsChecked,
    kDescription,
    kSupportNode,
    kFinalTotal
};

// Settlement
enum class SettlementEnum {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kStakeholder,
    kDescription,
    kIsFinished,
    kInitialTotal,
};

// kPIId: kParty or kItem
enum class EntryRefEnum {
    kIssuedTime = 0,
    kOrderId,
    kSection,
    kPIId,
    kExternalItem,
    kFirst,
    kSecond,
    kUnitPrice,
    kDiscountPrice,
    kDescription,
    kInitial,
};

// Enum class defining check options
enum class Check { kOff = 0, kOn, kFlip };

#endif // ENUMCLASS_H
