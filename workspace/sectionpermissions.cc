#include "sectionpermissions.h"

#include <QObject>

namespace section {

std::span<const PermissionItem> PermissionItems(Section section)
{
    static const PermissionItem basic[] = {
        { 0b01, QObject::tr("Read Only") },
        { 0b11, QObject::tr("Read Write") },
        { 1 << 2, QObject::tr("Lock") },
        { 1 << 3, QObject::tr("Unlock") },
    };

    static const PermissionItem order[] = {
        { 0b01, QObject::tr("Read Only") },
        { 0b11, QObject::tr("Read Write") },
        { 1 << 2, QObject::tr("Release") },
        { 1 << 3, QObject::tr("Unrelease") },
        { 1 << 4, QObject::tr("Settle") },
        { 1 << 5, QObject::tr("Unsettle") },
    };

    switch (section) {
    case Section::kFinance:
    case Section::kTask:
    case Section::kInventory:
    case Section::kPartner:
        return basic;

    case Section::kSale:
    case Section::kPurchase:
        return order;
    }

    std::unreachable();
}

QString PermissionsDisplay(Section section, int permissions)
{
    if (permissions == 0) {
        return {};
    }

    QStringList result {};

    for (const auto& item : PermissionItems(section)) {
        if ((permissions & item.permission) != item.permission) {
            continue;
        }

        // Skip permissions covered by higher-level permissions.
        if (item.permission == 0b01 && (permissions & 0b11) == 0b11) {
            continue;
        }

        result.emplaceBack(item.text);
    }

    return result.join(QStringLiteral(" | "));
}

}
