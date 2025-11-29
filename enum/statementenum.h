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

#ifndef STATEMENTENUM_H
#define STATEMENTENUM_H

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

#endif // STATEMENTENUM_H
