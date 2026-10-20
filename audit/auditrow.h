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

#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

namespace audit {

enum class RowField {
    kTargetId,
    kUsername,
    kLhsNodeName,
    kRhsNodeName,
    kCreatedTime,
    kTargetCode,
    kTargetType,
    kTargetOperation,
    kTargetField,
    kPlaceholder,
    kBefore,
    kAfter,
};

struct Row {
    QString target_id {};
    QString username {};
    QString lhs_node_name {};
    QString rhs_node_name {};

    // -- 2. Timestamp -----------------------------------------------------------
    QString created_time {}; // UTC — maps to TIMESTAMPTZ

    // -- 3. Integers ------------------------------------------------------------
    int target_operation {}; // Workspace key
    int target_type {}; // Discriminator for the audited entity type
    int target_field {};

    // -- 4. Variable-length -----------------------------------------------------
    QString target_code {}; // Default: ""
    QJsonValue before {}; // State before the action — maps to JSONB
    QJsonValue after {}; // State after the action  — maps to JSONB

    void Reset();
    void ReadJson(const QJsonObject& object);
};

enum class TargetType {
    kNode = 0,
    kEntry = 1,
    kSettlement = 2,
};

enum class TargetOperation {
    // Data: 100–199
    kInsert = 100,
    kUpdate = 101,
    kDelete = 102,
    kMove = 103,
    kReplace = 104,

    // --- Order workflow: 200–219 ---
    kRelease = 200,
    kRecall = 201,

    // --- Finance: 220–239 ---
    kPeriodClose = 220,

    // Lock: 300–399
    kLock = 300,
    kUnlock = 301,
};

enum class TargetField {
    kNone = 0,

    // --- Node: 100–199 ---
    kName = 100,
    kDirectionRule = 101,

    // --- Entry: 200–299 ---
    kNumeric = 200,
    kRate = 201,
    kLinkedNode = 202,

    // --- Common: 300–399 ---
    kStatus = 300,
    kContent = 301,
};

}
