#include "partner_inventory_registry.h"

void PartnerInventoryRegistry::ReplaceInternalSku(const QUuid& old_id, const QUuid& new_id)
{
    if (old_id.isNull() || new_id.isNull())
        return;

    QVector<Value> values {};
    QVector<std::pair<QUuid, QUuid>> keys {};

    for (auto it = map_.begin(); it != map_.end();) {
        if (it.key().second != old_id) {
            ++it;
            continue;
        }

        auto key = it.key();
        key.second = new_id;

        keys.push_back(key);
        values.push_back(it.value());

        it = map_.erase(it);
    }

    for (int i = 0; i < keys.size(); ++i)
        map_.insert(keys[i], values[i]);
}
