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

        // Asset operation carriers
        { kInventory, QObject::tr("Inventory") },
        { kFixedAsset, QObject::tr("Fixed Asset") },
        { kIntangibleAsset, QObject::tr("Intangible Asset") },

        // Deferral / accrual carriers
        { kPrepaidExpense, QObject::tr("Prepaid Expense") },
        { kAccruedLiability, QObject::tr("Accrued Liability") },
        { kDeferredRevenue, QObject::tr("Deferred Revenue") },

        // Tax carrier
        { kTax, QObject::tr("Tax") },

        // Financing carriers
        { kDebt, QObject::tr("Debt") },
        { kEquity, QObject::tr("Equity") },
        { kRetainedEarnings, QObject::tr("Retained Earnings") },
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

}
