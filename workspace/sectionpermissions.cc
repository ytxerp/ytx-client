#include "sectionpermissions.h"

#include <QtCore/qobject.h>

namespace section {

std::span<const PermissionItem> PermissionItems()
{
    static const PermissionItem list[] = {
        { Permission::kFinanceReadOnly, QObject::tr("Finance R") },
        { Permission::kFinanceReadWrite, QObject::tr("Finance W") },
        { Permission::kTaskReadOnly, QObject::tr("Task R") },
        { Permission::kTaskReadWrite, QObject::tr("Task W") },
        { Permission::kInventoryReadOnly, QObject::tr("Inventory R") },
        { Permission::kInventoryReadWrite, QObject::tr("Inventory W") },
        { Permission::kPartnerReadOnly, QObject::tr("Partner R") },
        { Permission::kPartnerReadWrite, QObject::tr("Partner W") },
        { Permission::kSaleReadOnly, QObject::tr("Sale R") },
        { Permission::kSaleReadWrite, QObject::tr("Sale W") },
        { Permission::kPurchaseReadOnly, QObject::tr("Purchase R") },
        { Permission::kPurchaseReadWrite, QObject::tr("Purchase W") },
    };

    return list;
}

QString PermissionsDisplay(Permissions permissions)
{
    if (permissions == 0) {
        return QString();
    }

    QStringList result {};

    auto AppendPermission = [&](Permission read_bit, Permission write_bit, const QString& read_text, const QString& write_text) {
        if ((permissions & write_bit) == write_bit) {
            result.emplaceBack(write_text);
        } else if ((permissions & read_bit) == read_bit) {
            result.emplaceBack(read_text);
        }
    };

    AppendPermission(Permission::kFinanceReadOnly, Permission::kFinanceReadWrite, QObject::tr("Finance R"), QObject::tr("Finance W"));
    AppendPermission(Permission::kTaskReadOnly, Permission::kTaskReadWrite, QObject::tr("Task R"), QObject::tr("Task W"));
    AppendPermission(Permission::kInventoryReadOnly, Permission::kInventoryReadWrite, QObject::tr("Inventory R"), QObject::tr("Inventory W"));
    AppendPermission(Permission::kPartnerReadOnly, Permission::kPartnerReadWrite, QObject::tr("Partner R"), QObject::tr("Partner W"));
    AppendPermission(Permission::kSaleReadOnly, Permission::kSaleReadWrite, QObject::tr("Sale R"), QObject::tr("Sale W"));
    AppendPermission(Permission::kPurchaseReadOnly, Permission::kPurchaseReadWrite, QObject::tr("Purchase R"), QObject::tr("Purchase W"));

    return result.join(" | ");
}
}
