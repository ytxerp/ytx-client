#include "searchentrymodel.h"

#include "global/websocket.h"
#include "utils/jsongen.h"

SearchEntryModel::SearchEntryModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

void SearchEntryModel::RSearchEntry(const EntryList& entry_list)
{
    beginResetModel();
    entry_list_ = entry_list;
    endResetModel();
}

QModelIndex SearchEntryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SearchEntryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SearchEntryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return entry_list_.size();
}

int SearchEntryModel::columnCount(const QModelIndex& /*parent*/) const { return info_.full_entry_header.size(); }

QVariant SearchEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.full_entry_header.at(section);

    return QVariant();
}

void SearchEntryModel::Search(const QString& text)
{
    if (!text.isEmpty()) {
        WebSocket::Instance().SendMessage(kEntrySearch, JsonGen::SearchEntry(info_.section_str, text));
    }
}
