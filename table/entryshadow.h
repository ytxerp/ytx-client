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

struct State {
    QJsonObject entry {};
    QJsonObject lhs_node {};
    QJsonObject rhs_node {};

    QJsonObject WriteJson() const;
};

struct EntryShadow {
    QUuid* id {};
    QDateTime* issued_time {};
    QString* code {};
    QUuid* lhs_node {};
    QString* description {};
    QStringList* document {};
    int* status {};
    QUuid* rhs_node {};

    double balance {};

    QUuid* user_id {};
    QDateTime* created_time {};
    QUuid* created_by {};
    QDateTime* updated_time {};
    QUuid* updated_by {};

    Entry* entry {};

    // Binding mode:
    // • Parallel binding: shadow.lhs_node → entry.lhs_node, shadow.rhs_node → entry.rhs_node
    // • Cross binding:    shadow.lhs_node → entry.rhs_node, shadow.rhs_node → entry.lhs_node
    bool is_parallel {};

    // BindEntry connects the shadow to a concrete Entry instance.
    // Subclasses can:
    //   1. Call the base implementation to bind common fields, or
    //   2. Partially bind only relevant fields (e.g., EntryShadowO for order entries),
    //      skipping unused common fields like issued_time, document, or status.
    virtual void BindEntry(Entry* base, bool parallel);

    // ResetState clears all bound pointers and restores the shadow to default values.
    virtual void ResetState();

    // Serialize shadow to JSON.
    virtual QJsonObject WriteJson() const;
    virtual ~EntryShadow() = default;
};

struct EntryShadowF final : EntryShadow {
    double* lhs_rate {};
    double* rhs_rate {};
    double* lhs_debit {};
    double* lhs_credit {};
    double* rhs_debit {};
    double* rhs_credit {};

    void BindEntry(Entry* base, bool is_parallel) override;
    void ResetState() override;
    QJsonObject WriteJson() const override;
};

struct EntryShadowI final : EntryShadow {
    double* lhs_rate {};
    double* rhs_rate {};
    double* lhs_debit {};
    double* lhs_credit {};
    double* rhs_debit {};
    double* rhs_credit {};

    void BindEntry(Entry* base, bool is_parallel) override;
    void ResetState() override;
    QJsonObject WriteJson() const override;
};

struct EntryShadowT final : EntryShadow {
    double* lhs_rate {};
    double* rhs_rate {};

    double* lhs_debit {};
    double* lhs_credit {};
    double* rhs_debit {};
    double* rhs_credit {};

    void BindEntry(Entry* base, bool is_parallel) override;
    void ResetState() override;
    QJsonObject WriteJson() const override;
};

struct EntryShadowP final : EntryShadow {
    double* unit_price {};
    QUuid* external_sku {};

    void BindEntry(Entry* base, bool is_parallel) override;
    void ResetState() override;
    QJsonObject WriteJson() const override { std::unreachable(); }
};

struct EntryShadowO final : EntryShadow {
    double* unit_price {};
    QUuid* external_sku {};

    double* count {};
    double* measure {};

    double* initial {};
    double* final {};
    double* discount {};
    double* unit_discount {};

    void BindEntry(Entry* base, bool is_parallel) override;
    void ResetState() override;
    QJsonObject WriteJson() const override { std::unreachable(); }
};

using EntryShadowList = QList<EntryShadow*>;

#endif // ENTRYSHADOW_H
