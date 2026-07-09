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

// NOTE: All enums in this file represent transient, in-memory client-side
// state only. None of these values are persisted to the database or sent
// as part of the domain data itself — they exist purely to track the
// runtime status of the connection, session, or local objects awaiting
// server confirmation. Do not serialize them as part of ReadJson/WriteJson
// or any other data persistence path.
//
// Naming convention: enums suffixed with `State` are transient/runtime-only
// (this file). Enums suffixed with `Status` (e.g. NodeStatus, SettlementStatus)
// represent persisted business/domain fields that are stored in the database
// and/or serialized via ReadJson/WriteJson. Keep this distinction consistent
// when adding new enums.

#ifndef STATEENUM_H
#define STATEENUM_H

enum class ConnectionState {
    Connecting,
    Connected,
    Disconnected,
};

enum class LoginState { LoggedIn, LoggedOut };

// Tracks whether a locally held object (Node, Entry, etc.) is in sync with
// the server. This is purely a client-side bookkeeping state used to drive
// UI feedback (e.g. disabling edits, showing visual cues) and to guard
// against operating on data whose server-side outcome is not yet known.
enum class SyncState {
    kCreating,
    kSynced,
    kUpdating,
    kDeleting,
    kError,
};

#endif // STATEENUM_H
