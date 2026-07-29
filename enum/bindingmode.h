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

#ifndef BINDINGMODE_H
#define BINDINGMODE_H

enum class BindingMode {
    kParallel,
    kCross,
};

/// Indicates which side of the Entry is the input side for this operation.
/// The server always treats this side as the source of the update and derives
/// the opposite side accordingly.
enum class InputSide {
    kLhs,
    kRhs,
};

constexpr InputSide ToValueInputSide(BindingMode mode)
{
    switch (mode) {
    case BindingMode::kParallel:
        return InputSide::kLhs;

    case BindingMode::kCross:
        return InputSide::kRhs;
    }
}

constexpr InputSide ToLinkedNodeInputSide(BindingMode mode)
{
    switch (mode) {
    case BindingMode::kParallel:
        return InputSide::kRhs;

    case BindingMode::kCross:
        return InputSide::kLhs;
    }
}

#endif // BINDINGMODE_H
