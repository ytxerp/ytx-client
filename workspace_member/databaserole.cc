#include "databaserole.h"

#include <QtCore/qobject.h>

namespace database {

std::span<const RoleItem> RoleItemList()
{
    static const RoleItem list[] = {
        { Role::kFinanceReadonly, QObject::tr("Finance R") },
        { Role::kFinanceReadwrite, QObject::tr("Finance W") },
        { Role::kTaskReadonly, QObject::tr("Task R") },
        { Role::kTaskReadwrite, QObject::tr("Task W") },
        { Role::kInventoryReadonly, QObject::tr("Inventory R") },
        { Role::kInventoryReadwrite, QObject::tr("Inventory W") },
        { Role::kPartnerReadonly, QObject::tr("Partner R") },
        { Role::kPartnerReadwrite, QObject::tr("Partner W") },
        { Role::kSaleReadonly, QObject::tr("Sale R") },
        { Role::kSaleReadwrite, QObject::tr("Sale W") },
        { Role::kPurchaseReadonly, QObject::tr("Purchase R") },
        { Role::kPurchaseReadwrite, QObject::tr("Purchase W") },
    };

    return list;
}

QString RolesDisplay(Roles roles)
{
    if (roles == 0) {
        return QString();
    }

    QStringList result {};

    auto AppendRole = [&](Role read_bit, Role write_bit, const QString& read_text, const QString& write_text) {
        if ((roles & write_bit) == write_bit) {
            result.emplaceBack(write_text);
        } else if ((roles & read_bit) == read_bit) {
            result.emplaceBack(read_text);
        }
    };

    AppendRole(Role::kFinanceReadonly, Role::kFinanceReadwrite, QObject::tr("Finance R"), QObject::tr("Finance W"));
    AppendRole(Role::kTaskReadonly, Role::kTaskReadwrite, QObject::tr("Task R"), QObject::tr("Task W"));
    AppendRole(Role::kInventoryReadonly, Role::kInventoryReadwrite, QObject::tr("Inventory R"), QObject::tr("Inventory W"));
    AppendRole(Role::kPartnerReadonly, Role::kPartnerReadwrite, QObject::tr("Partner R"), QObject::tr("Partner W"));
    AppendRole(Role::kSaleReadonly, Role::kSaleReadwrite, QObject::tr("Sale R"), QObject::tr("Sale W"));
    AppendRole(Role::kPurchaseReadonly, Role::kPurchaseReadwrite, QObject::tr("Purchase R"), QObject::tr("Purchase W"));

    return result.join(" | ");
}
}
