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
    QUuid partner {};
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
    partner = QUuid();
    pbalance = 0.0;
    ccount = 0.0;
    cmeasure = 0.0;
    camount = 0.0;
    cbalance = 0.0;
    csettlement = 0.0;
}

inline void Statement::ReadJson(const QJsonObject& object)
{
    if (object.contains(kPartner))
        partner = QUuid(object[kPartner].toString());

    if (object.contains("pbalance"))
        pbalance = object["pbalance"].toString().toDouble();

    if (object.contains("ccount"))
        ccount = object["ccount"].toString().toDouble();

    if (object.contains("cmeasure"))
        cmeasure = object["cmeasure"].toString().toDouble();

    if (object.contains("camount"))
        camount = object["camount"].toString().toDouble();

    if (object.contains("cbalance"))
        cbalance = object["cbalance"].toString().toDouble();

    if (object.contains("csettlement"))
        csettlement = object["csettlement"].toString().toDouble();
}

struct StatementPrimary {
    QDateTime issued_time {};
    double count_total {};
    double measure_total {};
    double initial_total {};
    bool status {};
    QString description {};
    QUuid employee {};
    double final_total {};

    void ResetState();
    void ReadJson(const QJsonObject& object);
};

inline void StatementPrimary::ResetState()
{
    issued_time = {};
    count_total = 0.0;
    measure_total = 0.0;
    initial_total = 0.0;
    status = false;
    description.clear();
    employee = QUuid();
    final_total = 0.0;
}

inline void StatementPrimary::ReadJson(const QJsonObject& object)
{
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object[kIssuedTime].toString(), Qt::ISODate);

    if (object.contains(kCountTotal))
        count_total = object[kCountTotal].toString().toDouble();

    if (object.contains(kMeasureTotal))
        measure_total = object[kMeasureTotal].toString().toDouble();

    if (object.contains(kInitialTotal))
        initial_total = object[kInitialTotal].toString().toDouble();

    if (object.contains(kDescription))
        description = object[kDescription].toString();

    if (object.contains(kEmployee))
        employee = QUuid(object[kEmployee].toString());

    if (object.contains(kFinalTotal))
        final_total = object[kFinalTotal].toString().toDouble();
}

struct StatementSecondary {
    QDateTime issued_time {};
    QUuid rhs_node {};
    double count {};
    double measure {};
    double unit_price {};
    double initial {};
    bool status {};
    QString description {};
    QUuid external_sku {};

    void ResetState();
    void ReadJson(const QJsonObject& object);
};

inline void StatementSecondary::ResetState()
{
    issued_time = {};
    count = 0.0;
    measure = 0.0;
    initial = 0.0;
    unit_price = 0.0;
    status = false;
    description.clear();
    rhs_node = QUuid();
    external_sku = QUuid();
}

inline void StatementSecondary::ReadJson(const QJsonObject& object)
{
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object[kIssuedTime].toString(), Qt::ISODate);

    if (object.contains(kRhsNode))
        rhs_node = QUuid(object[kRhsNode].toString());

    if (object.contains(kCount))
        count = object[kCount].toString().toDouble();

    if (object.contains(kMeasure))
        measure = object[kMeasure].toString().toDouble();

    if (object.contains(kUnitPrice))
        unit_price = object[kUnitPrice].toString().toDouble();

    if (object.contains(kInitial))
        initial = object[kInitial].toString().toDouble();

    if (object.contains(kDescription))
        description = object[kDescription].toString();

    if (object.contains(kExternalSku))
        external_sku = QUuid(object[kExternalSku].toString());
}

using StatementList = QList<Statement*>;
using StatementPrimaryList = QList<StatementPrimary*>;
using StatementSecondaryList = QList<StatementSecondary*>;

#endif // STATEMENT_H
