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

#include <array>

// TODO: 改为 enum class
enum NodeKind { kLeaf = 0, kBranch };

// - 4 corresponds to kSale
// - 5 corresponds to kPurchase
// Section values must start from 0 to maintain consistency.
// Abbreviations: Finance -> F, Inventory -> I, Task -> T, Partner -> P, Order -> O
enum class Section { kFinance = 0, kTask, kInventory, kPartner, kSale, kPurchase };

inline constexpr std::array<Section, 6> kSections
    = { Section::kFinance, Section::kTask, Section::kInventory, Section::kPartner, Section::kSale, Section::kPurchase };

// defining section's unit
enum class UnitO { kImmediate = 0, kMonthly, kPending };
enum class UnitS { kCustomer = 0, kVendor, kEmployee };
enum class UnitI { kInternal = 0, kPosition, kExternal };
enum class UnitT { kExternal = 0, kInternal };

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
    kUnitPrice,
    kCode,
    kDescription,
    kDocument,
    kStatus,
    kRhsNode,
    kExternalSku,
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
    kDiscountPrice,
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
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kUnitCost,
    kDocument,
    kStatus,
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
    kStatus,
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
    kStatus,
    kInitialTotal,
    kFinalTotal,
};

enum class NodeEnumP {
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
    kIssuedTime,
    kDescription,
    kDirectionRule,
    kKind,
    kUnit,
    kEmployee,
    kCountTotal,
    kMeasureTotal,
    kStatus,
    kInitialTotal,
    kDiscountTotal,
    kFinalTotal,
    kPartner,
    kSettlementId,
};

// P:Previous, C:Current, Statement
enum class StatementEnum { kPartner = 0, kPBalance, kCCount, kCMeasure, kCGrossAmount, kCBalance, kPlaceholder, kCSettlement };

enum class StatementPrimaryEnum { kIssuedTime = 0, kCount, kMeasure, kInitialTotal, kStatus, kDescription, kEmployee, kFinalTotal };

enum class StatementSecondaryEnum { kIssuedTime = 0, kRhsNode, kCount, kMeasure, kUnitPrice, kInitialTotal, kStatus, kDescription, kSupportNode, kFinalTotal };

// Settlement
enum class SettlementEnum {
    kId = 0,
    kUserId,
    kCreateTime,
    kCreateBy,
    kUpdateTime,
    kUpdateBy,
    kIssuedTime,
    kPartner,
    kDescription,
    kStatus,
    kInitialTotal,
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
    kDiscountPrice,
    kDescription,
    kInitial,
};

// Enum class defining entry actions
enum class EntryAction { kMarkNone = 0, kMarkAll, kMarkToggle };

enum class EntryStatus { kUnmarked = 0, kMarked };
enum class NodeStatus { kUnfinished = 0, kReviewed, kVoided };

#endif // ENUMCLASS_H
