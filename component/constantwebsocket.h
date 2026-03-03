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

#ifndef CONSTANTWEBSOCKET_H
#define CONSTANTWEBSOCKET_H

#include <QString>

// ================= WS Key Naming Convention =================
//
// Format:
//   <subject>_<verb>
//   <subject>_<qualifier>_<verb>
//
// Subject:
//   - Use singular business noun (node, entry, settlement, tag)
//   - Lowercase snake_case
//
// Verb:
//   - Always base form: insert, update, delete, search, apply, recall, ack, notify, allow, deny
//   - Direction semantics (soft convention):
//       • apply -> server actively pushes this message to client
//       • ack   -> client sends request, server acknowledges
//       • notify -> server informs client of events (similar to apply)
//   - Other verbs follow context-dependent semantics
//
// Note:
//   Message direction (client/server) is context-dependent.
//   The same key may be used in both directions.
//
// Notes:
//   - Avoid tense variants (inserted / updating / deletion)
//   - Avoid plural subjects
//   - Key verb is suffix, handler function verb is prefix
//     e.g. "node_insert" -> InsertNode(), "login_notify" -> NotifyLogin()
// =============================================================

namespace WsKey {
// ---------------- Auth ----------------
// Client requests
inline const QString kLogin { QStringLiteral("login") }; // send-only: client login request
inline const QString kRegister { QStringLiteral("register") }; // send-only: client register request
// Server responses
inline const QString kLoginNotify = QStringLiteral("login_notify"); // server response for login
inline const QString kRegisterNotify = QStringLiteral("register_notify"); // server response for register

// Config
inline const QString kSharedConfigApply { QStringLiteral("shared_config_apply") };
inline const QString kPartnerEntryApply { QStringLiteral("partner_entry_apply") };
inline const QString kDefaultUnitUpdate { QStringLiteral("default_unit_update") };
inline const QString kDefaultUnitDeny { QStringLiteral("default_unit_deny") };
inline const QString kDocumentDirUpdate { QStringLiteral("document_dir_update") };

// Ack
inline const QString kTreeAck { QStringLiteral("tree_ack") };
inline const QString kTableAck { QStringLiteral("table_ack") };
inline const QString kOrderReferenceAck { QStringLiteral("order_reference_ack") };
inline const QString kStatementAck { QStringLiteral("statement_ack") };
inline const QString kStatementNodeAck { QStringLiteral("statement_node_ack") };
inline const QString kStatementEntryAck { QStringLiteral("statement_entry_ack") };
inline const QString kSettlementAck { QStringLiteral("settlement_ack") };
inline const QString kSettlementItemAck { QStringLiteral("settlement_item_ack") };
inline const QString kNodeAck { QStringLiteral("node_ack") };

// ⚠️ no handler needed
// server never emits this message
// recall already synchronizes client state
inline const QString kSettlementDelete = QStringLiteral("settlement_delete");

// Settlement
inline const QString kSettlementInsert { QStringLiteral("settlement_insert") };
inline const QString kSettlementUpdate { QStringLiteral("settlement_update") };
inline const QString kSettlementRecall { QStringLiteral("settlement_recall") };

// Tree
inline const QString kTreeApply { QStringLiteral("tree_apply") };
inline const QString kTreeSyncFinish { QStringLiteral("tree_sync_finish") };
inline const QString kNodeInsert { QStringLiteral("node_insert") };
inline const QString kNodeUpdate { QStringLiteral("node_update") };
inline const QString kNodeSearch { QStringLiteral("node_search") };
inline const QString kNodeNameUpdate { QStringLiteral("node_name_update") };
inline const QString kNodeDrag { QStringLiteral("node_drag") };
inline const QString kLeafDelete { QStringLiteral("leaf_delete") };
inline const QString kLeafReplace { QStringLiteral("leaf_replace") };
inline const QString kBranchDelete { QStringLiteral("branch_delete") };
inline const QString kNodeDirectionRuleUpdate { QStringLiteral("node_direction_rule_update") };

// --- Leaf delete flow ---
// client send: kLeafDeleteCheck
// server response:
//   - kLeafDeleteDenied
//   - kLeafDeleteSafely
inline const QString kLeafDeleteCheck = QStringLiteral("leaf_delete_check"); // send-only
inline const QString kLeafDeleteDeny = QStringLiteral("leaf_delete_deny"); // push
inline const QString kLeafDeleteAllow = QStringLiteral("leaf_delete_allow"); // push

// Tag
inline const QString kTagApply { QStringLiteral("tag_apply") };
inline const QString kTagInsert { QStringLiteral("tag_insert") };
inline const QString kTagUpdate { QStringLiteral("tag_update") };
inline const QString kTagDelete { QStringLiteral("tag_delete") };

// Entry
inline const QString kEntryInsert { QStringLiteral("entry_insert") };
inline const QString kEntryUpdate { QStringLiteral("entry_update") };
inline const QString kEntryDelete { QStringLiteral("entry_delete") };
inline const QString kEntryDescriptionSearch { QStringLiteral("entry_description_search") };
inline const QString kEntryTagSearch { QStringLiteral("entry_tag_search") };
inline const QString kBatchMark { QStringLiteral("batch_mark") };
inline const QString kEntryLinkedNodeUpdate { QStringLiteral("entry_linked_node_update") };
inline const QString kEntryRateUpdate { QStringLiteral("entry_rate_update") };
inline const QString kEntryNumericUpdate { QStringLiteral("entry_numeric_update") };

// Order
inline const QString kOrderInsertSave { QStringLiteral("order_insert_save") };
inline const QString kOrderUpdateSave { QStringLiteral("order_update_save") };
inline const QString kOrderInsertRelease { QStringLiteral("order_insert_release") };
inline const QString kOrderUpdateRelease { QStringLiteral("order_update_release") };
inline const QString kOrderRecall { QStringLiteral("order_recall") };

// Misc
inline const QString kOperationDeny { QStringLiteral("operation_deny") };
}

namespace WsField {
inline const QString kMark { QStringLiteral("mark") };
inline const QString kNodeUpdate { QStringLiteral("node_update") };
inline const QString kDefaultUnit { QStringLiteral("default_unit") };
inline const QString kDocumentDir { QStringLiteral("document_dir") };
}

#endif // CONSTANTWEBSOCKET_H
