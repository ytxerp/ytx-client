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

#include <QString>
#include <span>

#include "enum/section.h"

namespace reimbursement_permission {
inline constexpr int kApprove = 1 << 2;
inline constexpr int kUnapprove = 1 << 3;
inline constexpr int kPost = 1 << 4;
inline constexpr int kUnpost = 1 << 5;
}

namespace order_permission {
inline constexpr int kRelease = 1 << 2;
inline constexpr int kUnrelease = 1 << 3;
}

namespace section {

struct Permissions {
    int finance {};
    int task {};
    int inventory {};
    int partner {};
    int sale {};
    int purchase {};
    int reimbursement {};
};

struct PermissionItem {
    int permission {};
    QString text {};
};

std::span<const PermissionItem> PermissionItems(Section section);
QString PermissionsDisplay(Section section, int permissions);
}
