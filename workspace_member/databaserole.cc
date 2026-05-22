#include "databaserole.h"

#include <QtCore/qobject.h>

#include "component/constantstring.h"

namespace database {

std::span<const RoleItem> RoleItemList()
{
    static const RoleItem list[] = {
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

QString RolesDisplay(Roles roles)
{
    if (roles == 0) {
        return string_const::kEmpty;
    }

    QStringList result {};

    auto AppendRole = [&](Role read_bit, Role write_bit, const QString& read_text, const QString& write_text) {
        if ((roles & write_bit) == write_bit) {
            result.emplaceBack(write_text);
        } else if ((roles & read_bit) == read_bit) {
            result.emplaceBack(read_text);
        }
    };

    AppendRole(FINANCE_READONLY, FINANCE_READWRITE, QObject::tr("Finance R"), QObject::tr("Finance W"));
    AppendRole(TASK_READONLY, TASK_READWRITE, QObject::tr("Task R"), QObject::tr("Task W"));
    AppendRole(INVENTORY_READONLY, INVENTORY_READWRITE, QObject::tr("Inventory R"), QObject::tr("Inventory W"));
    AppendRole(PARTNER_READONLY, PARTNER_READWRITE, QObject::tr("Partner R"), QObject::tr("Partner W"));
    AppendRole(SALE_READONLY, SALE_READWRITE, QObject::tr("Sale R"), QObject::tr("Sale W"));
    AppendRole(PURCHASE_READONLY, PURCHASE_READWRITE, QObject::tr("Purchase R"), QObject::tr("Purchase W"));

    return result.join(" | ");
}
}
