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

#ifndef CONSTANTDOUBLE_H
#define CONSTANTDOUBLE_H

#include <cstdlib>
#include <limits>

inline constexpr double kTolerance = 1e-8;
inline constexpr double kDoubleMax = std::numeric_limits<double>::max();
inline constexpr double kDoubleLowest = std::numeric_limits<double>::lowest();

inline bool FloatEqual(double a, double b) noexcept { return std::abs(a - b) < kTolerance; }
inline bool FloatChanged(double a, double b) noexcept { return !FloatEqual(a, b); }

#endif // CONSTANTDOUBLE_H
