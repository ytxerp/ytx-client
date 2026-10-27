#include "partner_inventory_registry.h"

void PartnerInventoryRegistry::MergeInternalSku(QUuid source_id, QUuid target_id)
{
    if (source_id.isNull() || target_id.isNull())
        return;

    QHash<std::pair<QUuid, QUuid>, Value> pending {};

    for (auto it = hash_.begin(); it != hash_.end();) {
        if (it.key().second != source_id) {
            ++it;
            continue;
        }

        auto key = it.key();
        key.second = target_id;
        pending.insert(key, it.value());
        it = hash_.erase(it);
    }

    hash_.insert(pending);
}
