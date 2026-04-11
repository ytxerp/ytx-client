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
enum class WsKey : uint8_t {
    // --- Auth ---
    kLogin = 1,
    kRegister = 2,
    kLoginNotify = 3,
    kRegisterNotify = 4,
    // --- Config ---
    kSharedConfigApply = 5,
    kPartnerEntryApply = 6,
    kDefaultUnitUpdate = 7,
    kDefaultUnitDeny = 8,
    kDocumentDirUpdate = 9,
    // --- Ack ---
    kTreeAck = 10,
    kTableAck = 11,
    kOrderReferenceAck = 12,
    kStatementAck = 13,
    kStatementNodeAck = 14,
    kStatementEntryAck = 15,
    kSettlementAck = 16,
    kSettlementItemAck = 17,
    kNodeAck = 18,
    // --- Settlement ---
    kSettlementInsert = 19,
    kSettlementUpdate = 20,
    kSettlementRecall = 21,
    // ⚠️ no handler needed
    // server never emits this message
    // recall already synchronizes client state
    kSettlementDelete = 22,
    // --- Tree ---
    kTreeApply = 23,
    kTreeSyncFinish = 24,
    kNodeInsert = 25,
    kNodeUpdate = 26,
    kNodeSearch = 27,
    kNodeNameUpdate = 28,
    kNodeDrag = 29,
    kLeafDelete = 30,
    kLeafDeleteP = 31,
    kLeafDeleteO = 32,
    kLeafReplace = 33,
    kBranchDelete = 34,
    kNodeDirectionRuleUpdate = 35,
    // --- Leaf delete flow ---
    kLeafDeleteCheck = 36,
    kLeafDeleteDeny = 37,
    kLeafDeleteAllow = 38,
    // --- Tag ---
    kTagApply = 39,
    kTagInsert = 40,
    kTagUpdate = 41,
    kTagDelete = 42,
    // --- Entry ---
    kEntryInsert = 43,
    kEntryUpdate = 44,
    kEntryDelete = 45,
    kEntryDescriptionSearch = 46,
    kEntryTagSearch = 47,
    kBatchMark = 48,
    kEntryLinkedNodeUpdate = 49,
    kEntryRateUpdate = 50,
    kEntryNumericUpdate = 51,
    // --- Order ---
    kOrderInsertSave = 52,
    kOrderUpdateSave = 53,
    kOrderInsertRelease = 54,
    kOrderUpdateRelease = 55,
    kOrderRecall = 56,
    // --- Misc ---
    kOperationDeny = 57,
    // --- Profile ---
    kAccountNameUpdate = 58,
    kAccountUsernameUpdate = 59,
    // --- Workspace Member ---
    kWorkspaceMemberAck = 60,
    kWorkspaceMemberUpdate = 61,
    kWorkspaceMemberDelete = 62,
    // --- Account Role ---
    kAccountRoleUpdate = 63,
    kAccountRoleDelete = 64,
    // --- Entry ---
    kEntryIssuedTimeUpdate = 65,
    // -- Audit Log ---
    kAuditLogAck = 66,

};

