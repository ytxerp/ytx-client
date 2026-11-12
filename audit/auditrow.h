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

#include <QJsonArray>
#include <QJsonObject>

namespace audit {

inline QString JsonValueToString(const QJsonValue& value, QJsonDocument::JsonFormat format)
{
    if (value.isObject()) {
        const QJsonObject obj { value.toObject() };
        return obj.isEmpty() ? QString {} : QString::fromUtf8(QJsonDocument(obj).toJson(format));
    }

    if (value.isArray()) {
        const QJsonArray arr { value.toArray() };
        return arr.isEmpty() ? QString {} : QString::fromUtf8(QJsonDocument(arr).toJson(format));
    }

    return {};
}

constexpr int kRawJsonRole = Qt::UserRole + 1;

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
    QString before_compact {};
    QString after_compact {};
    QJsonValue before {};
    QJsonValue after {};

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
    kMerge = 104,

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
