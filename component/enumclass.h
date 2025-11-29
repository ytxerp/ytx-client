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

/** @brief Kind of node in a tree structure. */
enum class NodeKind { kLeaf = 0, kBranch };

/** @brief Workflow status of a node. Just for Task and Order sections. */
enum class NodeStatus { kRecalled = 0, kReleased };

/** @brief Action applied to all entries in a leaf table widget. */
enum class EntryAction { kMarkNone = 0, kMarkAll, kMarkToggle };

/** @brief Status of an entry. */
enum class EntryStatus { kUnmarked = 0, kMarked };

/** @brief Unit kinds. */
/** @brief Currency kinds, ordered by common usage and trading volume. */
enum class Currency {
    USD = 0, // US Dollar
    EUR = 1, // Euro
    JPY = 2, // Japanese Yen
    GBP = 3, // British Pound
    AUD = 4, // Australian Dollar
    CAD = 5, // Canadian Dollar
    CHF = 6, // Swiss Franc
    CNY = 7, // Chinese Yuan
    HKD = 8, // Hong Kong Dollar
    NZD = 9, // New Zealand Dollar
    SEK = 10, // Swedish Krona
    NOK = 11, // Norwegian Krone
    SGD = 12, // Singapore Dollar
    KRW = 13, // South Korean Won
    MXN = 14, // Mexican Peso
    INR = 15 // Indian Rupee
};

inline const char* currency_code(Currency c)
{
    switch (c) {
    case Currency::USD:
        return "USD";
    case Currency::EUR:
        return "EUR";
    case Currency::JPY:
        return "JPY";
    case Currency::GBP:
        return "GBP";
    case Currency::AUD:
        return "AUD";
    case Currency::CAD:
        return "CAD";
    case Currency::CHF:
        return "CHF";
    case Currency::CNY:
        return "CNY";
    case Currency::HKD:
        return "HKD";
    case Currency::NZD:
        return "NZD";
    case Currency::SEK:
        return "SEK";
    case Currency::NOK:
        return "NOK";
    case Currency::SGD:
        return "SGD";
    case Currency::KRW:
        return "KRW";
    case Currency::MXN:
        return "MXN";
    case Currency::INR:
        return "INR";
    }
}

inline const char* currency_symbol(Currency c)
{
    switch (c) {
    case Currency::USD:
        return "$";
    case Currency::EUR:
        return "€";
    case Currency::JPY:
        return "¥";
    case Currency::GBP:
        return "£";
    case Currency::AUD:
        return "$";
    case Currency::CAD:
        return "$";
    case Currency::CHF:
        return "CHF";
    case Currency::CNY:
        return "¥";
    case Currency::HKD:
        return "$";
    case Currency::NZD:
        return "$";
    case Currency::SEK:
        return "kr";
    case Currency::NOK:
        return "kr";
    case Currency::SGD:
        return "$";
    case Currency::KRW:
        return "₩";
    case Currency::MXN:
        return "$";
    case Currency::INR:
        return "₹";
    }
}

enum class UnitO { kImmediate = 0, kMonthly, kPending };
enum class UnitP { kCustomer = 0, kVendor, kEmployee };
enum class UnitI { kInternal = 0, kPosition, kExternal };

enum class UnitT {
    kTarget = 0, // 0–9 reserved for target types
    kSource = 10, // 10–19 reserved for source types
    kAction = 20, // 20–29 reserved for action types
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
    kUnitDiscount,
    kDescription,
    kInitial,
};

#endif // ENUMCLASS_H
