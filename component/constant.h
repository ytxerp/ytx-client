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
inline constexpr int kCoefficient4 = 4;
inline constexpr int kCoefficient5 = 5;
inline constexpr int kCoefficient8 = 8;
inline constexpr int kCoefficient16 = 16;
inline constexpr double kTolerance = 1e-8;

inline constexpr int kHundred = 100;
inline constexpr int kRowHeight = 24;
inline constexpr int kThreeThousand = 3000;
inline constexpr long long kBatchSize = 50;

namespace Pool {
inline constexpr qsizetype kExpandSize { 100 };
inline constexpr qsizetype kMaxSize { 1000 };
}

// Boolean constants
inline constexpr bool kIsHidden = true;

namespace Rule {
// Finance/Item/Task: Credit increase, Debit decrease; calculation: credit - debit
// Order: Refund Order (returned transaction)
inline constexpr bool kDDCI = true;
inline constexpr bool kRO = true;

// Finance/Item/Task: Debit increase, Credit decrease; calculation: debit - credit
// Order: Trade Order (normal transaction)
inline constexpr bool kDICD = false;
inline constexpr bool kTO = false;

inline const QString kStrDDCI = QStringLiteral("DDCI");
inline const QString kStrDICD = QStringLiteral("DICD");

inline const QString kStrTO = QStringLiteral("TO");
inline const QString kStrRO = QStringLiteral("RO");
}

// Maximum and minimum values for PostgreSQL NUMERIC(12,4)
// Up to 8 digits before the decimal point and 4 digits after
inline constexpr double kMaxNumeric_12_4 = 99999999.9999;
inline constexpr double kMinNumeric_12_4 = -99999999.9999;

// Maximum and minimum values for PostgreSQL NUMERIC(16,8)
// Up to 8 digits before the decimal point and 8 digits after
// Used only in the finance module for exchange rates
inline constexpr double kMaxNumeric_16_8 = 99999999.99999999;
inline constexpr double kMinNumeric_16_8 = -99999999.99999999;

inline constexpr int kMaxNumericScale_4 = 4;
inline constexpr int kMaxNumericScale_8 = 8;

// Time constants
inline const QString kDateTimeFST = QStringLiteral("yyyy-MM-dd HH:mm");
inline const QString kDateFST = QStringLiteral("yyyy-MM-dd");
inline const QString kMonthFST = QStringLiteral("yyyyMM");

inline const QTime kEndTime = { 23, 59 }; // 23:59
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
inline constexpr char kCheck[] = "check";

// Operator symbols
inline const QString kMinux = QStringLiteral("-");
inline const QString kPlus = QStringLiteral("+");

// File suffixes
inline const QString kDotSuffixCache = QStringLiteral(".cache");
inline const QString kDotSuffixINI = QStringLiteral(".ini");
inline const QString kDotSuffixLOCK = QStringLiteral(".lock");
inline const QString kDotSuffixXLSX = QStringLiteral(".xlsx");
inline const QString kSuffixPERCENT = QStringLiteral("%");
inline const QString kSuffixXLSX = QStringLiteral("xlsx");

// Node rule, kind, unit

inline const QString kBranchKind = QStringLiteral("B");
inline const QString kLeafKind = QStringLiteral("L");

inline const QString kUnitInternal = QStringLiteral("INT");
inline const QString kUnitPosition = QStringLiteral("POS");
inline const QString kUnitExternal = QStringLiteral("EXT");
inline const QString kUnitCustomer = QStringLiteral("CUST");
inline const QString kUnitEmployee = QStringLiteral("EMP");
inline const QString kUnitVendor = QStringLiteral("VEND");
inline const QString kUnitImmediate = QStringLiteral("IS");
inline const QString kUnitMonthly = QStringLiteral("MS");
inline const QString kUnitPending = QStringLiteral("PEND");

// App ini keys
inline const QString kGeometry = QStringLiteral("geometry");
inline const QString kHeaderState = QStringLiteral("header_state");
inline const QString kMainwindow = QStringLiteral("mainwindow");
inline const QString kPrinter = QStringLiteral("printer");
inline const QString kSection = QStringLiteral("section");
inline const QString kIsParallel = QStringLiteral("is_parallel");
inline const QString kSplitter = QStringLiteral("splitter");
inline const QString kStart = QStringLiteral("start");
inline const QString kEnd = QStringLiteral("end");
inline const QString kStartSection = QStringLiteral("start/section");
inline const QString kTheme = QStringLiteral("theme");

// EntryData → represents multiple entries or the entire table's data set, suitable for server bulk responses
// NodeData → represents multiple nodes or the entire node table's data set
inline const QString kTreeAcked = QStringLiteral("tree_acked");
inline const QString kLeafAcked = QStringLiteral("leaf_acked");
inline const QString kOneNode = QStringLiteral("one_node");

