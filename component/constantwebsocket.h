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

#include <cstdint>

// ================= Naming Rules =================
//
// 1. Data transfer:
//    Fetch -> Client-initiated data retrieval
//    Sync  -> Server-initiated data synchronization
//    Fetch responses may reuse the same key.
//
// 2. Business operations:
//    Key     -> <Subject><Action>
//    Handler -> <Action><Subject>()
//    e.g. kEntryUpdate -> UpdateEntry()
//
// 3. Results and events:
//    Key     -> <Subject><Outcome|Deny|Notify>
//    Handler -> On<MessageName>()
//    e.g. kLoginOutcome -> OnLoginOutcome()
//         kPermissionDeny -> OnPermissionDeny()
//
// 4. Keep the same key name and numeric ID on both ends.
//
// =================================================
enum class WsKey : uint16_t {
    // --- Auth (1000-1099) ---
    kLogin = 1000,
    kLoginOutcome = 1001,

    kRegister = 1010,
    kRegisterOutcome = 1011,

    // --- Config (1100-1199) ---
    kDefaultUnitUpdateDeny = 1100,
    kDefaultUnitUpdate = 1101,

    kDocumentDirUpdate = 1110,

    kPartnerEntrySync = 1120,
    kSharedConfigSync = 1121,

    // --- Audit Log (1200-1299) ---
    kAuditLogFetch = 1200,

    // --- Settlement (1300-1399) ---
    kSettlementPrimaryFetch = 1300,
    kSettlementDelete = 1301,
    kSettlementInsert = 1302,
    kSettlementSecondaryFetch = 1303,
    kSettlementRecall = 1304,
    kSettlementUpdate = 1305,
    kSettlementViewFetch = 1306,

    // --- Tree (1400-1499) ---
    kLeafMerge = 1400,
    kBranchDelete = 1401,
    kLeafDelete = 1402,
    kPartnerLeafDelete = 1403,
    kOrderLeafDelete = 1404,

    kLeafDeleteCheck = 1410,
    kLeafDeleteAllow = 1411,
    kLeafDeleteDeny = 1412,

    kNodeDirectionRuleUpdate = 1420,
    kNodeMove = 1421,
    kNodeInsert = 1422,
    kNodeNameUpdate = 1423,
    kNodeUpdate = 1424,
    kNodeStatusUpdate = 1425,

    kTreeSync = 1430,
    kTreeSyncFinish = 1431,
    kTreeFetch = 1432,
    kNodeFetch = 1433,

    // --- Tag (1500-1599) ---
    kTagSync = 1500,
    kTagDelete = 1501,
    kTagInsert = 1502,
    kTagUpdate = 1503,

    // --- Entry (1600-1699) ---
    kTableFetch = 1600,

    // Entry CRUD
    kEntryInsert = 1601,
    kEntryUpdate = 1602,
    kEntryDelete = 1603,

    // Entry field updates
    kEntryLinkedNodeUpdate = 1610,
    kEntryRateUpdate = 1611,
    kEntryNumericUpdate = 1612,
    kEntryLockedUpdate = 1613,

    // Batch operations
    kEntriesMark = 1620,
    kEntriesLock = 1621,

    // Search
    kEntryDescriptionSearch = 1650,
    kEntryTagSearch = 1651,
    kNodeTagSearch = 1652,
    kNodeNameSearch = 1653,

    // --- Misc (1700-1799) ---
    kOperationDeny = 1700,
    kPermissionDeny = 1710,
    kPermissionDenyOrder = 1720,
    kPermissionDenySettlement = 1730,
    kPermissionDenyNode = 1740,
    kPermissionDenyEntry = 1750,
    kPermissionDenyEntries = 1751,
    kLeafDeleteLockedDeny = 1752,
    kPermissionDenyReimbursement = 1760,

    // --- Profile ---
    kAccountNameUpdate = 1800,
    kAccountUsernameUpdate = 1801,

    // --- Workspace Member / Role ---
    kAccountRoleDelete = 1900,
    kAccountRoleUpdate = 1901,

    kWorkspaceMemberFetch = 1910,
    kWorkspaceMemberDelete = 1911,
    kWorkspaceMemberUpdate = 1912,

    // --- Statement ---
    kStatementPrimaryFetch = 2000,
    kStatementSecondaryFetch = 2001,
    kStatementTertiaryFetch = 2002,

    // --- Reimbursement (2100-2199) ---
    kReimbursementInsert = 2100,
    kReimbursementUpdate = 2101,
    kReimbursementApprove = 2102,
    kReimbursementUnapprove = 2103,
    kReimbursementPost = 2104,
    kReimbursementUnpost = 2105,
    kReimbursementNodeUpdate = 2106,

