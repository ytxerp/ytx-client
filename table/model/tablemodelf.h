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

#ifndef TABLEMODELF_H
#define TABLEMODELF_H

#include "tablemodel.h"

class TableModelF final : public TableModel {
    Q_OBJECT

public:
    TableModelF(CTableModelArg& arg, QObject* parent = nullptr);
    ~TableModelF() override = default;

protected:
    bool UpdateNumeric(EntryShadow* shadow, double value, int row, bool is_debit) override;
    bool UpdateRate(EntryShadow* shadow, double value) override;
    bool UpdateLinkedNode(EntryShadow* shadow, const QUuid& value, int row) override;
    void InitRate(EntryShadow* shadow) const override
    {
        *shadow->lhs_rate = 1.0;
        *shadow->rhs_rate = 1.0;
    }
};

#endif // TABLEMODELF_H
