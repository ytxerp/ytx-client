#pragma once

#include <QString>
#include <span>

#include "enum/section.h"

namespace section {

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
