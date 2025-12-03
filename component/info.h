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

#ifndef INFO_H
#define INFO_H

#include <QMap>
#include <QPointer>
#include <QStringList>
#include <QUuid>

#include "enum/section.h"
#include "tree/itemmodel.h"

struct SectionInfo {
    Section section {};

    QUuid last_tab_id {};

    QString node {}; // SQL database node table name, also used as QSettings section name, be carefull with it
    QString path {}; // SQL database node_path table name
    QString entry {}; // SQL database node_entry table name
    QString settlement {}; // SQL database node_settlement table name

    QStringList node_header {};
    QStringList entry_header {};

    QStringList node_referenced_header {};

    QStringList settlement_header {};
    QStringList settlement_primary_header {};

    QStringList statement_header {};
    QStringList statement_primary_header {};
    QStringList statement_secondary_header {};

    QStringList full_entry_header {};

    QMap<int, QString> unit_map {};
    QMap<int, QString> unit_symbol_map {};
    QMap<bool, QString> rule_map {};
    QMap<int, QString> kind_map {};

    QPointer<ItemModel> unit_model {};
    QPointer<ItemModel> rule_model {};
};

struct TabInfo {
    Section section {};
    QUuid id {};

    // Equality operator overload to compare two TabInfo structs
    bool operator==(const TabInfo& other) const noexcept { return std::tie(section, id) == std::tie(other.section, other.id); }

    // Inequality operator overload to compare two TabInfo structs
    bool operator!=(const TabInfo& other) const noexcept { return !(*this == other); }
};

using CSectionInfo = const SectionInfo;

#endif // INFO_H
