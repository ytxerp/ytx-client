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

#ifndef SECTION_H
#define SECTION_H

#include <array>

#include "component/constant.h"

/**
 * @brief Enumerates all functional sections (modules) in the system.
 *
 * The integer values are continuous starting from 0, and MUST remain continuous
 * because they are used as array indices and may also be stored in the database.
 *
 * Sections:
 *  - kFinance   (0) : Accounting / double-entry finance module
 *  - kTask      (1) : Task management (double-entry: lhs/rhs operations)
 *  - kInventory (2) : Inventory / stock control (double-entry: in/out flow)
 *  - kPartner   (3) : Partner / stakeholder information (single-entry)
 *  - kSale      (4) : Sales order module (single-entry)
 *  - kPurchase  (5) : Purchase order module (single-entry)
 *
 * Abbreviations:
 *  - Finance (F)
 *  - Inventory (I)
 *  - Task (T)
 *  - Partner (P)
 *  - Order (O)
 *
 * Notes:
 *  - Double-entry modules always involve both LHS / RHS or dual nodes.
 *  - Single-entry modules store only one-sided entries.
 */
enum class Section { kFinance = 0, kTask, kInventory, kPartner, kSale, kPurchase };

/**
 * @brief Constant array containing all defined sections in proper order.
 *
 * This array is used for iteration and must match the enum order exactly.
 */
inline constexpr std::array<Section, 6> kSectionArray
    = { Section::kFinance, Section::kTask, Section::kInventory, Section::kPartner, Section::kSale, Section::kPurchase };

/**
 * @brief Whether the given section belongs to a double-entry module.
 *
 * Double-entry modules involve two-sided (LHS / RHS) records and require
 * synchronized or balanced relationships, similar to double-entry accounting.
 */
inline bool IsDoubleEntry(Section s) { return s == Section::kFinance || s == Section::kInventory || s == Section::kTask; }

/**
 * @brief Whether the given section belongs to a single-entry module.
 *
 * Single-entry modules describe one-sided records only and do not require
 * dual-node synchronization like the double-entry modules.
 *
 * Note:
 *   This is the complement of IsDoubleEntry(), but is explicitly defined
 *   for clarity and readability.
 */
inline bool IsSingleEntry(Section s) { return s == Section::kPartner || s == Section::kSale || s == Section::kPurchase; }

/**
 * @brief Whether the given section represents an order-type module.
 *
 * Order modules include:
 *  - kSale     : Sales order
 *  - kPurchase : Purchase order
 *
 * They share common behaviors such as order flow, document structure,
 * and are often processed together in business logic.
 */
inline bool IsOrderSection(Section s) { return s == Section::kSale || s == Section::kPurchase; }

/**
 * @brief Maps Section → QString (name constants).
 *
 * Used for serialization, display text, and conversion to string form.
 * Keys must remain unique.
 */
inline const QHash<Section, QString> kSectionString {
    { Section::kFinance, kFinance },
    { Section::kPartner, kPartner },
    { Section::kInventory, kInventory },
    { Section::kTask, kTask },
    { Section::kSale, kSale },
    { Section::kPurchase, kPurchase },
};

/**
 * @brief Maps QString → Section (reverse conversion).
 *
 * Used for parsing, deserialization, and loading from storage.
 * The string constants (kFinance, kSale, etc.) must be unique and stable.
 */
inline const QHash<QString, Section> kStringSection {
    { kFinance, Section::kFinance },
    { kPartner, Section::kPartner },
    { kInventory, Section::kInventory },
    { kTask, Section::kTask },
    { kSale, Section::kSale },
    { kPurchase, Section::kPurchase },
};

#endif // SECTION_H
