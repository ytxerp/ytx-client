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

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QUuid>

struct SharedConfig {
    int default_unit {};
    QString document_dir {};

    bool operator==(const SharedConfig& other) const noexcept
    {
        return std::tie(default_unit, document_dir) == std::tie(other.default_unit, other.document_dir);
    }

    bool operator!=(const SharedConfig& other) const noexcept { return !(*this == other); }
};

struct AppConfig {
    QString theme {};
    QString language {};
    QString separator {};
    QString printer {};
    QString company_name {};

    // Equality operator overload to compare two AppConfig structs
    bool operator==(const AppConfig& other) const noexcept
    {
        return std::tie(theme, language, separator, printer, company_name)
            == std::tie(other.theme, other.language, other.separator, other.printer, other.company_name);
    }

    // Inequality operator overload to compare two AppConfig structs
    bool operator!=(const AppConfig& other) const noexcept { return !(*this == other); }
};

struct SectionConfig {
    QString static_label {};
    QUuid static_node {};
    QString dynamic_label {};
    QUuid dynamic_node_lhs {};
    QString operation {};
    QUuid dynamic_node_rhs {};
    QString date_format {};
    int amount_decimal {};
    int rate_decimal {};

    // Equality operator overload to compare two SectionConfig structs
    bool operator==(const SectionConfig& other) const noexcept
    {
        return std::tie(static_label, static_node, dynamic_label, dynamic_node_lhs, operation, dynamic_node_rhs, date_format, amount_decimal, rate_decimal)
            == std::tie(other.static_label, other.static_node, other.dynamic_label, other.dynamic_node_lhs, other.operation, other.dynamic_node_rhs,
                other.date_format, other.amount_decimal, other.rate_decimal);
    }

    // Inequality operator overload to compare two SectionConfig structs
    bool operator!=(const SectionConfig& other) const noexcept { return !(*this == other); }
};

using CAppConfig = const AppConfig;
using CSharedConfig = const SharedConfig;
using CSectionConfig = const SectionConfig;

#endif // CONFIG_H
