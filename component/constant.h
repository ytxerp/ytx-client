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

#ifndef CONSTVALUE_H
#define CONSTVALUE_H

#include <QString>
#include <QTime>

// Numeric constants
inline constexpr int kCoefficient5 = 5;
inline constexpr int kCoefficient8 = 8;
inline constexpr int kCoefficient16 = 16;

inline constexpr double kTolerance = 1e-8;
inline bool FloatEqual(double a, double b) noexcept { return std::abs(a - b) < kTolerance; }
inline bool FloatChanged(double a, double b) noexcept { return !FloatEqual(a, b); }

inline constexpr int kRowHeight = 24;
inline constexpr int kThreeThousand = 3000;
inline constexpr int kTwoThousand = 2000;

inline constexpr int HEARTBEAT_INTERVAL = 20000;
inline constexpr int TIMEOUT_THRESHOLD = 35000;

namespace Pool {
inline constexpr qsizetype kExpandSize { 100 };
inline constexpr qsizetype kMaxSize { 1000 };
}

// Boolean constants
inline constexpr bool kIsHidden = true;

namespace Rule {
// Finance/Inventory/Task: Credit increase, Debit decrease; calculation: credit - debit
// Order: Refund Order (returned transaction)
inline constexpr bool kDDCI = true;
inline constexpr bool kRO = true;

// Finance/Inventory/Task: Debit increase, Credit decrease; calculation: debit - credit
// Order: Trade Order (normal transaction)
inline constexpr bool kDICD = false;
inline constexpr bool kTO = false;

inline const QString kStrDDCI = QStringLiteral("DDCI");
inline const QString kStrDICD = QStringLiteral("DICD");

inline const QString kStrTO = QStringLiteral("TO");
inline const QString kStrRO = QStringLiteral("RO");
}

inline constexpr double kDoubleMax = std::numeric_limits<double>::max();
inline constexpr double kDoubleLowest = std::numeric_limits<double>::lowest();

inline constexpr int kMaxNumericScale_4 = 4;
inline constexpr int kMaxNumericScale_8 = 8;

// Time constants
inline const QString kDateTimeFST = QStringLiteral("yyyy-MM-dd HH:mm");
inline const QString kDateFST = QStringLiteral("yyyy-MM-dd");
inline const QString kMonthFST = QStringLiteral("yyyyMM");

inline const QTime kStartTime = { 0, 0 }; // 00:00

// Empty & punctuation
inline const QString kEmptyString = {};
inline const QString kColon = QStringLiteral(":");
inline const QString kDash = QStringLiteral("-");
inline const QString kHalfWidthPeriod = QStringLiteral(".");
inline const QString kFullWidthPeriod = QStringLiteral("。");
inline const QString kSemicolon = QStringLiteral(";");
inline const QString kSlash = QStringLiteral("/");

inline const QString kSeparator = QStringLiteral("separator");

// Operator symbols
inline const QString kMinux = QStringLiteral("-");
inline const QString kPlus = QStringLiteral("+");

// File suffixes
inline const QString kDotSuffixINI = QStringLiteral(".ini");
inline const QString kDotSuffixXLSX = QStringLiteral(".xlsx");

// Node rule, kind, unit

inline const QString kBranchKind = QStringLiteral("B");
inline const QString kLeafKind = QStringLiteral("L");

inline const QString kUnitTarget = QStringLiteral("TGT");
inline const QString kUnitAction = QStringLiteral("ACT");
inline const QString kUnitSource = QStringLiteral("SRC");
inline const QString kUnitInternal = QStringLiteral("INT");
inline const QString kUnitPosition = QStringLiteral("POS");
inline const QString kUnitExternal = QStringLiteral("EXT");
inline const QString kUnitCustomer = QStringLiteral("CUS");
inline const QString kUnitEmployee = QStringLiteral("EMP");
inline const QString kUnitVendor = QStringLiteral("VEN");
inline const QString kUnitImmediate = QStringLiteral("IMM");
inline const QString kUnitMonthly = QStringLiteral("MON");
inline const QString kUnitPending = QStringLiteral("PEN");

