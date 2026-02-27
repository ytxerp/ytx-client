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

#ifndef STATEMENT_H
#define STATEMENT_H

#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

#include "component/constant.h"

struct Statement final {
    QUuid partner_id {};
    double pbalance {};
    double ccount {};
    double cmeasure {};
    double camount {};
    double cbalance {};
    double csettlement {};

    void Reset();
    void ReadJson(const QJsonObject& object);
};

inline void Statement::Reset() { *this = Statement {}; }

inline void Statement::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kPartnerId); val.isString())
        partner_id = QUuid(val.toString());
    if (const auto val = object.value(kPBalance); val.isString())
        pbalance = val.toString().toDouble();
    if (const auto val = object.value(kCCount); val.isString())
        ccount = val.toString().toDouble();
    if (const auto val = object.value(kCMeasure); val.isString())
        cmeasure = val.toString().toDouble();
    if (const auto val = object.value(kCAmount); val.isString())
        camount = val.toString().toDouble();
    if (const auto val = object.value(kCBalance); val.isString())
        cbalance = val.toString().toDouble();
    if (const auto val = object.value(kCSettlement); val.isString())
        csettlement = val.toString().toDouble();
}

struct StatementNode final {
    QDateTime issued_time {};
    QString code {};
    double count {};
    double measure {};
    double amount {};
    int status {};
    QString description {};
    QUuid employee_id {};
    double settlement {};

    void Reset();
    void ReadJson(const QJsonObject& object);
};

inline void StatementNode::Reset() { *this = StatementNode {}; }

inline void StatementNode::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kIssuedTime); val.isString())
        issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kCount); val.isString())
        count = val.toString().toDouble();
    if (const auto val = object.value(kMeasure); val.isString())
        measure = val.toString().toDouble();
    if (const auto val = object.value(kAmount); val.isString())
        amount = val.toString().toDouble();
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (const auto val = object.value(kCode); val.isString())
        code = val.toString();
    if (const auto val = object.value(kEmployeeId); val.isString())
        employee_id = QUuid(val.toString());
    if (const auto val = object.value(kSettlement); val.isString())
        settlement = val.toString().toDouble();
}

struct StatementEntry final {
    QDateTime issued_time {};
    QString code {};
    QUuid internal_sku {};
    double count {};
    double measure {};
    double unit_price {};
    double amount {};
    int status {};
    QString description {};

    void Reset();
    void ReadJson(const QJsonObject& object);
};

inline void StatementEntry::Reset() { *this = StatementEntry {}; }

inline void StatementEntry::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kIssuedTime); val.isString())
        issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kInternalSku); val.isString())
        internal_sku = QUuid(val.toString());
    if (const auto val = object.value(kCount); val.isString())
        count = val.toString().toDouble();
    if (const auto val = object.value(kMeasure); val.isString())
        measure = val.toString().toDouble();
    if (const auto val = object.value(kUnitPrice); val.isString())
        unit_price = val.toString().toDouble();
    if (const auto val = object.value(kAmount); val.isString())
        amount = val.toString().toDouble();
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (const auto val = object.value(kCode); val.isString())
        code = val.toString();
}

using StatementEntryList = QList<StatementEntry*>;
using CStatementEntryList = const QList<StatementEntry*>;

#endif // STATEMENT_H
