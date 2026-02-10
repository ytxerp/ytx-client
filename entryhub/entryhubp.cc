#include "entryhubp.h"

#include <QJsonArray>

#include "global/entrypool.h"
#include "utils/entryutils.h"

EntryHubP::EntryHubP(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubP::DeleteLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) { DeleteLeafFunction(leaf_entry); }

void EntryHubP::ApplyInventoryIntReplace(const QUuid& old_item_id, const QUuid& new_item_id)
{
    for (auto* entry : std::as_const(entry_cache_)) {
        auto* d_entry = static_cast<EntryP*>(entry);

        if (d_entry->rhs_node == old_item_id)
            d_entry->rhs_node = new_item_id;
    }

    {
        QHash<std::pair<QUuid, QUuid>, EntryValue> new_map {};
        new_map.reserve(entry_map_.size());

        for (auto it = entry_map_.cbegin(); it != entry_map_.cend(); ++it) {
            auto key = it.key();
            auto value = it.value();

            if (key.second == old_item_id) {
                key.second = new_item_id;
            }

            new_map.insert(key, value);
        }

        entry_map_.swap(new_map);
    }
}

void EntryHubP::ApplyInventoryExtReplace(const QUuid& old_item_id, const QUuid& new_item_id)
{
    for (auto* entry : std::as_const(entry_cache_)) {
        auto* d_entry = static_cast<EntryP*>(entry);

        if (d_entry->external_sku == old_item_id)
            d_entry->external_sku = new_item_id;
    }

    for (auto it = entry_map_.begin(); it != entry_map_.end(); ++it) {
        if (it.value().external_sku == old_item_id) {
            it.value().external_sku = new_item_id;
        }
    }
}

std::optional<double> EntryHubP::UnitPrice(const QUuid& partner_id, const QUuid& internal_sku) const
{
    auto it = entry_map_.constFind({ partner_id, internal_sku });
    if (it != entry_map_.constEnd()) {
        return it->unit_price;
    }

    return std::nullopt;
}

QUuid EntryHubP::ExternalSku(const QUuid& partner_id, const QUuid& internal_sku) const
{
    auto it = entry_map_.constFind({ partner_id, internal_sku });
    if (it != entry_map_.constEnd()) {
        return it->external_sku;
    }

    return QUuid();
}

void EntryHubP::SearchDescription(QList<Entry*>& entry_list, CString& text) const
{
    entry_list.reserve(entry_cache_.size() / 2);

    for (const auto& [id, entry] : entry_cache_.asKeyValueRange()) {
        Q_ASSERT(entry && "EntryHubP::SearchEntry encountered null entry in cache");

        if (entry->description.contains(text, Qt::CaseInsensitive)) {
            entry_list.emplaceBack(entry);
        }
    }
}

void EntryHubP::SearchTag(QList<Entry*>& entry_list, const QSet<QString>& tag_set) const
{
    if (tag_set.isEmpty())
        return;

    for (const auto& [id, entry] : entry_cache_.asKeyValueRange()) {
        Q_ASSERT(entry && "EntryHubP::SearchTag encountered null entry in cache");

        const QStringList& entry_tags { entry->tag };
        for (const QString& tag_id : entry_tags) {
            if (tag_set.contains(tag_id)) {
                entry_list.emplaceBack(entry);
                break; // IMPORTANT: avoid duplicate insertion
            }
        }
    }
}

void EntryHubP::PushEntry(const QUuid& node_id)
{
    EntryList entry_list {};

    for (const auto& [id, entry] : entry_cache_.asKeyValueRange()) {
        Q_ASSERT(entry && "EntryHubP::PushEntry encountered null entry in cache");

        if (entry->lhs_node == node_id) {
            entry_list.emplaceBack(entry);
        }
    }

    emit SAppendMultiEntry(node_id, entry_list);
}

void EntryHubP::ApplyPartnerEntry(const QJsonArray& array)
{
    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };
        const QUuid id { QUuid(obj.value(kId).toString()) };

        EntryP* entry { static_cast<EntryP*>(EntryPool::Instance().Allocate(section_)) };
        entry->ReadJson(obj);
        entry_cache_.insert(id, entry);
        entry_map_.insert({ entry->lhs_node, entry->rhs_node }, { entry->unit_price, entry->external_sku });
    }
}

void EntryHubP::InsertEntry(const QJsonObject& data)
{
    EntryP* entry { static_cast<EntryP*>(EntryPool::Instance().Allocate(section_)) };
    entry->ReadJson(data);

    entry_cache_.insert(entry->id, entry);
    entry_map_.insert({ entry->lhs_node, entry->rhs_node }, { entry->unit_price, entry->external_sku });

    emit SAppendOneEntry(entry->lhs_node, entry);
}

void EntryHubP::DeleteEntry(const QUuid& entry_id)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* entry { static_cast<EntryP*>(it.value()) };

        entry_map_.remove({ entry->lhs_node, entry->rhs_node });

        emit SDeleteOneEntry(entry->lhs_node, entry_id);

        EntryPool::Instance().Recycle(entry, section_);
    }
}

void EntryHubP::UpdateEntry(const QUuid& id, const QJsonObject& update)
{
    auto it = entry_cache_.constFind(id);
    if (it != entry_cache_.constEnd()) {
        auto* entry { static_cast<EntryP*>(it.value()) };

        const QUuid old_rhs_node { entry->rhs_node };

        entry->ReadJson(update);

        {
            const bool update_map_value { update.contains(kUnitPrice) || update.contains(kExternalSku) };
            const bool update_map_key { update.contains(kRhsNode) };

            if (update_map_key) {
                entry_map_.remove({ entry->lhs_node, old_rhs_node });
                entry_map_.insert({ entry->lhs_node, entry->rhs_node }, { entry->unit_price, entry->external_sku });
            } else if (update_map_value)
                entry_map_[{ entry->lhs_node, old_rhs_node }] = { entry->unit_price, entry->external_sku };
        }

        const auto [start, end] = Utils::EntryCacheColumnRange(section_);
        emit SRefreshField(entry->lhs_node, id, start, end);
    };
}

#if 0
std::optional<std::pair<QUuid, double>> EntryHubP::ResolveFromExternal(const QUuid& partner_id, const QUuid& external_sku) const
{
    for (const auto* trans : std::as_const(entry_cache_)) {
        auto* d_trans = static_cast<const EntryP*>(trans);

        if (d_trans->lhs_node == partner_id && d_trans->external_sku == external_sku) {
            return std::make_pair(d_trans->rhs_node, d_trans->unit_price);
        }
    }

    return std::nullopt;
}
#endif