// App ini keys
inline const QString kGeometry = QStringLiteral("geometry");
inline const QString kTreeHeaderState = QStringLiteral("tree_header_state");
inline const QString kTableHeaderState = QStringLiteral("table_header_state");
inline const QString kMainwindow = QStringLiteral("mainwindow");
inline const QString kPrinter = QStringLiteral("printer");
inline const QString kCompanyName = QStringLiteral("company_name");
inline const QString kSection = QStringLiteral("section");
inline const QString kIsParallel = QStringLiteral("is_parallel");
inline const QString kSplitter = QStringLiteral("splitter");
inline const QString kStart = QStringLiteral("start");
inline const QString kEnd = QStringLiteral("end");
inline const QString kStartSection = QStringLiteral("start/section");
inline const QString kTheme = QStringLiteral("theme");

// Client-initiated data requests
inline const QString kTreeAcked = QStringLiteral("tree_acked");
inline const QString kNodeAcked = QStringLiteral("node_acked");
inline const QString kTableAcked = QStringLiteral("table_acked");
inline const QString kLinkedEntry = QStringLiteral("linked_entry");
inline const QString kSaleReferenceAcked = QStringLiteral("sale_reference_acked");
inline const QString kStatementAcked = QStringLiteral("statement_acked");
inline const QString kStatementNodeAcked = QStringLiteral("statement_node_acked");
inline const QString kStatementEntryAcked = QStringLiteral("statement_entry_acked");
inline const QString kSettlementAcked = QStringLiteral("settlement_acked");
inline const QString kSettlementItemAcked = QStringLiteral("settlement_item_acked");
inline const QString kSettlementRemoved = QStringLiteral("settlement_removed");
inline const QString kSettlementInsertValidationFailed = QStringLiteral("settlement_insert_validation_failed");
inline const QString kSettlementUpdateValidationFailed = QStringLiteral("settlement_update_validation_failed");

// Server-initiated data push
inline const QString kTreeApplied = QStringLiteral("tree_applied");
inline const QString kSharedConfig = QStringLiteral("shared_config");

// Decimal config
inline const QString kAmountDecimal = QStringLiteral("amount_decimal");
inline const QString kRateDecimal = QStringLiteral("rate_decimal");
inline const QString kQuantityDecimal = QStringLiteral("quantity_decimal");

// Other ini keys
inline const QString kDateFormat = QStringLiteral("date_format");
inline const QString kDynamicLabel = QStringLiteral("dynamic_label");
inline const QString kDynamicNodeLhs = QStringLiteral("dynamic_node_lhs");
inline const QString kDynamicNodeRhs = QStringLiteral("dynamic_node_rhs");
inline const QString kOperation = QStringLiteral("operation");
inline const QString kStaticLabel = QStringLiteral("static_label");
inline const QString kStaticNode = QStringLiteral("static_node");

inline const QString kLeafRemoveCheck = QStringLiteral("leaf_remove_check");
inline const QString kLeafRemoveDenied = QStringLiteral("leaf_remove_denied");
inline const QString kLeafRemove = QStringLiteral("leaf_remove");
inline const QString kLeafReplace = QStringLiteral("leaf_replace");

inline const QString kExternalSku = QStringLiteral("external_sku");
inline const QString kInternalSku = QStringLiteral("internal_sku");

// Node is referenced by internal linked columns within the same section
inline const QString kInsideRef = QStringLiteral("inside_ref");

// Internal inventory node is referenced by other sections, e.g., Partner, Sale, Purchase
inline const QString kInventoryIntRef = QStringLiteral("inventory_int_ref");

// External inventory node is referenced by other sections, e.g., Partner, Sale, Purchase
inline const QString kInventoryExtRef = QStringLiteral("inventory_ext_ref");

// Internal or External inventory node is referenced by other sections, e.g., Partner, Sale, Purchase
inline const QString kInventoryOutsideRef = QStringLiteral("inventory_outside_ref");

