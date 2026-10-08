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

#include "component/using.h"
#include "order/statement/statement.h"
#include "utils/daterange.h"

class ExportExcel {
public:
    static ExportExcel& Instance()
    {
        static ExportExcel instance {};
        return instance;
    }

    void StatementTertiaryAsync(CString& path, CString& partner_name, CUuid& partner_id, CString& unit_string, const utils::DateRange& range,
        CStringList& header, const QList<statement::TertiaryRow>& list, const QList<QVariant>& summary);

    void StatementSecondaryAsync(CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines);

    ExportExcel(const ExportExcel&) = delete;
    ExportExcel& operator=(const ExportExcel&) = delete;
    ExportExcel(ExportExcel&&) = delete;
    ExportExcel& operator=(ExportExcel&&) = delete;

private:
    ExportExcel() = default;
    ~ExportExcel() = default;

    static bool StatementTertiary(CString& path, CString& partner_name, CUuid& partner_id, CString& unit_string, const utils::DateRange& range,
        CStringList& header, const QList<statement::TertiaryRow>& list, const QList<QVariant>& summary);

    static bool StatementSecondary(CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines);
};
