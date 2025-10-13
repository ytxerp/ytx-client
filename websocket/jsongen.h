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

#include "component/enumclass.h"
#include "component/using.h"
#include "tree/node.h"

#ifndef JSONGEN_H
#define JSONGEN_H

namespace JsonGen {

QJsonObject Login(CString& email, CString& password, CString& workspace);
QJsonObject Register(CString& email, CString& password);

QJsonObject NodeInsert(Section section, const Node* node, CUuid& parent_id);
QJsonObject DragNode(Section section, CUuid& node_id, CUuid& parent_id);

QJsonObject LeafRemove(Section section, CUuid& node_id);
QJsonObject BranchRemove(Section section, CUuid& node_id, CUuid& parent_id);

QJsonObject LeafRemoveCheck(Section section, CUuid& node_id);
QJsonObject LeafReplace(Section section, CUuid& old_id, CUuid& new_id, bool inventory_external_ref);

QJsonObject EntryAction(Section section, CUuid& node_id, int action);
QJsonObject EntryUpdate(Section section, CUuid& entry_id, CJsonObject& cache);

QJsonObject TreeAcked(Section section, const QDateTime& start, const QDateTime& end);
QJsonObject LeafEntry(Section section, CUuid& node_id, CUuid& entry_id = {});
QJsonObject NodeAcked(Section section, CUuid& node_id);

QJsonObject NodeDirectionRule(Section section, CUuid& node_id, bool direction_rule);
QJsonObject NodeStatus(Section section, CUuid& node_id, int status);
QJsonObject NodeName(Section section, CUuid& node_id, CString& name);
QJsonObject NodeUpdate(Section section, CUuid& node_id, CJsonObject& cache);
QJsonObject DocumentDir(Section section, CString& document_dir);
QJsonObject DefaultUnit(Section section, int unit);

QJsonObject NodeDelta(CUuid& node_id, double initial_delta, double final_delta);
QJsonObject EntrySearch(Section section, CString& keyword);
QJsonObject NodeSearch(Section section, CString& keyword);

}

#endif // JSONGEN_H
