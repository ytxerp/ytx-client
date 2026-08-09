#include "workspacerole.h"

namespace workspace {
std::span<const RoleItem> RoleItems()
{
    static const RoleItem list[] = {
        { Role::kGuest, QObject::tr("Guest") },
        { Role::kMember, QObject::tr("Member") },
        { Role::kAdmin, QObject::tr("Admin") },
        { Role::kOwner, QObject::tr("Owner") },
    };

    return list;
}

QString RoleDisplay(Role role)
{
    for (const auto& item : RoleItems()) {
        if (item.role == role)
            return item.text;
    }

    return {};
}
}
