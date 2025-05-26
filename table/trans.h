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

#ifndef TRANS_H
#define TRANS_H

#include <QStringList>
#include <QUuid>

struct Trans {
    QUuid id {};
    QString issued_time {};
    QString code {};
    QUuid lhs_node {};
    double lhs_ratio {};
    double lhs_debit {};
    double lhs_credit {};
    QString description {};
    QStringList document {};
    bool is_checked { false };
    double rhs_credit {};
    double rhs_debit {};
    double rhs_ratio {};
    QUuid rhs_node {};

    // order
    QUuid support_id {};
    double discount {};

    void GenerateId() { id = QUuid::createUuidV7(); }
    void ResetState()
    {
        id = QUuid();
        issued_time.clear();
        code.clear();
        lhs_node = QUuid();
        lhs_ratio = 0.0;
        lhs_debit = 0.0;
        lhs_credit = 0.0;
        description.clear();
        rhs_node = QUuid();
        rhs_ratio = 0.0;
        rhs_debit = 0.0;
        rhs_credit = 0.0;
        is_checked = false;
        document.clear();
        support_id = QUuid();
        discount = 0.0;
    }
};

struct TransShadow {
    QUuid* id {};
    QString* issued_time {};
    QString* code {};
    QUuid* lhs_node {};
    double* lhs_ratio {};
    double* lhs_debit {};
    double* lhs_credit {};
    QString* description {};
    QStringList* document {};
    bool* is_checked {};
    double* rhs_credit {};
    double* rhs_debit {};
    double* rhs_ratio {};
    QUuid* rhs_node {};
    QUuid* support_id {};
    double* discount {};

    double subtotal {};

    void GenerateId() { id = nullptr; }
    void ResetState()
    {
        id = nullptr;
        issued_time = nullptr;
        code = nullptr;
        lhs_node = nullptr;
        lhs_ratio = nullptr;
        lhs_debit = nullptr;
        lhs_credit = nullptr;
        description = nullptr;
        rhs_node = nullptr;
        rhs_ratio = nullptr;
        rhs_debit = nullptr;
        rhs_credit = nullptr;
        is_checked = nullptr;
        document = nullptr;
        support_id = nullptr;
        discount = nullptr;

        subtotal = 0.0;
    }
};

using TransList = QList<Trans*>;
using TransShadowList = QList<TransShadow*>;

#endif // TRANS_H
