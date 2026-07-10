#ifndef DATABASEROLE_H
#define DATABASEROLE_H

#include <QFlags>
#include <QString>
#include <span>

namespace database {

enum class Role {
    kFinanceReadonly = 0b01,
    kFinanceReadwrite = 0b11,

    kTaskReadonly = 0b01 << 2,
    kTaskReadwrite = 0b11 << 2,

    kInventoryReadonly = 0b01 << 4,
    kInventoryReadwrite = 0b11 << 4,

    kPartnerReadonly = 0b01 << 6,
    kPartnerReadwrite = 0b11 << 6,

    kSaleReadonly = 0b01 << 8,
    kSaleReadwrite = 0b11 << 8,

    kPurchaseReadonly = 0b01 << 10,
    kPurchaseReadwrite = 0b11 << 10,
};

Q_DECLARE_FLAGS(Roles, Role)
Q_DECLARE_OPERATORS_FOR_FLAGS(Roles)

struct RoleItem {
    Role role {};
    QString text {};
};

std::span<const RoleItem> RoleItemList();
QString RolesDisplay(Roles roles);
}

#endif // DATABASEROLE_H
