#include "finance_role.h"

#include <QObject>

#include "component/constantstring.h"

namespace finance {

std::span<const RoleItem> RoleList()
{
    static const RoleItem list[] = {
        // Asset accounts
        { kCash, QObject::tr("Cash") },
        { kBank, QObject::tr("Bank") },
        { kWallet, QObject::tr("Wallet") },
        { kReceivable, QObject::tr("Receivable") },
        { kInventory, QObject::tr("Inventory") },
        { kFixedAsset, QObject::tr("Fixed Asset") },

        // Liability accounts
        { kPayable, QObject::tr("Payable") },
        { kTax, QObject::tr("Tax") },

        // Equity
        { kEquity, QObject::tr("Equity") },

        // Profit & Loss
        { kRevenue, QObject::tr("Revenue") },
        { kExpense, QObject::tr("Expense") },
    };

    return list;
}

QString RoleDisplay(Roles roles)
{
    if (roles == 0) {
        return string_const::kEmpty;
    }

    QStringList result {};

    for (const auto& item : RoleList()) {
        if ((roles & item.role) == item.role) {
            result.append(item.text);
        }
    }

    return result.join(" | ");
}

}
