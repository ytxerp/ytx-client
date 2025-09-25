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
#include <QUuid>

struct Statement {
    QUuid party {};
    double pbalance {};
    double ccount {};
    double cmeasure {};
    double cgross_amount {};
    double cbalance {};
    double csettlement {};

    void ResetState();
};

inline void Statement::ResetState()
{
    party = QUuid();
    pbalance = 0.0;
    ccount = 0.0;
    cmeasure = 0.0;
    cgross_amount = 0.0;
    cbalance = 0.0;
    csettlement = 0.0;
}

struct StatementPrimary {
    QDateTime issued_time {};
    double count {};
    double measure {};
    double initial_total {};
    bool is_checked {};
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
    is_checked = false;
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
    bool is_checked {};
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
    is_checked = false;
    description.clear();
    rhs_node = QUuid();
    support_id = QUuid();
}

using StatementList = QList<Statement*>;
using StatementPrimaryList = QList<StatementPrimary*>;
using StatementSecondaryList = QList<StatementSecondary*>;

#endif // STATEMENT_H
