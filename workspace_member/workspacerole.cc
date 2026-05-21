#include "workspacerole.h"

namespace workspace {
std::span<const RoleItem> RoleList()
{
    static const RoleItem list[] = {
        { static_cast<int>(Role::kGuest), QObject::tr("Guest") },
        { static_cast<int>(Role::kMember), QObject::tr("Member") },
        { static_cast<int>(Role::kAdmin), QObject::tr("Admin") },
        { static_cast<int>(Role::kOwner), QObject::tr("Owner") },
    };

    return list;
}

QHash<int, QString> RoleHash()
{
    static const QHash<int, QString> hash = []() {
        QHash<int, QString> h {};
        for (const auto& item : RoleList())
            h.insert(item.role, item.text);
        return h;
    }();

    return hash;
}

}
