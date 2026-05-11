#include "databaserole.h"

#include <QtCore/qobject.h>

namespace database_role {

std::span<const PermissionItem> RoleList()
{
    static const PermissionItem list[] = {
        { FINANCE_READONLY, QObject::tr("Finance R") },
        { FINANCE_READWRITE, QObject::tr("Finance W") },
        { TASK_READONLY, QObject::tr("Task R") },
        { TASK_READWRITE, QObject::tr("Task W") },
        { INVENTORY_READONLY, QObject::tr("Inventory R") },
        { INVENTORY_READWRITE, QObject::tr("Inventory W") },
        { PARTNER_READONLY, QObject::tr("Partner R") },
        { PARTNER_READWRITE, QObject::tr("Partner W") },
        { SALE_READONLY, QObject::tr("Sale R") },
        { SALE_READWRITE, QObject::tr("Sale W") },
        { PURCHASE_READONLY, QObject::tr("Purchase R") },
        { PURCHASE_READWRITE, QObject::tr("Purchase W") },
    };

    return list;
}

QString RoleDisplay(PermissionBits bits)
{
    QStringList result {};

    for (const auto& item : RoleList()) {
        if ((bits & item.bit) == item.bit) {
            result.emplaceBack(item.text);
        }
    }

    return result.join(" | ");
}
}
