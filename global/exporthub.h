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
#include "utils/daterange.h"

class ExportHub {
public:
    static ExportHub& Instance()
    {
        static ExportHub instance {};
        return instance;
    }

    void StatementTertiaryAsync(CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines);
    void StatementSecondaryAsync(CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines);

    ExportHub(const ExportHub&) = delete;
    ExportHub& operator=(const ExportHub&) = delete;
    ExportHub(ExportHub&&) = delete;
    ExportHub& operator=(ExportHub&&) = delete;

private:
    ExportHub() = default;
    ~ExportHub() = default;

    static bool StatementTertiary(CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines);
    static bool StatementSecondary(CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines);
};