// Partner node (customer or supplier) is referenced in other sections, e.g., Sale, Purchase
inline const QString kPartnerRef = QStringLiteral("partner_ref");

// Employee node (staff) is referenced in other sections, e.g., Sale, Purchase
inline const QString kEmployeeRef = QStringLiteral("employee_ref");

// Node from Sale or Purchase is referenced by settlement entries
inline const QString kSettlementRef = QStringLiteral("settlement_ref");

inline const QString kNodeInsert = QStringLiteral("node_insert");
inline const QString kNodeDrag = QStringLiteral("node_drag");
inline const QString kNodeUpdate = QStringLiteral("node_update");
inline const QString kNodeName = QStringLiteral("node_name");

inline const QString kOrderInsertSaved = QStringLiteral("order_insert_saved");
inline const QString kOrderUpdateSaved = QStringLiteral("order_update_saved");
inline const QString kOrderInsertReleased = QStringLiteral("order_insert_released");
inline const QString kOrderUpdateReleased = QStringLiteral("order_update_released");
inline const QString kOrderRecalled = QStringLiteral("order_recalled");

inline const QString kSettlementInserted = QStringLiteral("settlement_inserted");
inline const QString kSettlementUpdated = QStringLiteral("settlement_updated");
inline const QString kSettlementRecalled = QStringLiteral("settlement_recalled");
inline const QString kSettlementItemDeleted = QStringLiteral("settlement_item_deleted");
inline const QString kSettlementItemInserted = QStringLiteral("settlement_item_inserted");
inline const QString kPartnerUpdated = QStringLiteral("partner_updated");

inline const QString kNodeTotal = QStringLiteral("node_total");
inline const QString kDeletedEntryArray = QStringLiteral("deleted_entry_array");
inline const QString kInsertedEntryArray = QStringLiteral("inserted_entry_array");
inline const QString kUpdatedEntryArray = QStringLiteral("updated_entry_array");

// App language
inline const QString kEnUS = QStringLiteral("en_US");
inline const QString kLanguage = QStringLiteral("language");
inline const QString kZhCN = QStringLiteral("zh_CN");
inline const QString kSolarizedDark = QStringLiteral("Solarized Dark");
inline const QSet<QString> kSupportedLanguages { kZhCN, kEnUS };

// Database & connection
inline const QString kLogin = QStringLiteral("login");
inline const QString kEmail = QStringLiteral("email");
inline const QString kPassword = QStringLiteral("password");
inline const QString kKeyword = QStringLiteral("keyword");
inline const QString kRegister = QStringLiteral("register");
inline const QString kRegisterResult = QStringLiteral("register_result");
inline const QString kWorkspaceAccessPending = QStringLiteral("workspace_access_pending");
inline const QString kServer = QStringLiteral("server");
inline const QString kLoginResult = QStringLiteral("login_result");
inline const QString kWorkspace = QStringLiteral("workspace");
inline const QString kHost = QStringLiteral("host");
inline const QString kBranchRemove = QStringLiteral("branch_remove");
inline const QString kNode = QStringLiteral("node");
inline const QString kNodeSearch = QStringLiteral("node_search");
inline const QString kNodeArray = QStringLiteral("node_array");
inline const QString kPath = QStringLiteral("path");
inline const QString kPathArray = QStringLiteral("path_array");
inline const QString kMeta = QStringLiteral("meta");
inline const QString kEntry = QStringLiteral("entry");
inline const QString kPort = QStringLiteral("port");
inline const QString kSourceConnection = QStringLiteral("source_connection");
inline const QString kUser = QStringLiteral("user");

