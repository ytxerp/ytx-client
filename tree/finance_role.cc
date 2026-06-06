#include "finance_role.h"

#include <QObject>

namespace finance {

std::span<const RoleItem> RoleItemList()
{
    static const RoleItem list[] = {
        // Fund carriers
        { kCash, QObject::tr("Cash") },
        { kBank, QObject::tr("Bank") },
        { kWallet, QObject::tr("Wallet") },
        // Settlement carriers
        { kReceivable, QObject::tr("Receivable") },
        { kPayable, QObject::tr("Payable") },
        { kPrepayment, QObject::tr("Prepayment") },
        { kAdvanceReceipt, QObject::tr("Advance Receipt") },
        // Asset carriers
        { kInventory, QObject::tr("Inventory") },
        { kFixedAsset, QObject::tr("Fixed Asset") },
        { kIntangibleAsset, QObject::tr("Intangible Asset") },
        { kLongTermInvestment, QObject::tr("Long Term Investment") },
        // Deferral / accrual carriers
        { kPrepaidExpense, QObject::tr("Prepaid Expense") },
        { kAccruedLiability, QObject::tr("Accrued Liability") },
        { kDeferredRevenue, QObject::tr("Deferred Revenue") },
        // Tax carrier
        { kTax, QObject::tr("Tax") },
        // Financing carriers
        { kDebt, QObject::tr("Debt") },
        // Equity carriers
        { kEquity, QObject::tr("Equity") },
        { kRetainedEarning, QObject::tr("Retained Earning") },
        // Income / Expense carriers
        { kIncome, QObject::tr("Income") },
        { kExpense, QObject::tr("Expense") },
    };
    return list;
}

QString RolesDisplay(Roles roles)
{
    if (roles == 0) {
        return QString();
    }

    QStringList result {};

    for (const auto& item : RoleItemList()) {
        if ((roles & item.role) == item.role) {
            result.append(item.text);
        }
    }

    return result.join(" | ");
}

QString CashKindName(CashKind kind)
{
    switch (kind) {
    // =====================================================
    // Operating Activities - Inflow
    // =====================================================
    case finance::CashKind::kSalesReceipt:
        return QObject::tr("Sales Receipt");
    case finance::CashKind::kTaxRefund:
        return QObject::tr("Tax Refund");
    case finance::CashKind::kOtherOperatingReceipt:
        return QObject::tr("Other Operating Receipt");

        // =====================================================
        // Operating Activities - Outflow
        // =====================================================
    case finance::CashKind::kPurchasePayment:
        return QObject::tr("Purchase Payment");
    case finance::CashKind::kSalaryPayment:
        return QObject::tr("Salary Payment");
    case finance::CashKind::kTaxPayment:
        return QObject::tr("Tax Payment");
    case finance::CashKind::kOtherOperatingPayment:
        return QObject::tr("Other Operating Payment");

        // =====================================================
        // Investing Activities - Inflow
        // =====================================================
    case finance::CashKind::kInvestmentReceipt:
        return QObject::tr("Investment Receipt");
    case finance::CashKind::kInvestmentIncomeReceipt:
        return QObject::tr("Investment Income Receipt");
    case finance::CashKind::kAssetDisposalReceipt:
        return QObject::tr("Asset Disposal Receipt");
    case finance::CashKind::kSubsidiaryDisposalReceipt:
        return QObject::tr("Subsidiary Disposal Receipt");
    case finance::CashKind::kLoanRepaymentReceipt:
        return QObject::tr("Loan Repayment Receipt");
    case finance::CashKind::kOtherInvestingReceipt:
        return QObject::tr("Other Investing Receipt");

        // =====================================================
        // Investing Activities - Outflow
        // =====================================================
    case finance::CashKind::kAssetPurchasePayment:
        return QObject::tr("Asset Purchase Payment");
    case finance::CashKind::kInvestmentPayment:
        return QObject::tr("Investment Payment");
    case finance::CashKind::kSubsidiaryAcquisitionPayment:
        return QObject::tr("Subsidiary Acquisition Payment");
    case finance::CashKind::kLoanPayment:
        return QObject::tr("Loan Payment");
    case finance::CashKind::kOtherInvestingPayment:
        return QObject::tr("Other Investing Payment");

        // =====================================================
        // Financing Activities - Inflow
        // =====================================================
    case finance::CashKind::kCapitalContributionReceipt:
        return QObject::tr("Capital Contribution Receipt");
    case finance::CashKind::kBorrowingReceipt:
        return QObject::tr("Borrowing Receipt");
    case finance::CashKind::kOtherFinancingReceipt:
        return QObject::tr("Other Financing Receipt");

        // =====================================================
        // Financing Activities - Outflow
        // =====================================================
    case finance::CashKind::kDebtRepaymentPayment:
        return QObject::tr("Debt Repayment Payment");
    case finance::CashKind::kDividendPayment:
        return QObject::tr("Dividend Payment");
    case finance::CashKind::kInterestPayment:
        return QObject::tr("Interest Payment");
    case finance::CashKind::kOtherFinancingPayment:
        return QObject::tr("Other Financing Payment");

        // =====================================================
        // Non-Cash / Special
        // =====================================================
    case finance::CashKind::kInternalTransfer:
        return QObject::tr("Internal Transfer");
    case finance::CashKind::kExchangeRateEffect:
        return QObject::tr("Exchange Rate Effect");
    case finance::CashKind::kAdjustment:
        return QObject::tr("Adjustment");
    case finance::CashKind::kUnclassified:
        return QObject::tr("Unclassified");

    case finance::CashKind::kNone:
    default:
        return QString();
    }
}

}
