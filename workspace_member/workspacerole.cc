#include "workspacerole.h"

namespace workspace_role {
std::span<const Item> RoleList()
{
    static const Item list[] = {
        { static_cast<int>(WorkspaceRole::kGuest), QObject::tr("Guest") },
        { static_cast<int>(WorkspaceRole::kMember), QObject::tr("Member") },
        { static_cast<int>(WorkspaceRole::kAdmin), QObject::tr("Admin") },
        { static_cast<int>(WorkspaceRole::kOwner), QObject::tr("Owner") },
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
