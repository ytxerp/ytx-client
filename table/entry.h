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

#ifndef ENTRY_H
#define ENTRY_H

#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

struct Entry {
    QUuid id {};
    QDateTime issued_time {};
    QString code {};
    QUuid lhs_node {};
    QString description {};
    QStringList document {};
    int status {};
    QUuid rhs_node {};

    QUuid user_id {};
    QDateTime created_time {};
    QUuid created_by {};
    QDateTime updated_time {};
    QUuid updated_by {};

    virtual void ResetState();
    virtual void ReadJson(const QJsonObject& object);
    virtual QJsonObject WriteJson() const;
    virtual ~Entry() = default;
};

struct EntryF final : Entry {
    double lhs_rate { 1.0 };
    double rhs_rate { 1.0 };

    double lhs_debit {};
    double lhs_credit {};
    double rhs_debit {};
    double rhs_credit {};

    void ResetState() override;
    void ReadJson(const QJsonObject& object) override;
};

struct EntryI final : Entry {
    double unit_cost {};

    double lhs_debit {};
    double lhs_credit {};
    double rhs_debit {};
    double rhs_credit {};

    void ResetState() override;
    void ReadJson(const QJsonObject& object) override;
};

struct EntryT final : Entry {
    double unit_cost {};

    double lhs_debit {};
    double lhs_credit {};
    double rhs_debit {};
    double rhs_credit {};

    void ResetState() override;
    void ReadJson(const QJsonObject& object) override;
};

struct EntryP final : Entry {
    double unit_price {};
    QUuid external_sku {};

    void ResetState() override;
    void ReadJson(const QJsonObject& object) override;
    QJsonObject WriteJson() const override;
};

struct EntryO final : Entry {
    double unit_price {};
    double count {};
    double measure {};

    QUuid external_sku {};

    double initial {};
    double final {};
    double discount {};
    double unit_discount {};

    void ResetState() override;
    void ReadJson(const QJsonObject& object) override;
    QJsonObject WriteJson() const override;
};

using EntryList = QList<Entry*>;

#endif // ENTRY_H
