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

#ifndef AUDITINFO_H
#define AUDITINFO_H

#include <QHash>
#include <QList>
#include <QString>
#include <QUuid>

namespace audit_hub {

struct AuditInfo {
    QHash<QUuid, QString> user_hash {}; // user_id, username
    QHash<int, QString> section_hash {};
    QHash<int, QString> ws_key_hash {};
    QHash<int, QString> level_hash {};
    QHash<int, QString> target_type_hash {};

    const QHash<QUuid, QString>* f_leaf_path {};
    const QHash<QUuid, QString>* f_branch_path {};
    const QHash<QUuid, QString>* i_leaf_path {};
    const QHash<QUuid, QString>* i_branch_path {};
    const QHash<QUuid, QString>* p_leaf_path {};
    const QHash<QUuid, QString>* p_branch_path {};
    const QHash<QUuid, QString>* t_leaf_path {};
    const QHash<QUuid, QString>* t_branch_path {};

    QStringList header {};
};

}

#endif // AUDITINFO_H
