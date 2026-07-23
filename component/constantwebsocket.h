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
    kOrderNameSearch = 27, // for order node
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
    kEntriesMark = 48,
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
    // --- Audit Log ---
    kAuditLogAck = 65,
    kOrderTagSearch = 66, // for order node
    kPeriodClose = 67,
    kInventoryHeatAck = 68,
    kPartnerHeatAck = 69,
    kBalanceSheetAck = 70,
    kIncomeStatementAck = 71,
    kCashFlowStatementAck = 72,
    // --- Order delivery binding ---
    kBindingRequest = 73, // Client -> Server: request to establish a delivery binding with a partner
    kBindingRequestOutcome = 74, // Server -> Client (requester): reports the outcome of a binding request
    // --- Order delivery ---
    kOrderDeliver = 75, // Client -> Server: request to deliver an order
    kOrderDeliverOutcome = 76, // Server -> Client (sender): reports the outcome of an order delivery attempt
    kOrderNotify = 77, // Server -> Client: notifies the receiver that new delivered orders are available
    kOrderFetch = 78, // Client -> Server: fetch pending delivered orders
    kOrderDetail = 79,
    kOrderAccept = 80,
    // --- Entry Partner ---
    kEntryInsertPartner = 81,
    kEntryDeletePartner = 82,

};

