#pragma once

#include <QFlags>
#include <QString>
#include <span>

#include "enum/section.h"

namespace section {

inline QString Display(Section section)
{
    switch (section) {
    case Section::kFinance:
        return QObject::tr("Finance");
    case Section::kTask:
        return QObject::tr("Task");
    case Section::kInventory:
        return QObject::tr("Inventory");
    case Section::kPartner:
        return QObject::tr("Partner");
    case Section::kSale:
        return QObject::tr("Sale");
    case Section::kPurchase:
        return QObject::tr("Purchase");
    }

    std::unreachable();
}

struct Permissions {
    int finance {};
    int task {};
    int inventory {};
    int partner {};
    int sale {};
    int purchase {};
};

struct PermissionItem {
    int permission {};
    QString text {};
};

std::span<const PermissionItem> PermissionItems(Section section);
QString PermissionsDisplay(Section section, int permissions);
}
