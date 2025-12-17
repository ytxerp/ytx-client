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

struct Statement {
    QUuid partner_id {};
    double pbalance {};
    double ccount {};
    double cmeasure {};
    double camount {};
    double cbalance {};
    double csettlement {};

    void ResetState();
    void ReadJson(const QJsonObject& object);
};

inline void Statement::ResetState()
{
    partner_id = QUuid();
    pbalance = 0.0;
    ccount = 0.0;
    cmeasure = 0.0;
    camount = 0.0;
    cbalance = 0.0;
    csettlement = 0.0;
}

inline void Statement::ReadJson(const QJsonObject& object)
{
    if (object.contains(kPartnerId))
        partner_id = QUuid(object[kPartnerId].toString());

    if (object.contains(kPBalance))
        pbalance = object[kPBalance].toString().toDouble();

    if (object.contains(kCCount))
        ccount = object[kCCount].toString().toDouble();

    if (object.contains(kCMeasure))
        cmeasure = object[kCMeasure].toString().toDouble();

    if (object.contains(kCAmount))
        camount = object[kCAmount].toString().toDouble();

    if (object.contains(kCBalance))
        cbalance = object[kCBalance].toString().toDouble();

    if (object.contains(kCSettlement))
        csettlement = object[kCSettlement].toString().toDouble();
}

struct StatementNode {
    QDateTime issued_time {};
    double count {};
    double measure {};
    double amount {};
    int status {};
    QString description {};
    QUuid employee_id {};
    double settlement {};

    void ResetState();
    void ReadJson(const QJsonObject& object);
};

inline void StatementNode::ResetState()
{
    issued_time = {};
    count = 0.0;
    measure = 0.0;
    amount = 0.0;
    status = 0;
    description.clear();
    employee_id = QUuid();
    settlement = 0.0;
}

inline void StatementNode::ReadJson(const QJsonObject& object)
{
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object[kIssuedTime].toString(), Qt::ISODate);

    if (object.contains(kCount))
        count = object[kCount].toString().toDouble();

    if (object.contains(kMeasure))
        measure = object[kMeasure].toString().toDouble();

    if (object.contains(kAmount))
        amount = object[kAmount].toString().toDouble();

    if (object.contains(kDescription))
        description = object[kDescription].toString();

    if (object.contains(kEmployeeId))
        employee_id = QUuid(object[kEmployeeId].toString());

    if (object.contains(kSettlement))
        settlement = object[kSettlement].toString().toDouble();
}

struct StatementEntry {
    QDateTime issued_time {};
    QUuid internal_sku {};
    double count {};
    double measure {};
    double unit_price {};
    double amount {};
    int status {};
    QString description {};
    QUuid external_sku {};

    void ResetState();
    void ReadJson(const QJsonObject& object);
};

inline void StatementEntry::ResetState()
{
    issued_time = {};
    count = 0.0;
    measure = 0.0;
    amount = 0.0;
    unit_price = 0.0;
    status = 0;
    description.clear();
    internal_sku = QUuid();
    external_sku = QUuid();
}

inline void StatementEntry::ReadJson(const QJsonObject& object)
{
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object[kIssuedTime].toString(), Qt::ISODate);

    if (object.contains(kInternalSku))
        internal_sku = QUuid(object[kInternalSku].toString());

    if (object.contains(kCount))
        count = object[kCount].toString().toDouble();

    if (object.contains(kMeasure))
        measure = object[kMeasure].toString().toDouble();

    if (object.contains(kUnitPrice))
        unit_price = object[kUnitPrice].toString().toDouble();

    if (object.contains(kAmount))
        amount = object[kAmount].toString().toDouble();

    if (object.contains(kDescription))
        description = object[kDescription].toString();

    if (object.contains(kExternalSku))
        external_sku = QUuid(object[kExternalSku].toString());
}

using StatementEntryList = QList<StatementEntry*>;
using CStatementEntryList = const QList<StatementEntry*>;

#endif // STATEMENT_H
