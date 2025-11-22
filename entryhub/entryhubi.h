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

#ifndef ENTRYHUBI_H
#define ENTRYHUBI_H

#include "entryhub.h"

class EntryHubI final : public EntryHub {
public:
    EntryHubI(CSectionInfo& info, QObject* parent = nullptr);

public:
    void UpdateEntryRate(const QUuid& entry_id, const QJsonObject& update, bool is_parallel) override;
    void UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& update) override;

protected:
    QString QSReadTransRef(int unit) const override;

private:
    QString QSReplaceLeafSI() const; // partner_node item_node
    QString QSReplaceLeafOSI() const; // order sale_node item_node
    QString QSReplaceLeafOPI() const; // order purchase_node item_node
};

#endif // ENTRYHUBI_H
