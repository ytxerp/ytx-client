#include "databaserole.h"

#include <QtCore/qobject.h>

namespace database_role {

std::span<const PermissionItem> RoleList()
{
    static const PermissionItem list[] = {
        { database_role::FINANCE_READONLY, QObject::tr("Finance R") },
        { database_role::FINANCE_READWRITE, QObject::tr("Finance W") },
        { database_role::TASK_READONLY, QObject::tr("Task R") },
        { database_role::TASK_READWRITE, QObject::tr("Task W") },
        { database_role::INVENTORY_READONLY, QObject::tr("Inventory R") },
        { database_role::INVENTORY_READWRITE, QObject::tr("Inventory W") },
        { database_role::PARTNER_READONLY, QObject::tr("Partner R") },
        { database_role::PARTNER_READWRITE, QObject::tr("Partner W") },
        { database_role::SALE_READONLY, QObject::tr("Sale R") },
        { database_role::SALE_READWRITE, QObject::tr("Sale W") },
        { database_role::PURCHASE_READONLY, QObject::tr("Purchase R") },
        { database_role::PURCHASE_READWRITE, QObject::tr("Purchase W") },
    };

    return list;
}

QString RoleDisplay(database_role::PermissionBits bits)
{
    QString result {};

    for (const auto& item : RoleList()) {
        if ((bits & item.bit) == item.bit) {
            if (!result.isEmpty())
                result += " | ";
            result += item.text;
        }
    }

    return result;
}
}
