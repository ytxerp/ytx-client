#ifndef DATABASEROLE_H
#define DATABASEROLE_H

#include <QFlags>
#include <QStringList>

namespace PermissionBits {

enum Flag {
    FINANCE_READONLY = 0b01,
    FINANCE_READWRITE = 0b11,

    TASK_READONLY = 0b01 << 2,
    TASK_READWRITE = 0b11 << 2,

    INVENTORY_READONLY = 0b01 << 4,
    INVENTORY_READWRITE = 0b11 << 4,

    PARTNER_READONLY = 0b01 << 6,
    PARTNER_READWRITE = 0b11 << 6,

    SALE_READONLY = 0b01 << 8,
    SALE_READWRITE = 0b11 << 8,

    PURCHASE_READONLY = 0b01 << 10,
    PURCHASE_READWRITE = 0b11 << 10,
};

Q_DECLARE_FLAGS(Flags, Flag)
Q_DECLARE_OPERATORS_FOR_FLAGS(Flags)

}

#endif // DATABASEROLE_H
