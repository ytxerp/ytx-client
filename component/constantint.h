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

#ifndef CONSTANTINT_H
#define CONSTANTINT_H

#include <QtCore/qtypes.h>

namespace UiConst {
inline constexpr int kRowHeight = 24;
inline constexpr int kCornerRadius = 2; // Corner radius of the tag/color rectangle
inline constexpr int kTextMarginFactor = 8;

inline constexpr int kTagHPadding = 4; // Horizontal padding inside each tag (distance from text to left/right edge of color block)
inline constexpr int kTagVPadding = 2; // Vertical padding inside each tag (distance from text to top/bottom edge of color block)
inline constexpr int kTagSpacing = 6; // Horizontal spacing between adjacent tags
inline constexpr int kTagMargin = 6; // Horizontal margin between tag group and cell border
}

namespace NumericConst {
inline constexpr int kDecimalPlaces4 = 4;
inline constexpr int kDecimalPlaces8 = 8;
}

namespace TimeConst {
inline constexpr int kHeartbeatIntervalMs = 30000;
inline constexpr int kTimeoutThresholdMs = 75000;
inline constexpr int kAutoCloseMs = 3000;
inline constexpr int kCooldownMs = 2000;
}

namespace PoolConst {
inline constexpr qsizetype kExpandSize = 100;
inline constexpr qsizetype kMaxSize = 1000;
}

namespace ZstdConst {
inline constexpr qsizetype kCompressThreshold { 256 };
}

#endif // CONSTANTINT_H
