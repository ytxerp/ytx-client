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

#ifndef AUTHENUM_H
#define AUTHENUM_H

enum class RegisterOutcome {
    Success = 0,
    EmptyEmail = 1,
    EmptyPassword = 2,
    InvalidEmail = 3,
    EmailAlreadyExists = 4,
    ServerError = 5,
};

enum class LoginOutcome {
    Success = 0,
    EmptyEmail = 1,
    EmptyPassword = 2,
    EmailNotFound = 3,
    PasswordIncorrect = 4,
    WorkspaceNotFound = 5,
    WorkspaceExpired = 6,
    WorkspaceAccessPending = 7,
    AlreadyLoggedIn = 8,
    ServerError = 9,
};

#endif // AUTHENUM_H