constexpr const char* WsMsgToString(WsKey msg)
{
    switch (msg) {
    // --- Auth ---
    case WsKey::kLogin:
        return "login";
    case WsKey::kRegister:
        return "register";
    case WsKey::kLoginNotify:
        return "login_notify";
    case WsKey::kRegisterNotify:
        return "register_notify";

    // --- Config ---
    case WsKey::kSharedConfigApply:
        return "shared_config_apply";
    case WsKey::kPartnerEntryApply:
        return "partner_entry_apply";
    case WsKey::kDefaultUnitUpdate:
        return "default_unit_update";
    case WsKey::kDefaultUnitDeny:
        return "default_unit_deny";
    case WsKey::kDocumentDirUpdate:
        return "document_dir_update";

    // --- Ack ---
    case WsKey::kTreeAck:
        return "tree_ack";
    case WsKey::kTableAck:
        return "table_ack";
    case WsKey::kOrderReferenceAck:
        return "order_reference_ack";
    case WsKey::kStatementAck:
        return "statement_ack";
    case WsKey::kStatementNodeAck:
        return "statement_node_ack";
    case WsKey::kStatementEntryAck:
        return "statement_entry_ack";
    case WsKey::kSettlementAck:
        return "settlement_ack";
    case WsKey::kSettlementItemAck:
        return "settlement_item_ack";
    case WsKey::kNodeAck:
        return "node_ack";

    // --- Settlement ---
    case WsKey::kSettlementInsert:
        return "settlement_insert";
    case WsKey::kSettlementUpdate:
        return "settlement_update";
    case WsKey::kSettlementRecall:
        return "settlement_recall";
    case WsKey::kSettlementDelete:
        return "settlement_delete"; // no handler needed

    // --- Tree ---
    case WsKey::kTreeApply:
        return "tree_apply";
    case WsKey::kTreeSyncFinish:
        return "tree_sync_finish";
    case WsKey::kNodeInsert:
        return "node_insert";
    case WsKey::kNodeUpdate:
        return "node_update";
    case WsKey::kNodeSearch:
        return "node_search";
    case WsKey::kNodeNameUpdate:
        return "node_name_update";
    case WsKey::kNodeDrag:
        return "node_drag";
    case WsKey::kLeafDelete:
        return "leaf_delete";
    case WsKey::kLeafDeleteP:
        return "leaf_delete_p";
    case WsKey::kLeafDeleteO:
        return "leaf_delete_o";
    case WsKey::kLeafReplace:
        return "leaf_replace";
    case WsKey::kBranchDelete:
        return "branch_delete";
    case WsKey::kNodeDirectionRuleUpdate:
        return "node_direction_rule_update";

    // --- Leaf delete flow ---
    case WsKey::kLeafDeleteCheck:
        return "leaf_delete_check";
    case WsKey::kLeafDeleteDeny:
        return "leaf_delete_deny";
    case WsKey::kLeafDeleteAllow:
        return "leaf_delete_allow";

    // --- Tag ---
    case WsKey::kTagApply:
        return "tag_apply";
    case WsKey::kTagInsert:
        return "tag_insert";
    case WsKey::kTagUpdate:
        return "tag_update";
    case WsKey::kTagDelete:
        return "tag_delete";

    // --- Entry ---
    case WsKey::kEntryInsert:
        return "entry_insert";
    case WsKey::kEntryUpdate:
        return "entry_update";
    case WsKey::kEntryDelete:
        return "entry_delete";
    case WsKey::kEntryDescriptionSearch:
        return "entry_description_search";
    case WsKey::kEntryTagSearch:
        return "entry_tag_search";
    case WsKey::kBatchMark:
        return "batch_mark";
    case WsKey::kEntryLinkedNodeUpdate:
        return "entry_linked_node_update";
    case WsKey::kEntryRateUpdate:
        return "entry_rate_update";
    case WsKey::kEntryNumericUpdate:
        return "entry_numeric_update";

    // --- Order ---
    case WsKey::kOrderInsertSave:
        return "order_insert_save";
    case WsKey::kOrderUpdateSave:
        return "order_update_save";
    case WsKey::kOrderInsertRelease:
        return "order_insert_release";
    case WsKey::kOrderUpdateRelease:
        return "order_update_release";
    case WsKey::kOrderRecall:
        return "order_recall";

    // --- Misc ---
    case WsKey::kOperationDeny:
        return "operation_deny";

    // --- Profile ---
    case WsKey::kAccountNameUpdate:
        return "account_name_update";
    case WsKey::kAccountUsernameUpdate:
        return "account_username_update";

    // --- Workspace Member ---
    case WsKey::kWorkspaceMemberAck:
        return "workspace_member_ack";
    case WsKey::kWorkspaceMemberUpdate:
        return "workspace_member_update";
    case WsKey::kWorkspaceMemberDelete:
        return "workspace_member_delete";
    case WsKey::kAccountRoleUpdate:
        return "account_role_update";
    case WsKey::kAccountRoleDelete:
        return "account_role_delete";
    case WsKey::kEntryIssuedTimeUpdate:
        return "entry_issued_time_update";
    case WsKey::kAuditLogAck:
        return "audit_log_ack";
    default:
        return "unknown";
    }
}

namespace WsField {
inline const QString kMark { QStringLiteral("mark") };
inline const QString kNodeUpdate { QStringLiteral("node_update") };
inline const QString kDefaultUnit { QStringLiteral("default_unit") };
inline const QString kDocumentDir { QStringLiteral("document_dir") };
inline const QString kKey { QStringLiteral("key") };
inline const QString kValue { QStringLiteral("value") };
}

#endif // CONSTANTWEBSOCKET_H
