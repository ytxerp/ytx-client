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

#ifndef NODEENUM_H
#define NODEENUM_H

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
    kUnitPrice,
    kColor,
    kCommission,
    kDirectionRule,
    kKind,
    kUnit,
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
    kIssuedTime,
    kColor,
    kDocument,
    kStatus,
    kDirectionRule,
    kKind,
    kUnit,
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
    kPartner,
    kIssuedTime,
    kDirectionRule,
    kDescription,
    kEmployee,
    kStatus,
    kKind,
    kUnit,
    kCountTotal,
    kMeasureTotal,
    kInitialTotal,
    kDiscountTotal,
    kFinalTotal,
    kSettlementId,
};

#endif // NODEENUM_H
