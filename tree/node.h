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

#ifndef NODE_H
#define NODE_H

#include <QJsonObject>
#include <QStringList>
#include <QUuid>

#include "enum/nodeenum.h"

struct Node {
    QString name {};
    QUuid id {};
    QString code {};
    QString description {};
    QString note {};
    NodeKind kind {};
    bool direction_rule {};
    NodeUnit unit {};
    QString color {};
    QStringList tag {};

    double final_total {};
    double initial_total {};

    QUuid user_id {};
    QDateTime created_time {};
    QUuid created_by {};
    QDateTime updated_time {};
    QUuid updated_by {};
    int version {};

    Node* parent {};
    QList<Node*> children {};

    virtual void ResetState();
    virtual void InvertTotal();

    virtual void ReadJson(const QJsonObject& object);
    virtual QJsonObject WriteJson() const;

    virtual ~Node() = default;
};

struct NodeF final : Node { };

struct NodeI final : Node {
    double unit_price {};
    double commission {};

    void ResetState() override;

    void ReadJson(const QJsonObject& object) override;
    QJsonObject WriteJson() const override;
};

struct NodeT final : Node {
    QStringList document {};
    QDateTime issued_time {};
    NodeStatus status {};

    void ResetState() override;

    void ReadJson(const QJsonObject& object) override;
    QJsonObject WriteJson() const override;
};

struct NodeP final : Node {
    int payment_term {};

    void ResetState() override;

    void ReadJson(const QJsonObject& object) override;
    QJsonObject WriteJson() const override;
};

struct NodeO final : Node {
    QUuid employee_id {};
    QUuid partner_id {};

    // NOTE: is_settled and settlement_id are managed by the server
    // - ReadJson: Updates from server
    // - WriteJson: Should NOT be serialized (server sets default: false)
    bool is_settled {};
    QUuid settlement_id {};

    QDateTime issued_time {};
    double count_total {};
    double measure_total {};
    double discount_total {};
    NodeStatus status {};

    void ResetState() override;
    void InvertTotal() override;

    void ReadJson(const QJsonObject& object) override;
    QJsonObject WriteJson() const override;
};

using NodeHash = QHash<QUuid, Node*>;
using CNodeHash = const QHash<QUuid, Node*>;
using NodeList = QList<Node*>;

using NodeListO = QList<NodeO*>;

#endif // NODE_H
