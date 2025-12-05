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
    if (object.contains("partner"))
        partner = QUuid(object["partner"].toString());

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
    double count {};
    double measure {};
    double initial_total {};
    bool status {};
    QString description {};
    QUuid employee {};
    double final_total {};

    void ResetState();
};

inline void StatementPrimary::ResetState()
{
    issued_time = {};
    count = 0.0;
    measure = 0.0;
    initial_total = 0.0;
    status = false;
    description.clear();
    employee = QUuid();
    final_total = 0.0;
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
    QUuid support_id {};

    void ResetState();
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
    support_id = QUuid();
}

using StatementList = QList<Statement*>;
using StatementPrimaryList = QList<StatementPrimary*>;
using StatementSecondaryList = QList<StatementSecondary*>;

#endif // STATEMENT_H
