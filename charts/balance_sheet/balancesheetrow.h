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

#ifndef BALANCESHEETROW_H
#define BALANCESHEETROW_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUuid>

#include "component/constant.h"
#include "enum/nodeenum.h"
#include "utils/entryutils.h"

struct BalanceSheetRow final {
    QString name {};
    QUuid id {};
    QString code {};
    QString description {};
    NodeKind kind {};
    bool direction_rule {};
    QString color {};
    QStringList tag {};
    QStringList document {};

    double final_total {};

    BalanceSheetRow* parent {};
    QList<BalanceSheetRow*> children {};

    inline void Reset() { *this = BalanceSheetRow {}; }
    inline void ReadJson(const QJsonObject& object)
    {
        if (const auto val = object.value(kName); val.isString())
            name = val.toString();
        if (const auto val = object.value(kId); val.isString())
            id = QUuid(val.toString());
        if (const auto val = object.value(kCode); val.isString())
            code = val.toString();
        if (const auto val = object.value(kDescription); val.isString())
            description = val.toString();
        if (const auto val = object.value(kKind); val.isDouble())
            kind = static_cast<NodeKind>(val.toInt());
        if (const auto val = object.value(kDirectionRule); val.isBool())
            direction_rule = val.toBool();
        if (const auto val = object.value(kFinalTotal); val.isString())
            final_total = val.toString().toDouble();
        if (const auto val = object.value(kColor); val.isString())
            color = val.toString();
        if (object.value(kTag).isArray())
            tag = utils::ReadStringList(object, kTag);
        if (object.value(kDocument).isArray())
            document = utils::ReadStringList(object, kDocument);
    }
};

#endif // BALANCESHEETROW_H