// Server-initiated data push
inline const QString kTreeApplied = QStringLiteral("tree_applied");
inline const QString kGlobalConfig = QStringLiteral("global_config");

// Decimal config
inline const QString kAmountDecimal = QStringLiteral("amount_decimal");
inline const QString kRateDecimal = QStringLiteral("rate_decimal");

// Other ini keys
inline const QString kDateFormat = QStringLiteral("date_format");
inline const QString kDynamicLabel = QStringLiteral("dynamic_label");
inline const QString kDynamicNodeLhs = QStringLiteral("dynamic_node_lhs");
inline const QString kDynamicNodeRhs = QStringLiteral("dynamic_node_rhs");
inline const QString kOperation = QStringLiteral("operation");
inline const QString kStaticLabel = QStringLiteral("static_label");
inline const QString kStaticNode = QStringLiteral("static_node");

inline const QString kLeafReference = QStringLiteral("leaf_reference");
inline const QString kLeafRemove = QStringLiteral("leaf_remove");
inline const QString kLeafReplace = QStringLiteral("leaf_replace");
inline const QString kLeafEntry = QStringLiteral("leaf_entry");

inline const QString kExternalReference = QStringLiteral("external_reference");
inline const QString kInternalReference = QStringLiteral("internal_reference");
inline const QString kExternalItem = QStringLiteral("external_item");

inline const QString kNodeInsert = QStringLiteral("node_insert");
inline const QString kNodeDrag = QStringLiteral("node_drag");
inline const QString kNodeUpdate = QStringLiteral("node_update");

// App language
inline const QString kEnUS = QStringLiteral("en_US");
inline const QString kLanguage = QStringLiteral("language");
inline const QString kZhCN = QStringLiteral("zh_CN");
inline const QString kSolarizedDark = QStringLiteral("Solarized Dark");

// Database & connection
inline const QString kLogin = QStringLiteral("login");
inline const QString kEmail = QStringLiteral("email");
inline const QString kPassword = QStringLiteral("password");
inline const QString kKeyword = QStringLiteral("keyword");
inline const QString kRegister = QStringLiteral("register");
inline const QString kRegisterResult = QStringLiteral("register_result");
inline const QString kWorkspaceAccessPending = QStringLiteral("workspace_access_pending");
inline const QString kServer = QStringLiteral("server");
inline const QString kLoginFailed = QStringLiteral("login_failed");
inline const QString kLoginSuccess = QStringLiteral("login_success");
inline const QString kWorkspace = QStringLiteral("workspace");
inline const QString kHost = QStringLiteral("host");
inline const QString kBranchRemove = QStringLiteral("branch_remove");
inline const QString kNode = QStringLiteral("node");
inline const QString kPath = QStringLiteral("path");
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
inline const QString kEntryRhsNode = QStringLiteral("entry_rhs_node");
inline const QString kEntryRate = QStringLiteral("entry_rate");
inline const QString kUpdateEntryDebit = QStringLiteral("update_entry_debit");
inline const QString kUpdateEntryCredit = QStringLiteral("update_entry_credit");
inline const QString kEntryNumeric = QStringLiteral("entry_numeric");

inline const QString kStatus = QStringLiteral("status");
inline const QString kNodeDelta = QStringLiteral("node_delta");
inline const QString kDocumentDir = QStringLiteral("document_dir");
inline const QString kDefaultUnit = QStringLiteral("default_unit");
inline const QString kUpdateDefaultUnitFailed = QStringLiteral("update_default_unit_failed");
inline const QString kUnreferencedNodeRemove = QStringLiteral("unreferenced_node_remove");

// Message fields
inline const QString kValue = QStringLiteral("value");
inline const QString kMsgType = QStringLiteral("msg_type");

// Tree & table columns
inline const QString kCode = QStringLiteral("code");
inline const QString kColor = QStringLiteral("color");
inline const QString kCommission = QStringLiteral("commission");
inline const QString kDescription = QStringLiteral("description");
inline const QString kDirectionRule = QStringLiteral("direction_rule");
inline const QString kName = QStringLiteral("name");
inline const QString kDocument = QStringLiteral("document");
inline const QString kEmployee = QStringLiteral("employee");
inline const QString kFirst = QStringLiteral("first");
inline const QString kFirstTotal = QStringLiteral("first_total");

