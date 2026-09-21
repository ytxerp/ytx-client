#include "sectionpermissions.h"

#include <QtCore/qobject.h>

namespace section {

std::span<const PermissionItem> PermissionItems(Section section)
{
    static const PermissionItem basic[] = {
        { 0b01, QObject::tr("R") },
        { 0b11, QObject::tr("W") },
    };

    static const PermissionItem order[] = {
        { 0b01, QObject::tr("R") },
        { 0b11, QObject::tr("W") },
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

QString PermissionsDisplay(const Permissions& permissions)
{
    QStringList result {};

    const auto append = [&](Section section, int value) {
        const QString text { PermissionsDisplay(section, value) };

        if (!text.isEmpty()) {
            result.emplaceBack(QStringLiteral("%1: %2").arg(Display(section), text));
        }
    };

    append(Section::kFinance, permissions.finance);
    append(Section::kTask, permissions.task);
    append(Section::kInventory, permissions.inventory);
    append(Section::kPartner, permissions.partner);
    append(Section::kSale, permissions.sale);
    append(Section::kPurchase, permissions.purchase);

    return result.join(QStringLiteral("; "));
}

}
