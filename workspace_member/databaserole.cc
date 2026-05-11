#include "databaserole.h"

#include <QtCore/qobject.h>

namespace PermissionBits {

std::span<const PermissionItem> DatabaseRoleList()
{
    static const PermissionItem list[] = {
        { PermissionBits::FINANCE_READONLY, QObject::tr("Finance R") },
        { PermissionBits::FINANCE_READWRITE, QObject::tr("Finance W") },
        { PermissionBits::TASK_READONLY, QObject::tr("Task R") },
        { PermissionBits::TASK_READWRITE, QObject::tr("Task W") },
        { PermissionBits::INVENTORY_READONLY, QObject::tr("Inventory R") },
        { PermissionBits::INVENTORY_READWRITE, QObject::tr("Inventory W") },
        { PermissionBits::PARTNER_READONLY, QObject::tr("Partner R") },
        { PermissionBits::PARTNER_READWRITE, QObject::tr("Partner W") },
        { PermissionBits::SALE_READONLY, QObject::tr("Sale R") },
        { PermissionBits::SALE_READWRITE, QObject::tr("Sale W") },
        { PermissionBits::PURCHASE_READONLY, QObject::tr("Purchase R") },
        { PermissionBits::PURCHASE_READWRITE, QObject::tr("Purchase W") },
    };

    return list;
}

QString DatabaseRoleToDisplay(PermissionBits::Flags flags)
{
    QString result {};

    for (const auto& item : DatabaseRoleList()) {
        if ((flags & item.flag) == item.flag) {
            if (!result.isEmpty())
                result += " | ";
            result += item.text;
        }
    }

    return result;
}
}
