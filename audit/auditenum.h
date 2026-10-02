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

#pragma once

namespace audit {

enum class TargetType {
    kNode = 0,
    kEntry = 1,
    kSettlement = 2,
};

enum class TargetOperation {
    kInsert = 0,
    kUpdate = 1,
    kDelete = 2,
    kMove = 3,
    kReplace = 4,
    kRelease = 5,
    kRecall = 6,
    kPeriodClose = 7,
};

enum class RowField {
    kTargetId,
    kUsername,
    kLhsNodeName,
    kRhsNodeName,
    kCreatedTime,
    kTargetCode,
    kTargetType,
    kTargetOperation,
    kTargetField,
    kPlaceholder,
    kBefore,
    kAfter,
};

enum class TargetField {
    kNone = 0,

    // --- Node ---
    kName = 1,
    kDirectionRule = 2,

    // --- Entry ---
    kNumeric = 3,
    kRate = 4,
    kLinkedNode = 5,

    // --- Common ---
    kStatus = 6,
    kContent = 7,
};

}
