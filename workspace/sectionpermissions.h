#pragma once

#include <QFlags>
#include <QString>
#include <span>

namespace section {

enum class Permission {
    kFinanceReadOnly = 0b01,
    kFinanceReadWrite = 0b11,

    kTaskReadOnly = 0b01 << 2,
    kTaskReadWrite = 0b11 << 2,

    kInventoryReadOnly = 0b01 << 4,
    kInventoryReadWrite = 0b11 << 4,

    kPartnerReadOnly = 0b01 << 6,
    kPartnerReadWrite = 0b11 << 6,

    kSaleReadOnly = 0b01 << 8,
    kSaleReadWrite = 0b11 << 8,

    kPurchaseReadOnly = 0b01 << 10,
    kPurchaseReadWrite = 0b11 << 10,
};

Q_DECLARE_FLAGS(Permissions, Permission)
Q_DECLARE_OPERATORS_FOR_FLAGS(Permissions)

struct PermissionItem {
    Permission permission {};
    QString text {};
};

std::span<const PermissionItem> PermissionItems();
QString PermissionsDisplay(Permissions permissions);
}