constexpr const char* WsMsgToString(WsKey msg)
{
    switch (msg) {
    // --- Auth ---
    case WsKey::kLogin:
        return "kLogin";
    case WsKey::kRegister:
        return "kRegister";
    case WsKey::kLoginNotify:
        return "kLoginNotify";
    case WsKey::kRegisterNotify:
        return "kRegisterNotify";

        // --- Config ---
    case WsKey::kSharedConfigApply:
        return "kSharedConfigApply";
    case WsKey::kPartnerEntryApply:
        return "kPartnerEntryApply";
    case WsKey::kDefaultUnitUpdate:
        return "kDefaultUnitUpdate";
    case WsKey::kDefaultUnitDeny:
        return "kDefaultUnitDeny";
    case WsKey::kDocumentDirUpdate:
        return "kDocumentDirUpdate";

        // --- Ack ---
    case WsKey::kTreeAck:
        return "kTreeAck";
    case WsKey::kTableAck:
        return "kTableAck";
    case WsKey::kOrderReferenceAck:
        return "kOrderReferenceAck";
    case WsKey::kStatementAck:
        return "kStatementAck";
    case WsKey::kStatementNodeAck:
        return "kStatementNodeAck";
    case WsKey::kStatementEntryAck:
        return "kStatementEntryAck";
    case WsKey::kSettlementAck:
        return "kSettlementAck";
    case WsKey::kSettlementItemAck:
        return "kSettlementItemAck";
    case WsKey::kNodeAck:
        return "kNodeAck";

        // --- Settlement ---
    case WsKey::kSettlementInsert:
        return "kSettlementInsert";
    case WsKey::kSettlementUpdate:
        return "kSettlementUpdate";
    case WsKey::kSettlementRecall:
        return "kSettlementRecall";
    case WsKey::kSettlementDelete:
        return "kSettlementDelete"; // no handler needed

        // --- Tree ---
    case WsKey::kTreeApply:
        return "kTreeApply";
    case WsKey::kTreeSyncFinish:
        return "kTreeSyncFinish";
    case WsKey::kNodeInsert:
        return "kNodeInsert";
    case WsKey::kNodeUpdate:
        return "kNodeUpdate";
    case WsKey::kOrderNameSearch:
        return "kOrderNameSearch";
    case WsKey::kNodeNameUpdate:
        return "kNodeNameUpdate";
    case WsKey::kNodeDrag:
        return "kNodeDrag";
    case WsKey::kLeafDelete:
        return "kLeafDelete";
    case WsKey::kLeafDeleteP:
        return "kLeafDeleteP";
    case WsKey::kLeafDeleteO:
        return "kLeafDeleteO";
    case WsKey::kLeafReplace:
        return "kLeafReplace";
    case WsKey::kBranchDelete:
        return "kBranchDelete";
    case WsKey::kNodeDirectionRuleUpdate:
        return "kNodeDirectionRuleUpdate";

        // --- Leaf delete flow ---
    case WsKey::kLeafDeleteCheck:
        return "kLeafDeleteCheck";
    case WsKey::kLeafDeleteDeny:
        return "kLeafDeleteDeny";
    case WsKey::kLeafDeleteAllow:
        return "kLeafDeleteAllow";

        // --- Tag ---
    case WsKey::kTagApply:
        return "kTagApply";
    case WsKey::kTagInsert:
        return "kTagInsert";
    case WsKey::kTagUpdate:
        return "kTagUpdate";
    case WsKey::kTagDelete:
        return "kTagDelete";

        // --- Entry ---
    case WsKey::kEntryInsert:
        return "kEntryInsert";
    case WsKey::kEntryUpdate:
        return "kEntryUpdate";
    case WsKey::kEntryDelete:
        return "kEntryDelete";
    case WsKey::kEntryDescriptionSearch:
        return "kEntryDescriptionSearch";
    case WsKey::kEntryTagSearch:
        return "kEntryTagSearch";
    case WsKey::kEntriesMark:
        return "kEntriesMark";
    case WsKey::kEntryLinkedNodeUpdate:
        return "kEntryLinkedNodeUpdate";
    case WsKey::kEntryRateUpdate:
        return "kEntryRateUpdate";
    case WsKey::kEntryNumericUpdate:
        return "kEntryNumericUpdate";

        // --- Order ---
    case WsKey::kOrderInsertSave:
        return "kOrderInsertSave";
    case WsKey::kOrderUpdateSave:
        return "kOrderUpdateSave";
    case WsKey::kOrderInsertRelease:
        return "kOrderInsertRelease";
    case WsKey::kOrderUpdateRelease:
        return "kOrderUpdateRelease";
    case WsKey::kOrderRecall:
        return "kOrderRecall";

        // --- Misc ---
    case WsKey::kOperationDeny:
        return "kOperationDeny";

        // --- Profile ---
    case WsKey::kAccountNameUpdate:
        return "kAccountNameUpdate";
    case WsKey::kAccountUsernameUpdate:
        return "kAccountUsernameUpdate";

        // --- Workspace Member ---
    case WsKey::kWorkspaceMemberAck:
        return "kWorkspaceMemberAck";
    case WsKey::kWorkspaceMemberUpdate:
        return "kWorkspaceMemberUpdate";
    case WsKey::kWorkspaceMemberDelete:
        return "kWorkspaceMemberDelete";
    case WsKey::kAccountRoleUpdate:
        return "kAccountRoleUpdate";
    case WsKey::kAccountRoleDelete:
        return "kAccountRoleDelete";
    case WsKey::kAuditLogAck:
        return "kAuditLogAck";
    case WsKey::kOrderTagSearch:
        return "kOrderTagSearch";
    case WsKey::kPeriodClose:
        return "kPeriodClose";
    case WsKey::kInventoryHeatAck:
        return "kInventoryHeatAck";
    case WsKey::kPartnerHeatAck:
        return "kPartnerHeatAck";
    case WsKey::kBalanceSheetAck:
        return "kBalanceSheetAck";
    case WsKey::kIncomeStatementAck:
        return "kIncomeStatementAck";
    case WsKey::kCashFlowStatementAck:
        return "kCashFlowStatementAck";

    case WsKey::kEntryDeletePartner:
        return "kEntryDeletePartner";
    case WsKey::kEntryInsertPartner:
        return "kEntryInsertPartner";

        // --- Order delivery binding ---
    case WsKey::kBindingRequest:
        return "kBindingRequest";
    case WsKey::kBindingRequestOutcome:
        return "kBindingRequestOutcome";
        // --- Order delivery ---
    case WsKey::kOrderDeliver:
        return "kOrderDeliver";
    case WsKey::kOrderDeliverOutcome:
        return "kOrderDeliverOutcome";
    case WsKey::kOrderNotify:
        return "kOrderNotify";
    case WsKey::kOrderFetch:
        return "kOrderFetch";
    case WsKey::kOrderDetail:
        return "kOrderDetail";
    case WsKey::kOrderAccept:
        return "kOrderAccept";
    }
}

namespace WsField {
inline const QString kMarkOperation { QStringLiteral("mark_operation") };
inline const QString kNodeUpdate { QStringLiteral("node_update") };
inline const QString kDefaultUnit { QStringLiteral("default_unit") };
inline const QString kDocumentDir { QStringLiteral("document_dir") };
inline const QString kKey { QStringLiteral("key") };
inline const QString kValue { QStringLiteral("value") };
}

#endif // CONSTANTWEBSOCKET_H
