#include "partner_inventory_registry.h"

void PartnerInventoryRegistry::ReplaceInternalSku(const QUuid& old_id, const QUuid& new_id)
{
    if (old_id.isNull() || new_id.isNull())
        return;

    QHash<std::pair<QUuid, QUuid>, Value> pending {};

    for (auto it = hash_.begin(); it != hash_.end();) {
        if (it.key().second != old_id) {
            ++it;
            continue;
        }

        auto key = it.key();
        key.second = new_id;
        pending.insert(key, it.value());
        it = hash_.erase(it);
    }

    hash_.insert(pending);
}