inline const QString kEntryInsert = QStringLiteral("entry_insert");
inline const QString kEntryUpdate = QStringLiteral("entry_update");
inline const QString kEntryRemove = QStringLiteral("entry_remove");
inline const QString kEntrySearch = QStringLiteral("entry_search");
inline const QString kEntryArray = QStringLiteral("entry_array");
inline const QString kUnsettledOrder = QStringLiteral("unsettled_order");
inline const QString kEntryLinkedNode = QStringLiteral("entry_linked_node");
inline const QString kEntryRate = QStringLiteral("entry_rate");
inline const QString kUpdateEntryDebit = QStringLiteral("update_entry_debit");
inline const QString kUpdateEntryCredit = QStringLiteral("update_entry_credit");
inline const QString kEntryNumeric = QStringLiteral("entry_numeric");

inline const QString kTotal = QStringLiteral("total");
inline const QString kTotalArray = QStringLiteral("total_array");
inline const QString kArray = QStringLiteral("array");
inline const QString kDocumentDir = QStringLiteral("document_dir");
inline const QString kDefaultUnit = QStringLiteral("default_unit");
inline const QString kUpdateDefaultUnitFailure = QStringLiteral("update_default_unit_failure");
inline const QString kLeafRemoveSafely = QStringLiteral("leaf_remove_safely");

inline const QString kPBalance = QStringLiteral("pbalance");
inline const QString kCCount = QStringLiteral("ccount");
inline const QString kCMeasure = QStringLiteral("cmeasure");
inline const QString kCAmount = QStringLiteral("camount");
inline const QString kCBalance = QStringLiteral("cbalance");
inline const QString kCSettlement = QStringLiteral("csettlement");

// Message fields
inline const QString kValue = QStringLiteral("value");

// Tree & table columns
inline const QString kCode = QStringLiteral("code");
inline const QString kColor = QStringLiteral("color");
inline const QString kCommission = QStringLiteral("commission");
inline const QString kDescription = QStringLiteral("description");
inline const QString kDirectionRule = QStringLiteral("direction_rule");
inline const QString kName = QStringLiteral("name");
inline const QString kDocument = QStringLiteral("document");
inline const QString kEmployee = QStringLiteral("employee");
inline const QString kCount = QStringLiteral("count");
inline const QString kCountTotal = QStringLiteral("count_total");
inline const QString kMeasure = QStringLiteral("measure");
inline const QString kAmount = QStringLiteral("amount");
inline const QString kMeasureTotal = QStringLiteral("measure_total");

inline const QString kId = QStringLiteral("id");
inline const QString kUserId = QStringLiteral("user_id");
inline const QString kSessionId = QStringLiteral("session_id");
inline const QString kEntryId = QStringLiteral("entry_id");
inline const QString kNodeId = QStringLiteral("node_id");
inline const QString kParentId = QStringLiteral("parent_id");
inline const QString kOrderId = QStringLiteral("order_id");
inline const QString kWidgetId = QStringLiteral("widget_id");
inline const QString kParentWidgetId = QStringLiteral("parent_widget_id");
inline const QString kOldNodeId = QStringLiteral("old_node_id");
inline const QString kNewNodeId = QStringLiteral("new_node_id");
inline const QString kExpireTime = QStringLiteral("expire_time");
inline const QString kSettlement = QStringLiteral("settlement");
inline const QString kSettlementId = QStringLiteral("settlement_id");
inline const QString kIsSettled = QStringLiteral("is_settled");
inline const QString kUpdate = QStringLiteral("update");
inline const QString kResult = QStringLiteral("result");
inline const QString kLhsTotal = QStringLiteral("lhs_total");
inline const QString kRhsTotal = QStringLiteral("rhs_total");
inline const QString kIsSelected = QStringLiteral("is_selected");

inline const QString kLhsRate = QStringLiteral("lhs_rate");
inline const QString kRhsRate = QStringLiteral("rhs_rate");
inline const QString kLhsNumeric = QStringLiteral("lhs_numeric");
inline const QString kRhsNumeric = QStringLiteral("rhs_numeric");
inline const QString kLhsDebit = QStringLiteral("lhs_debit");
inline const QString kLhsCredit = QStringLiteral("lhs_credit");
inline const QString kRhsDebit = QStringLiteral("rhs_debit");
inline const QString kRhsCredit = QStringLiteral("rhs_credit");

