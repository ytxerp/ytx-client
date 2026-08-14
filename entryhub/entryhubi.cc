#include "entryhubi.h"

EntryHubI::EntryHubI(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubI::ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id)
{
    Q_ASSERT(section_ == Section::kInventory && "Invalid section: only Inventory nodes are allowed to be replaced");

    QSet<QUuid> entry_id_set {};
    EntryList entry_list {};

    for (auto* entry : std::as_const(entry_cache_)) {
        if (entry->lhs_node == old_node_id && entry->rhs_node != new_node_id) {
            entry_id_set.insert(entry->id);
            entry_list.emplaceBack(entry);
            entry->lhs_node = new_node_id;
        }

        if (entry->rhs_node == old_node_id && entry->lhs_node != new_node_id) {
            entry_id_set.insert(entry->id);
            entry_list.emplaceBack(entry);
            entry->rhs_node = new_node_id;
        }
    }

    emit SDeleteEntries(old_node_id, entry_id_set);
    emit SAppendEntries(new_node_id, entry_list);
}
