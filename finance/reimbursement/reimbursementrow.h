/*
 * Copyright (C) 2026 YTX
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

#pragma once

#include <QJsonObject>
#include <QUuid>

#include "enum/stateenum.h"

namespace reimbursement {

enum class Status {
    kDraft,
    kApproved,
    kPosted,
};

enum class PrimaryField {
    kUsername,
    kIssuedTime,
    kDescription,
    kStatus,
    kPlaceholder,
    kInitialTotal,
};

struct PrimaryRow final {
    QUuid id {};
    QDateTime issued_time {};

    QString username {};
    QString description {};

    Status status { Status::kDraft };
    double initial_total {};
    int version {};

    SyncState sync_state { SyncState::kCreating };

    void Reset();
    void ReadJson(const QJsonObject& object);
    QJsonObject WriteJson() const;
};

enum class SecondaryField {
    kDescription,
    kDebitNode,
    kCreditNode,
    kDocument,
    kPlaceholder,
    kInitial,
};

struct SecondaryRow final {
    QUuid id {};
    QUuid lhs_node {};
    QUuid debit_node {};
    QUuid credit_node {};

    QString description {};
    QStringList document {};
    double initial {};
    int version {};

    SyncState sync_state { SyncState::kCreating };

    void Reset();
    void ReadJson(const QJsonObject& object);
    QJsonObject WriteJson() const;
};

} // namespace reimbursement