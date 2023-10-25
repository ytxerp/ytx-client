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
    void ApplyEntryRate(const QUuid& entry_id, const QJsonObject& data, bool is_parallel) override;
    void ApplyEntryNumeric(const QUuid& entry_id, const QJsonObject& data, bool is_parallel) override;

protected:
    QString QSReadTransRef(int unit) const override;
    std::pair<int, int> CacheColumnRange() const override { return { std::to_underlying(EntryEnumI::kCode), std::to_underlying(EntryEnumI::kIsChecked) }; }
    std::pair<int, int> NumericColumnRange() const override { return { std::to_underlying(EntryEnumI::kDebit), std::to_underlying(EntryEnumI::kBalance) }; }

private:
    QString QSReplaceLeafSI() const; // stakeholder_node item_node
    QString QSReplaceLeafOSI() const; // order sale_node item_node
    QString QSReplaceLeafOPI() const; // order purchase_node item_node
};

#endif // ENTRYHUBI_H
