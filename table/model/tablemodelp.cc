#include "tablemodelp.h"

#include <QDateTime>

#include "component/constant.h"
#include "component/constantwebsocket.h"
#include "enum/entryenum.h"
#include "global/entrypool.h"
#include "global/partner_inventory_registry.h"
#include "utils/entryutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelP::TableModelP(CTableModelArg& arg, QObject* parent)
    : TableModel { arg, parent }
{
}

void TableModelP::RAppendMultiEntries(const EntryList& entry_list)
{
    if (entry_list.isEmpty())
        return;

    EntryList list {};

    for (auto* entry : entry_list) {
        if (internal_sku_set_.contains(entry->rhs_node))
            continue;

        list.emplaceBack(entry);
        internal_sku_set_.insert(entry->rhs_node);
    }

    const auto row { list.size() };

    beginInsertRows(QModelIndex(), row, row + list.size() - 1);
    entry_list_.append(list);
    endInsertRows();

    sort(std::to_underlying(EntryEnum::kIssuedTime), Qt::AscendingOrder);
}

void TableModelP::RDetachOneEntry(const QUuid& entry_id, const QUuid& extra_value)
{
    auto idx { GetIndex(entry_id) };
    if (!idx.isValid())
        return;

    int row { idx.row() };
    beginRemoveRows(QModelIndex(), row, row);
    entry_list_.remove(row);
    endRemoveRows();

    internal_sku_set_.remove(extra_value);
}

void TableModelP::RAttachOneEntry(Entry* entry)
{
    auto row { entry_list_.size() };

    beginInsertRows(QModelIndex(), row, row);
    entry_list_.emplaceBack(entry);
    endInsertRows();

    internal_sku_set_.insert(entry->rhs_node);

    if (entry_list_.size() == 1)
        EmitDataChanged(row, row, std::to_underlying(EntryEnumP::kIssuedTime), std::to_underlying(EntryEnumP::kIssuedTime));
}

QModelIndex TableModelP::GetIndex(const QUuid& entry_id) const
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

bool TableModelP::removeRows(int row, int /*count*/, const QModelIndex& /*parent*/)
{
    Q_ASSERT(row >= 0 && row <= entry_list_.size() - 1);

    auto* entry { entry_list_.at(row) };
    if (entry->sync_state == SyncState::kDeleting)
        return true;

    const auto entry_id { entry->id };

    CancelPendingUpdate(entry_id);

    if (entry->sync_state == SyncState::kCreating) {
        // Never synced to server, safe to remove locally without a round trip.
        beginRemoveRows({}, row, row);
        entry_list_.removeAt(row);
        endRemoveRows();

        EntryPool::Instance().Recycle(entry, section_);
        return true;
    }

    entry->sync_state = SyncState::kDeleting;

    QJsonObject message { JsonGen::EntryMessage(section_, entry_id) };
    WebSocket::Instance()->SendMessage(WsKey::kEntryDelete, message);
    return true;
}

bool TableModelP::UpdateInternalSku(EntryP* entry, const QUuid& value)
{
    if (value.isNull())
        return false;

    const QUuid old_node { entry->rhs_node };
    if (old_node == value)
        return false;

    if (internal_sku_set_.contains(value))
        return false;

    entry->rhs_node = value;
    internal_sku_set_.insert(value);
    internal_sku_set_.remove(old_node);

    const QUuid entry_id { entry->id };

    QJsonObject message { JsonGen::EntryMessage(section_, entry_id) };

    if (old_node.isNull()) {
        entry->sync_state = SyncState::kSynced;

        message.insert(kEntry, entry->WriteJson());
        WebSocket::Instance()->SendMessage(WsKey::kEntryInsert, message);

        emit STransferOneEntry(entry);
        return true;
    }

    pending_updates_[entry_id].insert(kRhsNode, value.toString(QUuid::WithoutBraces));
    RestartTimer(entry_id, entry->version);

    return true;
}

QVariant TableModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const EntryEnumP column { index.column() };
    auto* d_entry { static_cast<EntryP*>(index.internalPointer()) };

    switch (column) {
    case EntryEnumP::kId:
        return d_entry->id;
    case EntryEnumP::kVersion:
        return d_entry->version;
    case EntryEnumP::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumP::kIssuedTime:
        return d_entry->issued_time;
    case EntryEnumP::kCode:
        return d_entry->code;
    case EntryEnumP::kUnitPrice:
        return d_entry->unit_price;
    case EntryEnumP::kDescription:
        return d_entry->description;
    case EntryEnumP::kDocument:
        return d_entry->document;
    case EntryEnumP::kTag:
        return d_entry->tag;
    case EntryEnumP::kStatus:
        return d_entry->status;
    case EntryEnumP::kRhsNode:
        return d_entry->rhs_node;
    case EntryEnumP::kExternalSku:
        return d_entry->external_sku;
    }
}

