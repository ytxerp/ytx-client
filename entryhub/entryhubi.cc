#include "entryhubi.h"

EntryHubI::EntryHubI(const Section section, QObject* parent)
    : EntryHub(section, parent)
{
}

void EntryHubI::ReplaceLeaf(QUuid old_node_id, QUuid new_node_id)
{
    Q_ASSERT(section_ == Section::kInventory && "Invalid section: only Inventory nodes are allowed to be replaced");

    QSet<QUuid> entry_id_set {};
    EntryList entry_list {};

    for (auto* entry : std::as_const(entry_cache_)) {
        if (entry->lhs_node == old_node_id && entry->rhs_node != new_node_id) {
            entry_id_set.insert(entry->id);
            entry_list.emplaceBack(entry);
            entry->lhs_node = new_node_id;
            entry->version += 1;
        }

        if (entry->rhs_node == old_node_id && entry->lhs_node != new_node_id) {
            entry_id_set.insert(entry->id);
            entry_list.emplaceBack(entry);
            entry->rhs_node = new_node_id;
            entry->version += 1;
        }
    }

    emit SDeleteEntries(old_node_id, entry_id_set);
    emit SAppendEntries(new_node_id, entry_list);
}
