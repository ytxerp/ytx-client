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

#ifndef CONSTANTBOOL_H
#define CONSTANTBOOL_H

// Boolean constants
inline constexpr bool kIsHidden = true;

namespace Rule {
// Finance/Inventory/Task: Credit increase, Debit decrease; calculation: credit - debit
// Order: Return Order (returned transaction)
inline constexpr bool kDDCI = true;
inline constexpr bool kRO = true;

// Finance/Inventory/Task: Debit increase, Credit decrease; calculation: debit - credit
// Order: Forward Order (normal transaction)
inline constexpr bool kDICD = false;
inline constexpr bool kFO = false;

}

#endif // CONSTANTBOOL_H