inline const QString kEntryId = QStringLiteral("entry_id");
inline const QString kOldRhsDelta = QStringLiteral("old_rhs_delta");
inline const QString kNewRhsDelta = QStringLiteral("new_rhs_delta");
inline const QString kOldRhsId = QStringLiteral("old_rhs_id");
inline const QString kNewRhsId = QStringLiteral("new_rhs_id");
inline const QString kId = QStringLiteral("id");
inline const QString kOldId = QStringLiteral("old_id");
inline const QString kNewId = QStringLiteral("new_id");
inline const QString kNodeId = QStringLiteral("node_id");
inline const QString kSessionId = QStringLiteral("session_id");
inline const QString kExpireTime = QStringLiteral("expire_time");
inline const QString kUserId = QStringLiteral("user_id");
inline const QString kTabId = QStringLiteral("tab_id");
inline const QString kSettlementNode = QStringLiteral("settlement_node");
inline const QString kCache = QStringLiteral("cache");
inline const QString kLhsDelta = QStringLiteral("lhs_delta");
inline const QString kRhsDelta = QStringLiteral("rhs_delta");
inline const QString kResult = QStringLiteral("result");

inline const QString kLhsRate = QStringLiteral("lhs_rate");
inline const QString kRhsRate = QStringLiteral("rhs_rate");
inline const QString kLhsDebit = QStringLiteral("lhs_debit");
inline const QString kLhsCredit = QStringLiteral("lhs_credit");
inline const QString kRhsDebit = QStringLiteral("rhs_debit");
inline const QString kRhsCredit = QStringLiteral("rhs_credit");

inline const QString kInitial = QStringLiteral("initial");
inline const QString kInitialDelta = QStringLiteral("initial_delta");
inline const QString kInitialTotal = QStringLiteral("initial_total");

inline const QString kFinal = QStringLiteral("final");
inline const QString kFinalDelta = QStringLiteral("final_delta");
inline const QString kFinalTotal = QStringLiteral("final_total");

inline const QString kDiscount = QStringLiteral("discount");
inline const QString kDiscountPrice = QStringLiteral("discount_price");
inline const QString kDiscountTotal = QStringLiteral("discount_total");

inline const QString kIsChecked = QStringLiteral("is_checked");
inline const QString kIsFinished = QStringLiteral("is_finished");
inline const QString kIsValid = QStringLiteral("is_valid");
inline const QString kIsInsert = QStringLiteral("is_insert");
inline const QString kIsSaved = QStringLiteral("is_saved");

inline const QString kIssuedTime = QStringLiteral("issued_time");
inline const QString kKind = QStringLiteral("kind");
inline const QString kNote = QStringLiteral("note");
inline const QString kParty = QStringLiteral("party");
inline const QString kPaymentTerm = QStringLiteral("payment_term");
inline const QString kRhsNode = QStringLiteral("rhs_node");
inline const QString kLhsNode = QStringLiteral("lhs_node");
inline const QString kSecond = QStringLiteral("second");
inline const QString kSecondTotal = QStringLiteral("second_total");
inline const QString kSettlement = QStringLiteral("settlement");
inline const QString kState = QStringLiteral("state");
inline const QString kUnit = QStringLiteral("unit");
inline const QString kUnitCost = QStringLiteral("unit_cost");
inline const QString kUnitPrice = QStringLiteral("unit_price");
inline const QString kVersion = QStringLiteral("version");

inline const QString kCreatedTime = QStringLiteral("created_time");
inline const QString kCreatedBy = QStringLiteral("created_by");
inline const QString kUpdatedTime = QStringLiteral("updated_time");
inline const QString kUpdatedBy = QStringLiteral("updated_by");
inline const QString kAncestor = QStringLiteral("ancestor");
inline const QString kDescendant = QStringLiteral("descendant");
inline const QString kDistance = QStringLiteral("distance");

// Others
inline const QString kYTX = QStringLiteral("ytx");
inline const QString kError = QStringLiteral("error");
inline const QString kCheckAction = QStringLiteral("check_action");
inline const QString kField = QStringLiteral("field");

// Constants for sections
inline const QString kFinance = QStringLiteral("finance");
inline const QString kFinanceNode = QStringLiteral("finance_node");
inline const QString kFinancePath = QStringLiteral("finance_path");
inline const QString kFinanceEntry = QStringLiteral("finance_entry");

inline const QString kItem = QStringLiteral("item");
inline const QString kItemNode = QStringLiteral("item_node");
inline const QString kItemPath = QStringLiteral("item_path");
inline const QString kItemEntry = QStringLiteral("item_entry");

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

inline const QString kStakeholder = QStringLiteral("stakeholder");
inline const QString kStakeholderNode = QStringLiteral("stakeholder_node");
inline const QString kStakeholderPath = QStringLiteral("stakeholder_path");
inline const QString kStakeholderEntry = QStringLiteral("stakeholder_entry");

inline const QString kTask = QStringLiteral("task");
inline const QString kTaskNode = QStringLiteral("task_node");
inline const QString kTaskPath = QStringLiteral("task_path");
inline const QString kTaskEntry = QStringLiteral("task_entry");

#endif // CONSTVALUE_H
