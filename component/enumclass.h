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

enum NodeType { kTypeLeaf, kTypeBranch, kTypeSupport };

// Define section values, starting from 0. In QSReadTransRef and delegate-SectionR:
// - 4 corresponds to kSales
// - 5 corresponds to kPurchase
// Section values must start from 0 to maintain consistency.
enum class Section { kFinance, kProduct, kTask, kStakeholder, kSales, kPurchase };

// Abbreviations: Finance -> F, Product -> P, Task -> T, Stakeholder -> S, Order -> O

// defining section's unit

enum class UnitO { kIS, kMS, kPEND };

enum class UnitS { kCust, kEmp, kVend };

enum class UnitP { kPos = 1 };

enum class UnitT { kProd = 1 };

// defining trans column
enum class TransEnum { kIssuedTime, kID, kLhsRatio, kCode, kDescription, kSupportID, kDocument, kIsChecked, kRhsNode };

enum class TransEnumF { kIssuedTime, kID, kLhsRatio, kCode, kDescription, kSupportID, kDocument, kIsChecked, kRhsNode, kDebit, kCredit, kSubtotal };

enum class TransEnumT { kIssuedTime, kID, kUnitCost, kCode, kDescription, kSupportID, kDocument, kIsChecked, kRhsNode, kDebit, kCredit, kSubtotal };

enum class TransEnumP { kIssuedTime, kID, kUnitCost, kCode, kDescription, kSupportID, kDocument, kIsChecked, kRhsNode, kDebit, kCredit, kSubtotal };

enum class TransEnumS { kIssuedTime, kID, kUnitPrice, kCode, kDescription, kOutsideProduct, kDocument, kIsChecked, kInsideProduct };

enum class TransEnumO {
    kInsideProduct,
    kID,
    kOutsideProduct,
    kFirst,
    kSecond,
    kUnitPrice,
    kDiscountPrice,
    kDescription,
    kCode,
    kColor,
    kGrossAmount,
    kDiscount,
    kNetAmount
};

enum class TransSearchEnum {
    kIssuedTime,
    kID,
    kCode,
    kLhsNode,
    kLhsRatio,
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kSupportID,
    kDiscount,
    kDocument,
    kIsChecked,
    kRhsCredit,
    kRhsDebit,
    kRhsRatio,
    kRhsNode
};

// kPP: kParty or kProduct
enum class TransRefEnum {
    kIssuedTime,
    kOrderNode,
    kSection,
    kPP,
    kOutsideProduct,
    kFirst,
    kSecond,
    kUnitPrice,
    kDiscountPrice,
    kDescription,
    kGrossAmount,
};

// P:Previous, C:Current
enum class StatementEnum { kParty, kPBalance, kCFirst, kCSecond, kCGrossAmount, kCBalance, kPlaceholder, kCSettlement };

enum class StatementSecondaryEnum {
    kIssuedTime,
    kInsideProduct,
    kFirst,
    kSecond,
    kUnitPrice,
    kGrossAmount,
    kIsChecked,
    kDescription,
    kOutsideProduct,
    kSettlement,
};

// defining node column
enum class NodeEnum { kName, kID, kCode, kDescription, kNote, kNodeType, kDirectionRule, kUnit };

enum class NodeEnumF { kName, kID, kCode, kDescription, kNote, kNodeType, kDirectionRule, kUnit, kForeignTotal, kLocalTotal };

enum class NodeEnumT {
    kName,
    kID,
    kCode,
    kDescription,
    kNote,
    kNodeType,
    kDirectionRule,
    kUnit,
    kIssuedTime,
    kColor,
    kDocument,
    kIsFinished,
    kUnitCost,
    kQuantity,
    kAmount
};

enum class NodeEnumP { kName, kID, kCode, kDescription, kNote, kNodeType, kDirectionRule, kUnit, kColor, kUnitPrice, kCommission, kQuantity, kAmount };

enum class NodeEnumS { kName, kID, kCode, kDescription, kNote, kNodeType, kPaymentTerm, kUnit, kDeadline, kEmployee, kTaxRate, kAmount };

enum class NodeEnumO {
    kName,
    kID,
    kParty,
    kDescription,
    kEmployee,
    kNodeType,
    kDirectionRule,
    kUnit,
    kIssuedTime,
    kFirst,
    kSecond,
    kIsFinished,
    kGrossAmount,
    kDiscount,
    kSettlement
};

enum class NodeSearchEnum {
    kName,
    kID,
    kCode,
    kDescription,
    kNote,
    kNodeType,
    kDirectionRule,
    kUnit,
    kParty,
    kEmployee,
    kIssuedTime,
    kColor,
    kDocument,
    kFirst,
    kSecond,
    kDiscount,
    kIsFinished,
    kInitialTotal,
    kFinalTotal
};

enum class StatementPrimaryEnum { kIssuedTime, kFirst, kSecond, kGrossAmount, kIsChecked, kDescription, kEmployee, kSettlement };

enum class SettlementEnum { kIssuedTime, kID, kParty, kDescription, kIsFinished, kGrossAmount };

// Enum class defining check options
enum class Check { kNone, kAll, kReverse };

#endif // ENUMCLASS_H