inline const QString kInitial = QStringLiteral("initial");
inline const QString kInitialDelta = QStringLiteral("initial_delta");
inline const QString kInitialTotal = QStringLiteral("initial_total");

inline const QString kFinal = QStringLiteral("final");
inline const QString kFinalTotal = QStringLiteral("final_total");

inline const QString kPartnerId = QStringLiteral("partner_id");
inline const QString kEmployeeId = QStringLiteral("employee_id");

inline const QString kDiscount = QStringLiteral("discount");
inline const QString kUnitDiscount = QStringLiteral("unit_discount");
inline const QString kDiscountTotal = QStringLiteral("discount_total");

inline constexpr char kAction[] = "action";
inline const QString kEntryAction = QStringLiteral("entry_action");

inline const QString kIsSaved = QStringLiteral("is_saved");

inline const QString kIssuedTime = QStringLiteral("issued_time");
inline const QString kStatus = QStringLiteral("status");
inline const QString kNodeStatus = QStringLiteral("node_status");
inline const QString kKind = QStringLiteral("kind");
inline const QString kNote = QStringLiteral("note");
inline const QString kPaymentTerm = QStringLiteral("payment_term");
inline const QString kRhsNode = QStringLiteral("rhs_node");
inline const QString kLhsNode = QStringLiteral("lhs_node");
inline const QString kState = QStringLiteral("state");
inline const QString kUnit = QStringLiteral("unit");
inline const QString kUnitCost = QStringLiteral("unit_cost");
inline const QString kUnitPrice = QStringLiteral("unit_price");

inline const QString kCreatedTime = QStringLiteral("created_time");
inline const QString kCreatedBy = QStringLiteral("created_by");
inline const QString kUpdatedTime = QStringLiteral("updated_time");
inline const QString kUpdatedBy = QStringLiteral("updated_by");
inline const QString kAncestor = QStringLiteral("ancestor");
inline const QString kDescendant = QStringLiteral("descendant");

// Others
inline const QString kYTX = QStringLiteral("ytx");
inline const QString kUi = QStringLiteral("ui");
inline const QString kPrint = QStringLiteral("print");
inline const QString kExport = QStringLiteral("export");
inline const QString kField = QStringLiteral("field");

// Constants for sections
inline const QString kFinance = QStringLiteral("finance");
inline const QString kFinanceNode = QStringLiteral("finance_node");
inline const QString kFinancePath = QStringLiteral("finance_path");
inline const QString kFinanceEntry = QStringLiteral("finance_entry");

inline const QString kInventory = QStringLiteral("inventory");
inline const QString kInventoryNode = QStringLiteral("inventory_node");
inline const QString kInventoryPath = QStringLiteral("inventory_path");
inline const QString kInventoryEntry = QStringLiteral("inventory_entry");

inline const QString kPurchase = QStringLiteral("purchase");
inline const QString kPurchaseNode = QStringLiteral("purchase_node");
inline const QString kPurchasePath = QStringLiteral("purchase_path");
inline const QString kPurchaseSettlement = QStringLiteral("purchase_settlement");
inline const QString kPurchaseEntry = QStringLiteral("purchase_entry");

inline const QString kSale = QStringLiteral("sale");
inline const QString kSaleNode = QStringLiteral("sale_node");
inline const QString kSalePath = QStringLiteral("sale_path");
inline const QString kSaleSettlement = QStringLiteral("sale_settlement");
inline const QString kSaleEntry = QStringLiteral("sale_entry");

inline const QString kPartner = QStringLiteral("partner");
inline const QString kPartnerNode = QStringLiteral("partner_node");
inline const QString kPartnerPath = QStringLiteral("partner_path");
inline const QString kPartnerEntry = QStringLiteral("partner_entry");

inline const QString kTask = QStringLiteral("task");
inline const QString kTaskNode = QStringLiteral("task_node");
inline const QString kTaskPath = QStringLiteral("task_path");
inline const QString kTaskEntry = QStringLiteral("task_entry");

#endif // CONSTVALUE_H
