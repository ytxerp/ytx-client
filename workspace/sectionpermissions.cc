#include "sectionpermissions.h"

#include <QtCore/qobject.h>

namespace section {

namespace {
    constexpr quint64 Bits(Permission permission) { return static_cast<quint64>(permission); }
}

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
        { Permission::kSaleRelease, QObject::tr("Sale Release") },
        { Permission::kSaleUnrelease, QObject::tr("Sale Unrelease") },
        { Permission::kSaleSettle, QObject::tr("Sale Settle") },
        { Permission::kSaleUnsettle, QObject::tr("Sale Unsettle") },

        { Permission::kPurchaseReadOnly, QObject::tr("Purchase R") },
        { Permission::kPurchaseReadWrite, QObject::tr("Purchase W") },
        { Permission::kPurchaseRelease, QObject::tr("Purchase Release") },
        { Permission::kPurchaseUnrelease, QObject::tr("Purchase Unrelease") },
        { Permission::kPurchaseSettle, QObject::tr("Purchase Settle") },
        { Permission::kPurchaseUnsettle, QObject::tr("Purchase Unsettle") },
    };

    return list;
}

QString PermissionsDisplay(Permissions permissions)
{
    if (permissions == 0) {
        return {};
    }

    QStringList result {};
    const auto items { PermissionItems() };

    for (const auto& item : items) {
        if (!permissions.testFlags(item.permission)) {
            continue;
        }

        const auto item_bits { Bits(item.permission) };

        const auto covered = std::ranges::any_of(items, [&](const PermissionItem& other) {
            const auto other_bits { Bits(other.permission) };
            return other_bits != item_bits && permissions.testFlags(other.permission) && (other_bits & item_bits) == item_bits;
        });

        if (!covered) {
            result.emplaceBack(item.text);
        }
    }

    return result.join(QStringLiteral(" | "));
}

}
