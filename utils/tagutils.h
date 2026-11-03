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

#include <QColor>
#include <QString>
#include <QUuid>

#include "tag/tagrow.h"

namespace tag {

struct SearchQuery final {
    QString text {}; // Normal search text (without [tag])
    QSet<QString> tags {}; // Tag names or tag IDs (depending on your mapping)
};

SearchQuery ParseSearchQuery(const QString& input, const QHash<QUuid, Row>& tag_hash);
QColor GetContrastColor(const QColor& bg_color);
QIcon CreateIcon(const Row& tag, bool checked);
QPixmap CreatePixmap(const Row& tag);

}
