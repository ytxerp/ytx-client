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

#ifndef SETTLEMENT_H
#define SETTLEMENT_H

#include <QJsonObject>
#include <QStringList>
#include <QUuid>

#include "global/resourcepool.h"

struct Settlement {
    QUuid id {};
    QUuid partner {};
    QDateTime issued_time {};
    QString description {};
    int status {};
    double amount {};

    QUuid user_id {};
    QDateTime created_time {};
    QUuid created_by {};
    QDateTime updated_time {};
    QUuid updated_by {};

    void ResetState();

    void ReadJson(const QJsonObject& object);
    QJsonObject WriteJson() const;
};

struct SettlementNode {
    QUuid id {};
    QUuid partner {};
    QUuid settlement_id {};
    QUuid employee {};
    QDateTime issued_time {};
    QString description {};
    double amount {};

    void ResetState();

    void ReadJson(const QJsonObject& object);
    QJsonObject WriteJson() const;
};

// Custom deleter that returns the Settlement to the ResourcePool
auto kSettlementDeleter = [](Settlement* s) { ResourcePool<Settlement>::Instance().Recycle(s); };

using SettlementList = QList<Settlement*>;
using SettlementNodeList = QList<SettlementNode*>;

#endif // SETTLEMENT_H
