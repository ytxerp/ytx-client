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

#include "component/using.h"
#include "tree/node.h"

#ifndef JSONGEN_H
#define JSONGEN_H

namespace JsonGen {

QJsonObject Login();
QJsonObject Register(CString& email, CString& password);

QJsonObject InsertNode(CString& section, const Node* node, CUuid& parent_id);
QJsonObject DragNode(CString& section, CUuid& node_id, CUuid& parent_id);

QJsonObject LeafRemove(CString& section, CUuid& node_id);
QJsonObject BranchRemove(CString& section, CUuid& node_id);

QJsonObject LeafRemoveCheck(CString& section, CUuid& node_id);
QJsonObject LeafReplace(CString& section, CUuid& old_id, CUuid& new_id, bool inventory_external_ref);

QJsonObject EntryAction(CString& section, CUuid& node_id, int action);

QJsonObject TreeAcked(CString& section, const QDateTime& start, const QDateTime& end);
QJsonObject LeafAcked(CString& section, CUuid& node_id, CUuid& entry_id = {});
QJsonObject NodeAcked(CString& section, CUuid& node_id);

QJsonObject NodeDirectionRule(CString& section, CUuid& node_id, bool direction_rule);
QJsonObject NodeStatus(CString& section, CUuid& node_id, int status);
QJsonObject Update(CString& section, CUuid& id, CJsonObject& cache);
QJsonObject UpdateDocumentDir(CString& section, CString& document_dir);
QJsonObject UpdateDefaultUnit(CString& section, int unit);

QJsonObject NodeDelta(CUuid& node_id, double initial_delta, double final_delta);
QJsonObject SearchEntry(CString& section, CString& keyword);

}

#endif // JSONGEN_H
