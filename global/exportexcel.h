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

#ifndef EXPORTEXCEL_H
#define EXPORTEXCEL_H

#include <QJsonObject>

#include "billing/statement/statement.h"
#include "component/using.h"

class ExportExcel {
public:
    static ExportExcel& Instance()
    {
        static ExportExcel instance {};
        return instance;
    }

    void StatementAsync(CString& path, CString& partner_name, CUuidString& inventory_leaf, CString& unit_string, CDateTime& start, CDateTime& end,
        CJsonObject& total, CStatementEntryList& list);

    ExportExcel(const ExportExcel&) = delete;
    ExportExcel& operator=(const ExportExcel&) = delete;
    ExportExcel(ExportExcel&&) = delete;
    ExportExcel& operator=(ExportExcel&&) = delete;

private:
    ExportExcel() = default;
    ~ExportExcel() = default;

    static bool Statement(CString& path, CString& partner_name, CUuidString& inventory_leaf, CString& unit_string, CDateTime& start, CDateTime& end,
        CJsonObject& total, CStatementEntryList& list);
};

#endif // EXPORTEXCEL_H
