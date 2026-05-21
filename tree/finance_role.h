#ifndef FINANCE_ROLE_H
#define FINANCE_ROLE_H

#include <QFlags>
#include <QString>
#include <span>

namespace finance {

enum class CashKind {
    kNone = 0,
    kOperatingIn = 1,
    kOperatingOut = 2,
    kInvestingIn = 3,
    kInvestingOut = 4,
    kFinancingIn = 5,
    kFinancingOut = 6,
};

// Describes the accounting role of an account.
// Multiple roles can be combined through QFlags.
enum Role {
    // Asset accounts
    kCash = 0b1 << 0,
    kBank = 0b1 << 1,
    kWallet = 0b1 << 2,
    kReceivable = 0b1 << 3,
    kInventory = 0b1 << 4,
    kFixedAsset = 0b1 << 5,

    // Liability accounts
    kPayable = 0b1 << 6,
    kTax = 0b1 << 7,

    // Equity
    kEquity = 0b1 << 8,

    // Profit & loss
    kRevenue = 0b1 << 9,
    kExpense = 0b1 << 10,
};

Q_DECLARE_FLAGS(Roles, Role)
Q_DECLARE_OPERATORS_FOR_FLAGS(Roles)

struct RoleItem {
    Role role {};
    QString text {};
};

std::span<const RoleItem> RoleList();
QString RoleDisplay(Roles roles);

}

#endif // FINANCE_ROLE_H
