#include "tablemodelsingle.h"

#include "enum/entryenum.h"

QModelIndex TableModelSingle::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, entry_list_.at(row));
}

QModelIndex TableModelSingle::GetIndex(QUuid entry_id) const
{
    int row { 0 };

    for (const auto* entry : entry_list_) {
        if (entry->id == entry_id) {
            return index(row, 0);
        }
        ++row;
    }

    return QModelIndex();
}

void TableModelSingle::RAppendEntries(const EntryList& entry_list)
{
    if (entry_list.isEmpty())
        return;

    const auto row { entry_list_.size() };

    beginInsertRows(QModelIndex(), row, row + entry_list.size() - 1);
    entry_list_.append(entry_list);
    endInsertRows();

    sort(std::to_underlying(EntryEnum::kIssuedTime), Qt::AscendingOrder);
}
