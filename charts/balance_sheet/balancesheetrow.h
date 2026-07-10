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
#include "component/constantstring.h"
#include "enum/nodeenum.h"
#include "utils/nodeutils.h"

namespace balance_sheet {

struct Row final {
    QString name {};
    QUuid id {};
    QString code {};
    QString description {};
    NodeKind kind {};
    bool direction_rule {};

    double opening_balance {};
    double closing_balance {};

    double change_amount {};
    double change_rate {};

    Row* parent {};
    QList<Row*> children {};

    inline void Reset() { *this = Row {}; }
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
        if (const auto val = object.value(balance_sheet::kClosingBalance); val.isString())
            closing_balance = val.toString().toDouble();
        if (const auto val = object.value(balance_sheet::kOpeningBalance); val.isString())
            opening_balance = val.toString().toDouble();

        change_amount = closing_balance - opening_balance;
        change_rate = utils::GrowthRate(closing_balance, opening_balance);
    }
};
}

#endif // BALANCESHEETROW_H
