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

#ifndef SETTLEMENTENUM_H
#define SETTLEMENTENUM_H

enum class SettlementStatus { kRecalled = 0, kReleased };

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
    kAmount,
};

enum class SettlementNodeEnum {
    kId = 0,
    kIssuedTime,
    kAmount,
    kIsSettled,
    kPartner,
    kDescription,
    kEmployee,
};

#endif // SETTLEMENTENUM_H
