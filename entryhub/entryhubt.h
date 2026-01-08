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

#ifndef ENTRYHUBT_H
#define ENTRYHUBT_H

#include "entryhub.h"

class EntryHubT final : public EntryHub {
public:
    explicit EntryHubT(CSectionInfo& info, QObject* parent = nullptr);

public:
    void UpdateEntryRate(const QUuid& entry_id, const QJsonObject& update, bool is_parallel) override;
    void UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& update) override;
};

#endif // ENTRYHUBT_H
