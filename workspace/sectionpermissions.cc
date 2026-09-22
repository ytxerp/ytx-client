#include "sectionpermissions.h"

#include <QtCore/qobject.h>

namespace section {

std::span<const PermissionItem> PermissionItems(Section section)
{
    static const PermissionItem basic[] = {
        { 0b01, QObject::tr("Read Only") },
        { 0b11, QObject::tr("Read Write") },
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
    const auto items { PermissionItems(section) };

    for (const auto& item : items) {
        if ((permissions & item.permission) != item.permission) {
            continue;
        }

        const auto covered = std::ranges::any_of(items, [&](const PermissionItem& other) {
            return other.permission != item.permission && (permissions & other.permission) == other.permission
                && (other.permission & item.permission) == item.permission;
        });

        if (!covered) {
            result.emplaceBack(item.text);
        }
    }

    return result.join(QStringLiteral(" | "));
}

}
