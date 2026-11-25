#include "tablemodelsingledeferred.h"

#include "global/entrypool.h"

TableModelSingleDeferred::~TableModelSingleDeferred() { EntryPool::Instance().Recycle(entry_list_, section_); }

void TableModelSingleDeferred::Finalize(QJsonObject& message)
{
    // deleted
    {
        QJsonArray deleted_entry_array {};
        for (const auto& id : std::as_const(pending_delete_)) {
            deleted_entry_array.append(id.toString(QUuid::WithoutBraces));
        }
        message.insert(kDeletedEntryArray, deleted_entry_array);
    }

    // insert and update
    {
        QJsonArray inserted_entry_array {};
        QJsonArray updated_entry_array {};

        for (auto* entry : std::as_const(entry_list_)) {
            switch (entry->sync_state) {
            case SyncState::kCreating:
                inserted_entry_array.append(entry->WriteJson());
                break;
            case SyncState::kUpdating:
                updated_entry_array.append(entry->WriteJson());
                break;
            case SyncState::kSynced:
            case SyncState::kDeleting:
            case SyncState::kError:
                break;
            }
        }

        message.insert(kInsertedEntryArray, inserted_entry_array);
        message.insert(kUpdatedEntryArray, updated_entry_array);
    }
}

void TableModelSingleDeferred::SyncSucceeded()
{
    for (auto* entry : std::as_const(entry_list_)) {
        switch (entry->sync_state) {
        case SyncState::kCreating:
        case SyncState::kUpdating:
            entry->sync_state = SyncState::kSynced;
            break;

        case SyncState::kSynced:
        case SyncState::kDeleting:
        case SyncState::kError:
            break;
        }
    }

    pending_delete_.clear();
}

bool TableModelSingleDeferred::UpdateDescription(Entry* entry, const QString& value)
{
    if (entry->description == value)
        return false;

    entry->description = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelSingleDeferred::UpdateDocument(Entry* entry, const QStringList& value)
{
    if (entry->document == value)
        return false;

    entry->document = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelSingleDeferred::UpdateTag(Entry* entry, const QStringList& value)
{
    if (entry->tag == value)
        return false;

    entry->tag = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelSingleDeferred::HasPendingUpdate() const
{
    if (!pending_delete_.isEmpty())
        return true;

    for (auto* entry : std::as_const(entry_list_)) {
        if (entry->sync_state == SyncState::kCreating || entry->sync_state == SyncState::kUpdating)
            return true;
    }
    return false;
}
