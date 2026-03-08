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
    kRegister,
    kLoginNotify,
    kRegisterNotify,

    // --- Config ---
    kSharedConfigApply,
    kPartnerEntryApply,
    kDefaultUnitUpdate,
    kDefaultUnitDeny,
    kDocumentDirUpdate,

    // --- Ack ---
    kTreeAck,
    kTableAck,
    kOrderReferenceAck,
    kStatementAck,
    kStatementNodeAck,
    kStatementEntryAck,
    kSettlementAck,
    kSettlementItemAck,
    kNodeAck,

    // --- Settlement ---
    kSettlementInsert,
    kSettlementUpdate,
    kSettlementRecall,

    // ⚠️ no handler needed
    // server never emits this message
    // recall already synchronizes client state
    kSettlementDelete,

    // --- Tree ---
    kTreeApply,
    kTreeSyncFinish,
    kNodeInsert,
    kNodeUpdate,
    kNodeSearch,
    kNodeNameUpdate,
    kNodeDrag,
    kLeafDelete,
    kLeafDeleteP,
    kLeafDeleteO,
    kLeafReplace,
    kBranchDelete,
    kNodeDirectionRuleUpdate,

    // --- Leaf delete flow ---
    kLeafDeleteCheck,
    kLeafDeleteDeny,
    kLeafDeleteAllow,

    // --- Tag ---
    kTagApply,
    kTagInsert,
    kTagUpdate,
    kTagDelete,

    // --- Entry ---
    kEntryInsert,
    kEntryUpdate,
    kEntryDelete,
    kEntryDescriptionSearch,
    kEntryTagSearch,
    kBatchMark,
    kEntryLinkedNodeUpdate,
    kEntryRateUpdate,
    kEntryNumericUpdate,

    // --- Order ---
    kOrderInsertSave,
    kOrderUpdateSave,
    kOrderInsertRelease,
    kOrderUpdateRelease,
    kOrderRecall,

    // --- Misc ---
    kOperationDeny
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
