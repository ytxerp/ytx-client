#ifndef CASHFLOWSTATEMENTROW_H
#define CASHFLOWSTATEMENTROW_H

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

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUuid>

#include "component/constant.h"
#include "enum/nodeenum.h"
#include "tree/finance_role.h"

struct CashFlowStatementRow final {
    QString name {};
    QUuid id {};
    QString code {};
    QString description {};
    NodeKind kind {};
    finance::CashKind cash_kind {};
    finance::Roles roles {};
    bool direction_rule {};

    double final_total {};

    CashFlowStatementRow* parent {};
    QList<CashFlowStatementRow*> children {};

    inline void Reset() { *this = CashFlowStatementRow {}; }
    inline void ReadJson(const QJsonObject& object)
    {
        // if (const auto val = object.value(kName); val.isString())
        //     name = val.toString();
        if (const auto val = object.value(kId); val.isString())
            id = QUuid(val.toString());
        if (const auto val = object.value(kCode); val.isString())
            code = val.toString();
        if (const auto val = object.value(kDescription); val.isString())
            description = val.toString();
        if (const auto val = object.value(kKind); val.isDouble())
            kind = static_cast<NodeKind>(val.toInt());
        if (const auto val = object.value(kCashKind); val.isDouble())
            cash_kind = static_cast<finance::CashKind>(val.toInt());
        if (const auto val = object.value(kDirectionRule); val.isBool())
            direction_rule = val.toBool();
        if (const auto val = object.value(kFinalTotal); val.isString())
            final_total = val.toString().toDouble();
        if (const auto val = object.value(kRoles); val.isDouble())
            roles = static_cast<finance::Roles>(val.toInt());
    }
};

#endif // CASHFLOWSTATEMENTROW_H
