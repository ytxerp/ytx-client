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
#include "enum/section.h"
#include "tag/tag.h"
#include "tree/node.h"

#ifndef JSONGEN_H
#define JSONGEN_H

namespace JsonGen {

QJsonObject Login(CString& email, CString& password, CString& workspace);
QJsonObject Register(CString& email, CString& password);

QJsonObject NodeInsert(Section section, const Node* node, CUuid& parent_id);
QJsonObject NodeDrag(Section section, CUuid& node_id, CUuid& parent_id);

QJsonObject LeafDelete(Section section, CUuid& node_id);
QJsonObject LeafDeleteP(Section section, CUuid& node_id);
QJsonObject LeafDeleteO(Section section, CUuid& node_id);
QJsonObject BranchDelete(Section section, CUuid& node_id, CUuid& parent_id);

QJsonObject LeafDeleteCheck(Section section, CUuid& node_id);
QJsonObject LeafReplace(Section section, CUuid& old_id, CUuid& new_id);

QJsonObject BatchMark(Section section, CUuid& node_id, int mark);
QJsonObject EntryUpdate(Section section, CUuid& entry_id, CJsonObject& update);
QJsonObject EntryValue(Section section, CUuid& entry_id, CJsonObject& update, bool is_parallel);
QJsonObject EntryDelete(Section section, CUuid& entry_id);
QJsonObject EntryLinkedNode(Section section, CUuid& entry_id);

QJsonObject TagUpdate(Section section, CUuid& id, CJsonObject& update);
QJsonObject TagInsert(Section section, const Tag* tag);
QJsonObject TagDelete(Section section, CUuid& tag_id);

QJsonObject WorkspaceMemberUpdate(CUuid& id, CJsonObject& update);
QJsonObject WorkspaceMemberDelete(CUuid& id);
QJsonObject WorkspaceMemberAck(CString& email, CString& workspace);

QJsonObject TreeAck(Section section, const QDateTime& start, const QDateTime& end);
QJsonObject TableAck(Section section, CUuid& node_id, CUuid& entry_id = {});
QJsonObject NodeAck(Section section, CUuid& node_id);
QJsonObject OrderReferenceAck(Section section, CUuid& widget_id, CUuid& node_id, int unit, const QDateTime& start, const QDateTime& end);
QJsonObject StatementAck(Section section, CUuid& widget_id, int unit, const QDateTime& start, const QDateTime& end);
QJsonObject StatementNodeAck(Section section, CUuid& widget_id, CUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end);
QJsonObject StatementEntryAck(Section section, CUuid& widget_id, CUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end);
QJsonObject SettlementAck(Section section, CUuid& widget_id, const QDateTime& start, const QDateTime& end);
QJsonObject SettlementItemAck(Section section, CUuid& widget_id, CUuid& partner_id, CUuid& settlement_id);
QJsonObject SettlementDelete(Section section, CUuid& settlement_id, int version);

QJsonObject NodeDirectionRule(Section section, CUuid& node_id, bool direction_rule);
QJsonObject NodeName(Section section, CUuid& node_id, CString& name);
QJsonObject NodeUpdate(Section section, CUuid& node_id, CJsonObject& update);
QJsonObject DocumentDir(Section section, CString& document_dir);
QJsonObject DefaultUnit(Section section, int unit);

QJsonObject EntryDescriptionSearch(Section section, CString& keyword);
QJsonObject EntryTagSearch(Section section, const QSet<QString>& tags);
QJsonObject NodeSearch(Section section, CString& keyword);

QJsonObject OrderRecall(Section section, CUuid& node_id, CJsonObject& update);
QJsonObject AccountName(CString& email, CString& name);
QJsonObject AccountUsername(CString& email, CString& username);

}

#endif // JSONGEN_H
