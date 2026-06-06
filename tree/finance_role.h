#ifndef FINANCE_ROLE_H
#define FINANCE_ROLE_H

#include <QFlags>
#include <QString>
#include <span>

namespace finance {

enum class CashKind {
    kNone = 0,

    // =====================================================
    // Operating Activities - Inflow  [100-199]
    // =====================================================
    kSalesReceipt = 100, // Cash received from sales of goods and rendering of services
    kTaxRefund = 101, // Tax refunds received
    kOtherOperatingReceipt = 102, // Other cash received relating to operating activities

    // =====================================================
    // Operating Activities - Outflow  [200-299]
    // =====================================================
    kPurchasePayment = 200, // Cash paid for goods and services received
    kSalaryPayment = 201, // Cash paid to and on behalf of employees
    kTaxPayment = 202, // Cash paid for all types of taxes
    kOtherOperatingPayment = 203, // Other cash paid relating to operating activities

    // =====================================================
    // Investing Activities - Inflow  [300-399]
    // =====================================================
    kInvestmentReceipt = 300, // Cash received from recovery of investments
    kInvestmentIncomeReceipt = 301, // Cash received from investment income
    kAssetDisposalReceipt = 302, // Net cash received from disposal of fixed assets, intangibles and other long-term assets
    kSubsidiaryDisposalReceipt = 303, // Net cash received from disposal of subsidiaries and other business units
    kLoanRepaymentReceipt = 304, // Cash received from repayment of loans extended
    kOtherInvestingReceipt = 305, // Other cash received relating to investing activities

    // =====================================================
    // Investing Activities - Outflow  [400-499]
    // =====================================================
    kAssetPurchasePayment = 400, // Cash paid for acquisition of fixed assets, intangibles and other long-term assets
    kInvestmentPayment = 401, // Cash paid for investments
    kSubsidiaryAcquisitionPayment = 402, // Net cash paid for acquisition of subsidiaries and other business units
    kLoanPayment = 403, // Cash paid for loans extended to other parties
    kOtherInvestingPayment = 404, // Other cash paid relating to investing activities

    // =====================================================
    // Financing Activities - Inflow  [500-599]
    // =====================================================
    kCapitalContributionReceipt = 500, // Cash received from capital contributions
    kBorrowingReceipt = 501, // Cash received from borrowings
    kOtherFinancingReceipt = 502, // Other cash received relating to financing activities

    // =====================================================
    // Financing Activities - Outflow  [600-699]
    // =====================================================
    kDebtRepaymentPayment = 600, // Cash repayments of amounts borrowed
    kDividendPayment = 601, // Cash paid for dividends and profit distributions
    kInterestPayment = 602, // Cash paid for interest
    kOtherFinancingPayment = 603, // Other cash paid relating to financing activities

    // =====================================================
    // Non-Cash / Special  [900-999]
    // =====================================================
    kExchangeRateEffect = 900, // Effect of foreign exchange rate changes on cash
    kAdjustment = 901, // System adjustment or reversal entry
    kUnclassified = 999, // Unclassified, used to detect missing configuration
};

QString CashKindName(finance::CashKind kind);

// Describes the business semantic role of a node.
// Multiple roles can be combined via bitwise OR.
//
// NOTE:
// - Financial statement structure is managed by the node tree.
// - Nodes with Fund carrier roles (Cash/Bank/Wallet) are used as
//   carriers in cash flow entries.
enum Role {
    kNone = 0,

    // Fund carriers
    kCash = 0b1 << 0, // Cash on hand
    kBank = 0b1 << 1, // Bank deposit
    kWallet = 0b1 << 2, // Digital wallet

    // Settlement carriers
    kReceivable = 0b1 << 3, // Accounts receivable
    kPayable = 0b1 << 4, // Accounts payable
    kPrepayment = 0b1 << 5, // Prepayment to suppliers
    kAdvanceReceipt = 0b1 << 6, // Advance receipt from customers

    // Asset carriers
    kInventory = 0b1 << 7, // Inventory
    kFixedAsset = 0b1 << 8, // Fixed assets
    kIntangibleAsset = 0b1 << 9, // Intangible assets
    kLongTermInvestment = 0b1 << 10, // Long-term investment

    // Deferral / accrual carriers
    kPrepaidExpense = 0b1 << 11, // Prepaid expense
    kAccruedLiability = 0b1 << 12, // Accrued liability
    kDeferredRevenue = 0b1 << 13, // Deferred revenue

    // Tax carrier
    kTax = 0b1 << 14, // Tax payable / receivable

    // Financing carriers
    kDebt = 0b1 << 15, // Loans and borrowings

    // Equity carriers
    kEquity = 0b1 << 16, // Paid-in capital
    kRetainedEarning = 0b1 << 17, // Retained earnings

    // Income / Expense carriers
    kIncome = 0b1 << 18, // Revenue and income
    kExpense = 0b1 << 19, // Cost and expense
};

Q_DECLARE_FLAGS(Roles, Role)
Q_DECLARE_OPERATORS_FOR_FLAGS(Roles)

struct RoleItem {
    Role role {};
    QString text {};
};

std::span<const RoleItem> RoleItemList();
QString RolesDisplay(Roles roles);

}

#endif // FINANCE_ROLE_H