bool TableModelP::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const EntryEnumP column { index.column() };

    auto* entry { static_cast<Entry*>(index.internalPointer()) };
    auto* d_entry { static_cast<EntryP*>(entry) };

    bool insert_registry { false };
    bool update_registry { false };

    const QUuid id { entry->id };
    const QUuid old_rhs_node { entry->rhs_node };
    const int version { entry->version };

    switch (column) {
    case EntryEnumP::kIssuedTime:
        entry::UpdateIssuedTime(
            pending_updates_[id], entry, kIssuedTime, value.toDateTime(), &Entry::issued_time, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kCode:
        entry::UpdateField(pending_updates_[id], entry, kCode, value.toString(), &Entry::code, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kDocument:
        entry::UpdateStringList(
            pending_updates_[id], entry, kDocument, value.toStringList(), &Entry::document, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kTag:
        entry::UpdateStringList(pending_updates_[id], entry, kTag, value.toStringList(), &Entry::tag, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kRhsNode:
        insert_registry = UpdateInternalSku(d_entry, value.toUuid());
        break;
    case EntryEnumP::kUnitPrice:
        update_registry = entry::UpdateDouble(
            pending_updates_[id], d_entry, kUnitPrice, value.toDouble(), &EntryP::unit_price, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kDescription:
        entry::UpdateField(
            pending_updates_[id], entry, kDescription, value.toString(), &Entry::description, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kStatus:
        entry::UpdateField(pending_updates_[id], entry, kStatus, value.toInt(), &Entry::status, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kExternalSku:
        update_registry = entry::UpdateField(
            pending_updates_[id], d_entry, kExternalSku, value.toString(), &EntryP::external_sku, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumP::kId:
    case EntryEnumP::kVersion:
    case EntryEnumP::kLhsNode:
        return false;
    }

    auto& registry { PartnerInventoryRegistry::Instance() };

    if (update_registry) {
        registry.Insert(d_entry->lhs_node, d_entry->rhs_node, d_entry->unit_price, d_entry->external_sku);
    }

    if (insert_registry) {
        registry.Remove(d_entry->lhs_node, old_rhs_node);
        registry.Insert(d_entry->lhs_node, d_entry->rhs_node, d_entry->unit_price, d_entry->external_sku);
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void TableModelP::sort(int column, Qt::SortOrder order)
{
    const EntryEnumP e_column { column };

    auto Compare = [e_column, order](Entry* lhs, Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryP>(lhs) };
        auto* d_rhs { DerivedPtr<EntryP>(rhs) };

        switch (e_column) {
        case EntryEnumP::kCode:
            return utils::CompareMember(lhs, rhs, &Entry::code, order);
        case EntryEnumP::kDescription:
            return utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumP::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &Entry::issued_time, order);
        case EntryEnumP::kUnitPrice:
            return utils::CompareMember(d_lhs, d_rhs, &EntryP::unit_price, order);
        case EntryEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case EntryEnumP::kTag:
            return utils::CompareMember(lhs, rhs, &EntryP::tag, order);
        case EntryEnumP::kStatus:
            return utils::CompareMember(lhs, rhs, &Entry::status, order);
        case EntryEnumP::kExternalSku:
            return utils::CompareMember(d_lhs, d_rhs, &EntryP::external_sku, order);
        case EntryEnumP::kRhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case EntryEnumP::kId:
        case EntryEnumP::kVersion:
        case EntryEnumP::kLhsNode:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(entry_list_, Compare);
    emit layoutChanged();
}

Qt::ItemFlags TableModelP::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    auto* entry { static_cast<Entry*>(index.internalPointer()) };
    if (entry->sync_state == SyncState::kDeleting)
        return flags & ~Qt::ItemIsEditable;

    const EntryEnumP column { index.column() };

    switch (column) {
    case EntryEnumP::kId:
    case EntryEnumP::kVersion:
    case EntryEnumP::kDocument:
    case EntryEnumP::kTag:
    case EntryEnumP::kStatus:
    case EntryEnumP::kLhsNode:
        flags &= ~Qt::ItemIsEditable;
        break;
    case EntryEnumP::kIssuedTime:
    case EntryEnumP::kRhsNode:
    case EntryEnumP::kCode:
    case EntryEnumP::kDescription:
    case EntryEnumP::kUnitPrice:
    case EntryEnumP::kExternalSku:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

QModelIndex TableModelP::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, entry_list_.at(row));
}

bool TableModelP::insertRows(int row, int, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent));

    auto* entry { EntryPool::Instance().Allocate(section_) };
    entry->id = QUuid::createUuidV7();
    entry->lhs_node = node_id_;

    beginInsertRows(parent, row, row);
    entry_list_.emplaceBack(entry);
    endInsertRows();

    return true;
}
