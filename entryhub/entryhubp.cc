#include "entryhubp.h"

#include <QJsonArray>

#include "global/entrypool.h"
#include "global/partner_inventory_registry.h"
#include "utils/entryutils.h"

EntryHubP::EntryHubP(Section section, QObject* parent)
    : EntryHub(section, parent)
{
}

void EntryHubP::DeleteSingleLeaf(const QSet<QUuid>& leaf_entry)
{
    qInfo() << Q_FUNC_INFO << leaf_entry.size();

    for (QUuid entry_id : leaf_entry) {
        Entry* entry { entry_cache_.take(entry_id) };
        if (!entry)
            continue;

        PartnerInventoryRegistry::Instance().Remove(entry->lhs_node, entry->rhs_node);
        EntryPool::Instance().Recycle(entry, section_);
    }
}

void EntryHubP::ReplaceInternalInventoryRef(QUuid old_item_id, QUuid new_item_id)
{
    for (auto* entry : std::as_const(entry_cache_)) {
        if (entry->rhs_node == old_item_id) {
            entry->rhs_node = new_item_id;
            entry->version += 1;
        }
    }

    PartnerInventoryRegistry::Instance().ReplaceInternalSku(old_item_id, new_item_id);
}

void EntryHubP::PushEntry(QUuid node_id)
{
    EntryList entry_list {};

    for (const auto& [id, entry] : entry_cache_.asKeyValueRange()) {
        Q_ASSERT(entry && "EntryHubP::PushEntry encountered null entry in cache");

        if (entry->lhs_node == node_id) {
            entry_list.emplaceBack(entry);
        }
    }

    emit SAppendEntries(node_id, entry_list);
}

void EntryHubP::ApplyPartnerEntry(const QJsonArray& array)
{
    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        const QJsonObject obj { value.toObject() };
        const QUuid id { QUuid(obj.value(kId).toString()) };

        EntryP* entry { static_cast<EntryP*>(EntryPool::Instance().Allocate(section_)) };
        entry->ReadJson(obj);
        entry_cache_.insert(id, entry);
        PartnerInventoryRegistry::Instance().Insert(entry->lhs_node, entry->rhs_node, entry->unit_price, entry->external_sku);
    }
}

void EntryHubP::InsertEntry(const QJsonObject& data)
{
    EntryP* entry { static_cast<EntryP*>(EntryPool::Instance().Allocate(section_)) };
    entry->ReadJson(data);

    entry_cache_.insert(entry->id, entry);
    PartnerInventoryRegistry::Instance().Insert(entry->lhs_node, entry->rhs_node, entry->unit_price, entry->external_sku);

    emit SAttachEntry(entry->lhs_node, entry);
}

void EntryHubP::DeleteEntry(QUuid entry_id)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it == entry_cache_.constEnd()) {
        return;
    }

    auto* entry { it.value() };

    PartnerInventoryRegistry::Instance().Remove(entry->lhs_node, entry->rhs_node);

    emit SDetachEntry(entry->lhs_node, entry_id, entry->rhs_node);

    entry_cache_.erase(it);
    EntryPool::Instance().Recycle(entry, section_);
}

void EntryHubP::UpdateEntry(QUuid id, const QJsonObject& update, int version)
{
    auto it = entry_cache_.constFind(id);
    if (it == entry_cache_.constEnd()) {
        return;
    }

    auto* entry { static_cast<EntryP*>(it.value()) };

    const QUuid old_rhs_node { entry->rhs_node };

    entry->ReadJson(update);
    entry->version = version;
    entry->sync_state = SyncState::kSynced;

    {
        const bool update_map_value { update.contains(kUnitPrice) || update.contains(kExternalSku) };
        const bool update_map_key { update.contains(kRhsNode) };

        auto& registry { PartnerInventoryRegistry::Instance() };

        if (update_map_key) {
            registry.Remove(entry->lhs_node, old_rhs_node);
            registry.Insert(entry->lhs_node, entry->rhs_node, entry->unit_price, entry->external_sku);
        } else if (update_map_value)
            registry.Insert(entry->lhs_node, old_rhs_node, entry->unit_price, entry->external_sku);
    }

    const auto [start, end] = entry::CacheColumnRange(section_);
    emit SRefreshField(entry->lhs_node, id, start, end);
}
