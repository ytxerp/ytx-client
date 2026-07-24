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

// NOTE: Enums in this file (suffixed with `Status`) represent persisted
// business/domain state. They are written to and read from the database,
// and participate in ReadJson/WriteJson serialization as part of the
// domain data itself. Do not confuse with `State`-suffixed enums
// (see StateEnum.h), which are transient client-side runtime state only
// and must never be persisted.

#ifndef STATUSENUM_H
#define STATUSENUM_H

/** @brief Workflow status of an order node. */
enum class NodeStatus { kUnreleased = 0, kReleased, kRecalled };

/** @brief Workflow status of a settlement bill. */
enum class SettlementStatus { kRecalled = 0, kReleased };

/** @brief EntryStatus of an entry. */
enum class EntryStatus { kUnmarked = 0, kMarked };

#endif // STATUSENUM_H
