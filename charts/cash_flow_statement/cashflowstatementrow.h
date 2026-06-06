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
#include "tree/finance_role.h"

struct CashFlowStatementRow final {
    QString name {};
    QUuid id {};
    QString code {};
    QString description {};
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

struct CashFlowStatementWrongRow final {
    QUuid id {};
    QDateTime issued_time {};
    QUuid lhs_node {};
    QString description {};
    QUuid rhs_node {};
    finance::CashKind cash_kind {};

    double lhs_debit {};
    double lhs_credit {};
    double rhs_debit {};
    double rhs_credit {};

    inline void Reset() { *this = CashFlowStatementWrongRow {}; }
    inline void ReadJson(const QJsonObject& object)
    {
        if (const auto val = object.value(kId); val.isString())
            id = QUuid(val.toString());
        if (const auto val = object.value(kIssuedTime); val.isString())
            issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);
        if (const auto val = object.value(kLhsNode); val.isString())
            lhs_node = QUuid(val.toString());
        if (const auto val = object.value(kDescription); val.isString())
            description = val.toString();
        if (const auto val = object.value(kRhsNode); val.isString())
            rhs_node = QUuid(val.toString());
        if (const auto val = object.value(kLhsDebit); val.isString())
            lhs_debit = val.toString().toDouble();
        if (const auto val = object.value(kLhsCredit); val.isString())
            lhs_credit = val.toString().toDouble();
        if (const auto val = object.value(kRhsDebit); val.isString())
            rhs_debit = val.toString().toDouble();
        if (const auto val = object.value(kRhsCredit); val.isString())
            rhs_credit = val.toString().toDouble();
        if (const auto val = object.value(kCashKind); val.isDouble())
            cash_kind = static_cast<finance::CashKind>(val.toInt());
    }
};

#endif // CASHFLOWSTATEMENTROW_H
