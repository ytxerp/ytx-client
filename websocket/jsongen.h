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

#include "component/using.h"
#include "enum/bindingmode.h"
#include "enum/section.h"
#include "tag/tagrow.h"
#include "tree/node.h"
#include "utils/daterange.h"

// Only client-owned data is included in the request.
// Server-owned fields are computed and filled by the server.

namespace JsonGen {

QJsonObject Login(CString& email, CString& password, CString& workspace, const utils::DateTimeRange& range, int utc_offset);
QJsonObject Register(CString& email, CString& password);

QJsonObject NodeInsert(Section section, const Node* node, QUuid parent_id);
QJsonObject NodeDrag(Section section, QUuid node_id, QUuid parent_id);

QJsonObject LeafDelete(Section section, QUuid node_id, int version);
QJsonObject BranchDelete(Section section, QUuid node_id, QUuid parent_id, int version);

QJsonObject LeafDeleteCheck(Section section, QUuid node_id, int version);
QJsonObject LeafMerge(Section section, QUuid source_id, QUuid target_id, int version);

QJsonObject MarkEntries(Section section, QUuid node_id, int operation);
QJsonObject LockEntries(Section section, QUuid node_id, bool is_locked);
QJsonObject EntryUpdate(Section section, QUuid entry_id, CJsonObject& update, int version);
QJsonObject EntryValue(Section section, QUuid entry_id, CJsonObject& update, InputSide side, int version);
QJsonObject EntryMessage(Section section, QUuid entry_id);

QJsonObject TagUpdate(Section section, QUuid id, CJsonObject& update, int version);
QJsonObject TagInsert(Section section, const tag::Row* tag);
QJsonObject TagDelete(Section section, QUuid tag_id, int version);

QJsonObject WorkspaceMemberUpdate(QUuid id, CJsonObject& update, int version);
QJsonObject WorkspaceMemberDelete(QUuid id);
QJsonObject WorkspaceMemberAck(QUuid widget_id, CString& workspace);
QJsonObject AuditLogAck(QUuid widget_id, CString& workspace, const utils::DateTimeRange& range, Section section);

QJsonObject TreeAck(Section section, const utils::DateTimeRange& range);
QJsonObject TableAck(Section section, QUuid node_id, QUuid entry_id = {});
QJsonObject NodeAck(Section section, QUuid node_id);
QJsonObject StatementPrimary(Section section, QUuid widget_id, int unit, const utils::DateTimeRange& range);
QJsonObject StatementSecondary(Section section, QUuid widget_id, QUuid partner_id, int unit, const utils::DateTimeRange& range);
QJsonObject StatementTertiary(Section section, QUuid widget_id, QUuid partner_id, int unit, const utils::DateTimeRange& range);
QJsonObject SettlementPrimary(Section section, QUuid widget_id, const utils::DateTimeRange& range);
QJsonObject SettlementSecondary(Section section, QUuid widget_id, QUuid partner_id, QUuid settlement_id);
QJsonObject SettlementDelete(Section section, QUuid settlement_id, int version);
QJsonObject SettlementView(Section section, QUuid widget_id, const utils::DateTimeRange& range);

QJsonObject NodeDirectionRule(Section section, QUuid node_id, bool direction_rule, int version);
QJsonObject NodeStatus(Section section, QUuid node_id, int status, int version);
QJsonObject NodeName(Section section, QUuid node_id, CString& name, int version);
QJsonObject NodeUpdate(Section section, QUuid node_id, CJsonObject& update, int version);
QJsonObject DocumentDir(Section section, CString& document_dir);
QJsonObject DefaultUnit(Section section, int unit);

QJsonObject EntryDescriptionSearch(Section section, QUuid widget_id, CString& keyword);
QJsonObject EntryTagSearch(Section section, QUuid widget_id, const QSet<QString>& tags);
QJsonObject OrderNameSearch(Section section, QUuid widget_id, CString& keyword);
QJsonObject OrderTagSearch(Section section, QUuid widget_id, const QSet<QString>& tags);

QJsonObject OrderRecall(Section section, QUuid node_id, CJsonObject& update, int version);
QJsonObject AccountName(CString& email, CString& name);
QJsonObject AccountUsername(CString& email, CString& username);

QJsonObject InventoryHeadAck(Section section, QUuid widget_id, const utils::DateTimeRange& range, int moc, int mpc, int mam);
QJsonObject PartnerHeadAck(Section section, QUuid widget_id, const utils::DateTimeRange& range, int moc, int mid, int mam);
QJsonObject BalanceSheetAck(QUuid widget_id, QUuid asset, QUuid liability, QUuid equity, const utils::DateTimeRange& range, int level);
QJsonObject IncomeStatementAck(QUuid widget_id, QUuid income, QUuid expense, const utils::DateTimeRange& range, int level);
QJsonObject CashFlowStatementAck(QUuid widget_id, const utils::DateTimeRange& range);
QJsonObject OrderOverviewAck(Section section, QUuid widget_id, const utils::DateTimeRange& range);

}
