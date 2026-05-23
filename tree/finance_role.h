#ifndef FINANCE_ROLE_H
#define FINANCE_ROLE_H

#include <QFlags>
#include <QString>
#include <span>

namespace finance {

// Describes the cash flow category of a cash flow statement node.
// This is used to mark report/tree nodes, not bank/cash accounts.
// All transactions under a node inherit the node's CashKind.
enum class CashKind {
    kNone = 0,

    // Operating activities
    kOperatingIn = 1,
    kOperatingOut = 2,

    // Investing activities
    kInvestingIn = 3,
    kInvestingOut = 4,

    // Financing activities
    kFinancingIn = 5,
    kFinancingOut = 6,
};

// Describes the business semantic role of an account.
// Multiple roles can be combined through QFlags.
//
// NOTE:
// - Financial statement structure is managed by the account tree.
// - Cash flow behavior is managed separately by CashKind.
// - Role only describes business capabilities / semantic traits.
enum Role {
    // Fund carriers
    kCash = 0b1 << 0,
    kBank = 0b1 << 1,
    kWallet = 0b1 << 2,

    // Settlement carriers
    kReceivable = 0b1 << 3,
    kPayable = 0b1 << 4,

    // Asset operation carriers
    kInventory = 0b1 << 5,
    kFixedAsset = 0b1 << 6,
    kIntangibleAsset = 0b1 << 7,

    // Deferral / accrual carriers
    kPrepaidExpense = 0b1 << 8,
    kAccruedLiability = 0b1 << 9,
    kDeferredRevenue = 0b1 << 10,

    // Tax carrier
    kTax = 0b1 << 11,

    // Financing carriers
    kDebt = 0b1 << 12,
    kEquity = 0b1 << 13,
    kRetainedEarnings = 0b1 << 14,
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