    // --- Finance (2200-2299) ---
    kBalanceSheetFetch = 2200,
    kCashFlowStatementFetch = 2201,
    kIncomeStatementFetch = 2202,

    kPeriodClose = 2210,

    // --- Task (2300-2399) ---

    // --- Inventory ---
    kInventoryHeatFetch = 2400,

    // --- Partner ---
    kPartnerEntryDelete = 2500,
    kPartnerEntryInsert = 2501,
    kPartnerEntryUpdate = 2502,

    kPartnerHeatFetch = 2520,

    // --- Order ---
    kOrderInsert = 2600,
    kOrderInsertRelease = 2601,
    kUnreleasedOrderSave = 2602,
    kUnreleasedOrderRelease = 2603,
    kOrderRecall = 2604,
    kRecalledOrderSave = 2605,
    kRecalledOrderRelease = 2606,
    kOrderNodeUpdate = 2607,

    kOrderNameSearch = 2610,
    kOrderTagSearch = 2611,

    kOrderNodeFetch = 2620,
    kOrderTreeFetch = 2621,
    kOrderOverviewFetch = 2622,
};

constexpr const char* WsMsgToString(WsKey msg)
{
    switch (msg) {
    case WsKey::kLogin:
        return "kLogin";
    case WsKey::kRegister:
        return "kRegister";
    case WsKey::kLoginOutcome:
        return "kLoginOutcome";
    case WsKey::kRegisterOutcome:
        return "kRegisterOutcome";

    case WsKey::kSharedConfigSync:
        return "kSharedConfigSync";
    case WsKey::kPartnerEntrySync:
        return "kPartnerEntrySync";
    case WsKey::kDefaultUnitUpdate:
        return "kDefaultUnitUpdate";
    case WsKey::kDefaultUnitUpdateDeny:
        return "kDefaultUnitUpdateDeny";
    case WsKey::kDocumentDirUpdate:
        return "kDocumentDirUpdate";

    case WsKey::kOrderTreeFetch:
        return "kOrderTreeFetch";
    case WsKey::kTableFetch:
        return "kTableFetch";
    case WsKey::kStatementPrimaryFetch:
        return "kStatementPrimaryFetch";
    case WsKey::kStatementSecondaryFetch:
        return "kStatementSecondaryFetch";
    case WsKey::kStatementTertiaryFetch:
        return "kStatementTertiaryFetch";
    case WsKey::kSettlementPrimaryFetch:
        return "kSettlementPrimaryFetch";
    case WsKey::kSettlementSecondaryFetch:
        return "kSettlementSecondaryFetch";
    case WsKey::kOrderNodeFetch:
        return "kOrderNodeFetch";

    case WsKey::kSettlementInsert:
        return "kSettlementInsert";
    case WsKey::kSettlementUpdate:
        return "kSettlementUpdate";
    case WsKey::kSettlementRecall:
        return "kSettlementRecall";
    case WsKey::kSettlementDelete:
        return "kSettlementDelete";
    case WsKey::kSettlementViewFetch:
        return "kSettlementViewFetch";

    case WsKey::kTreeFetch:
        return "kTreeFetch";
    case WsKey::kNodeFetch:
        return "kNodeFetch";
    case WsKey::kNodeTagSearch:
        return "kNodeTagSearch";
    case WsKey::kNodeNameSearch:
        return "kNodeNameSearch";

    case WsKey::kTreeSync:
        return "kTreeSync";
    case WsKey::kTreeSyncFinish:
        return "kTreeSyncFinish";
    case WsKey::kNodeInsert:
        return "kNodeInsert";
    case WsKey::kNodeUpdate:
        return "kNodeUpdate";
    case WsKey::kNodeStatusUpdate:
        return "kNodeStatusUpdate";
    case WsKey::kOrderNameSearch:
        return "kOrderNameSearch";
    case WsKey::kNodeNameUpdate:
        return "kNodeNameUpdate";
    case WsKey::kNodeMove:
        return "kNodeMove";
    case WsKey::kLeafDelete:
        return "kLeafDelete";
    case WsKey::kPartnerLeafDelete:
        return "kLeafDeletePartner";
    case WsKey::kOrderLeafDelete:
        return "kLeafDeleteOrder";
    case WsKey::kLeafMerge:
        return "kLeafMerge";
    case WsKey::kBranchDelete:
        return "kBranchDelete";
    case WsKey::kNodeDirectionRuleUpdate:
        return "kNodeDirectionRuleUpdate";

    case WsKey::kLeafDeleteCheck:
        return "kLeafDeleteCheck";
    case WsKey::kLeafDeleteDeny:
        return "kLeafDeleteDeny";
    case WsKey::kLeafDeleteAllow:
        return "kLeafDeleteAllow";

    case WsKey::kTagSync:
        return "kTagSync";
    case WsKey::kTagInsert:
        return "kTagInsert";
    case WsKey::kTagUpdate:
        return "kTagUpdate";
    case WsKey::kTagDelete:
        return "kTagDelete";

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
    case WsKey::kEntryLockedUpdate:
        return "kEntryLockedUpdate";
    case WsKey::kPermissionDenyEntry:
        return "kPermissionDenyEntry";
    case WsKey::kEntriesLock:
        return "kEntriesLock";
    case WsKey::kPermissionDenyEntries:
        return "kPermissionDenyEntries";
    case WsKey::kPartnerEntryUpdate:
        return "kPartnerEntryUpdate";

    case WsKey::kOrderInsert:
        return "kOrderInsert";
    case WsKey::kUnreleasedOrderSave:
        return "kUnreleasedOrderSave";
    case WsKey::kOrderInsertRelease:
        return "kOrderInsertRelease";
    case WsKey::kUnreleasedOrderRelease:
        return "kUnreleasedOrderRelease";
    case WsKey::kOrderRecall:
        return "kOrderRecall";
    case WsKey::kRecalledOrderSave:
        return "kRecalledOrderSave";
    case WsKey::kRecalledOrderRelease:
        return "kRecalledOrderRelease";

    case WsKey::kOperationDeny:
        return "kOperationDeny";
    case WsKey::kPermissionDeny:
        return "kPermissionDeny";
    case WsKey::kPermissionDenyOrder:
        return "kPermissionDenyOrder";
    case WsKey::kPermissionDenySettlement:
        return "kPermissionDenySettlement";
    case WsKey::kPermissionDenyNode:
        return "kPermissionDenyNode";

    case WsKey::kAccountNameUpdate:
        return "kAccountNameUpdate";
    case WsKey::kAccountUsernameUpdate:
        return "kAccountUsernameUpdate";

    case WsKey::kWorkspaceMemberFetch:
        return "kWorkspaceMemberFetch";
    case WsKey::kWorkspaceMemberUpdate:
        return "kWorkspaceMemberUpdate";
    case WsKey::kWorkspaceMemberDelete:
        return "kWorkspaceMemberDelete";
    case WsKey::kAccountRoleUpdate:
        return "kAccountRoleUpdate";
    case WsKey::kAccountRoleDelete:
        return "kAccountRoleDelete";
    case WsKey::kAuditLogFetch:
        return "kAuditLogFetch";
    case WsKey::kOrderTagSearch:
        return "kOrderTagSearch";
    case WsKey::kPeriodClose:
        return "kPeriodClose";
    case WsKey::kInventoryHeatFetch:
        return "kInventoryHeatFetch";
    case WsKey::kPartnerHeatFetch:
        return "kPartnerHeatFetch";
    case WsKey::kBalanceSheetFetch:
        return "kBalanceSheetFetch";
    case WsKey::kIncomeStatementFetch:
        return "kIncomeStatementFetch";
    case WsKey::kCashFlowStatementFetch:
        return "kCashFlowStatementFetch";

    case WsKey::kPartnerEntryDelete:
        return "kPartnerEntryDelete";
    case WsKey::kPartnerEntryInsert:
        return "kPartnerEntryInsert";

    case WsKey::kOrderOverviewFetch:
        return "kOrderOverviewFetch";
    case WsKey::kOrderNodeUpdate:
        return "kOrderNodeUpdate";
    case WsKey::kLeafDeleteLockedDeny:
        return "kLeafDeleteLockedDeny";
    case WsKey::kReimbursementInsert:
        return "kReimbursementInsert";
    case WsKey::kReimbursementUpdate:
        return "kReimbursementUpdate";
    case WsKey::kReimbursementApprove:
        return "kReimbursementApprove";
    case WsKey::kReimbursementUnapprove:
        return "kReimbursementUnapprove";
    case WsKey::kReimbursementPost:
        return "kReimbursementPost";
    case WsKey::kReimbursementUnpost:
        return "kReimbursementUnpost";
    case WsKey::kReimbursementNodeUpdate:
        return "kReimbursementNodeUpdate";
    case WsKey::kPermissionDenyReimbursement:
        return "kPermissionDenyReimbursement";
    }
}
