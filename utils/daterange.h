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

#ifndef DATERANGE_H
#define DATERANGE_H

#include <QDateTime>

#include "component/constant.h"

namespace utils {

struct DateTimeRange final {
    QDateTime start {};
    QDateTime end {};

    QString ToString() const { return QStringLiteral("[%1, %2)").arg(start.toString(Qt::ISODate), end.toString(Qt::ISODate)); }
};

struct DateRange final {
    QDate start {};
    QDate end {};

    bool IsValid() const noexcept { return start.isValid() && end.isValid() && start <= end; }

    // User-facing range.
    // Represents a closed interval [start, end].
    // Convert to query range [start, end) before sending to server.
    DateTimeRange ToQueryRange() const
    {
        Q_ASSERT(IsValid());

        return { QDateTime(start, kStartTime).toUTC(), QDateTime(end.addDays(1), kStartTime).toUTC() };
    }

    QString ToString() const { return QStringLiteral("[%1, %2]").arg(start.toString(Qt::ISODate), end.toString(Qt::ISODate)); }
};

}

#endif // DATERANGE_H
