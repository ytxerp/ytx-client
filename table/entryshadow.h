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

#ifndef ENTRYSHADOW_H
#define ENTRYSHADOW_H

#include <QJsonObject>

#include "table/entry.h"

struct EntryShadow final {
    QUuid* id {};
    QDateTime* issued_time {};
    QString* code {};
    QUuid* lhs_node {};
    QString* description {};
    QStringList* document {};
    QStringList* tag {};
    int* status {};
    QUuid* rhs_node {};
    double* lhs_rate {};
    double* rhs_rate {};

    double* lhs_debit {};
    double* lhs_credit {};
    double* rhs_debit {};
    double* rhs_credit {};

    double balance {};

    int* version {};

    Entry* entry {};

    // Binding mode:
    // • Parallel binding: shadow.lhs_node → entry.lhs_node, shadow.rhs_node → entry.rhs_node
    // • Cross binding:    shadow.lhs_node → entry.rhs_node, shadow.rhs_node → entry.lhs_node
    bool is_parallel {};

    // BindEntry connects the shadow to a concrete Entry instance.
    void BindEntry(Entry* base, bool parallel);

    // Reset clears all bound pointers and restores the shadow to default values.
    void Reset();

    // Serialize shadow to JSON.
    QJsonObject WriteJson() const;
};

using EntryShadowList = QList<EntryShadow*>;

#endif // ENTRYSHADOW_H
