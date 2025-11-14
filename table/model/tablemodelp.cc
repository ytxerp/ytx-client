#include "tablemodelp.h"

#include <QDateTime>

#include "component/constant.h"
#include "global/entrypool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelP::TableModelP(CTableModelArg& arg, QObject* parent)
    : TableModel { arg, parent }
{
}

void TableModelP::RAppendMultiEntry(const EntryList& entry_list)
{
    if (entry_list.isEmpty())
        return;

    const auto row { entry_list_.size() };

    beginInsertRows(QModelIndex(), row, row + entry_list.size() - 1);
    entry_list_.append(entry_list);
    endInsertRows();

    sort(std::to_underlying(EntryEnum::kIssuedTime), Qt::AscendingOrder);
}

bool TableModelP::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);

    auto* entry { entry_list_.at(row) };
    auto rhs_node_id { entry->rhs_node };

    beginRemoveRows(parent, row, row);
    entry_list_.removeAt(row);
    endRemoveRows();

    const auto entry_id { entry->id };

    if (!rhs_node_id.isNull()) {
        QJsonObject message { JsonGen::EntryRemove(section_, entry_id) };
        WebSocket::Instance()->SendMessage(kEntryRemove, message);
    }

    internal_sku_.remove(rhs_node_id);

    emit SRemoveEntry(entry_id);
    return true;
}

bool TableModelP::UpdateInternalSku(EntryP* entry, const QUuid& value)
{
    if (value.isNull())
        return false;

    if (internal_sku_.contains(value))
        return false;

    auto old_node { entry->rhs_node };
    if (old_node == value)
        return false;

    entry->rhs_node = value;
    internal_sku_.insert(value);

    const QUuid entry_id { entry->id };

    const QString old_node_id { old_node.toString(QUuid::WithoutBraces) };
    const QString new_node_id { value.toString(QUuid::WithoutBraces) };

    QJsonObject message { JsonGen::EntryLinkedNode(section_, entry_id) };

    if (old_node.isNull()) {
        message.insert(kEntry, entry->WriteJson());

        WebSocket::Instance()->SendMessage(kEntryInsert, message);
    }

    if (!old_node.isNull()) {
        message.insert(kOldNodeId, old_node_id);
        message.insert(kNewNodeId, new_node_id);
        message.insert(kField, kRhsNode);

        WebSocket::Instance()->SendMessage(kEntryLinkedNode, message);
    }

    return true;
}

QVariant TableModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry = DerivedPtr<EntryP>(entry_list_.at(index.row()));

    const EntryEnumP column { index.column() };

    switch (column) {
    case EntryEnumP::kId:
        return entry->id;
    case EntryEnumP::kUserId:
        return entry->user_id;
    case EntryEnumP::kCreateTime:
        return entry->created_time;
    case EntryEnumP::kCreateBy:
        return entry->created_by;
    case EntryEnumP::kUpdateTime:
        return entry->updated_time;
    case EntryEnumP::kUpdateBy:
        return entry->updated_by;
    case EntryEnumP::kLhsNode:
        return entry->lhs_node;
    case EntryEnumP::kIssuedTime:
        return entry->issued_time;
    case EntryEnumP::kCode:
        return entry->code;
    case EntryEnumP::kUnitPrice:
        return entry->unit_price;
    case EntryEnumP::kDescription:
        return entry->description;
    case EntryEnumP::kDocument:
        return entry->document;
    case EntryEnumP::kStatus:
        return entry->status;
    case EntryEnumP::kRhsNode:
        return entry->rhs_node;
    case EntryEnumP::kExternalSku:
        return entry->external_sku;
    default:
        return QVariant();
    }
}

bool TableModelP::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const EntryEnumP column { index.column() };

    auto* entry { entry_list_.at(index.row()) };
    auto* d_entry = DerivedPtr<EntryP>(entry);

    const QUuid id { entry->id };

    switch (column) {
    case EntryEnumP::kIssuedTime:
        NodeUtils::UpdateIssuedTime(pending_updates_[id], entry, kIssuedTime, value.toDateTime(), &Entry::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kCode:
        NodeUtils::UpdateField(pending_updates_[id], entry, kCode, value.toString(), &Entry::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kDocument:
        NodeUtils::UpdateDocument(pending_updates_[id], entry, kDocument, value.toStringList(), &Entry::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kRhsNode:
        UpdateInternalSku(d_entry, value.toUuid());
        break;
    case EntryEnumP::kUnitPrice:
        NodeUtils::UpdateDouble(pending_updates_[id], d_entry, kUnitPrice, value.toDouble(), &EntryP::unit_price, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kDescription:
        NodeUtils::UpdateField(pending_updates_[id], entry, kDescription, value.toString(), &Entry::description, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kStatus:
        NodeUtils::UpdateField(pending_updates_[id], entry, kStatus, value.toInt(), &Entry::status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kExternalSku:
        NodeUtils::UpdateUuid(pending_updates_[id], d_entry, kExternalSku, value.toUuid(), &EntryP::external_sku, [id, this]() { RestartTimer(id); });
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void TableModelP::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column <= info_.entry_header.size() - 1);

    auto Compare = [column, order](Entry* lhs, Entry* rhs) -> bool {
        const EntryEnumP e_column { column };

        auto* d_lhs { DerivedPtr<EntryP>(lhs) };
        auto* d_rhs { DerivedPtr<EntryP>(rhs) };

        switch (e_column) {
        case EntryEnumP::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (d_lhs->issued_time < d_rhs->issued_time) : (d_lhs->issued_time > d_rhs->issued_time);
        case EntryEnumP::kUserId:
            return (order == Qt::AscendingOrder) ? (d_lhs->user_id < d_rhs->user_id) : (d_lhs->user_id > d_rhs->user_id);
        case EntryEnumP::kCreateTime:
            return (order == Qt::AscendingOrder) ? (d_lhs->created_time < d_rhs->created_time) : (d_lhs->created_time > d_rhs->created_time);
        case EntryEnumP::kCreateBy:
            return (order == Qt::AscendingOrder) ? (d_lhs->created_by < d_rhs->created_by) : (d_lhs->created_by > d_rhs->created_by);
        case EntryEnumP::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (d_lhs->updated_time < d_rhs->updated_time) : (d_lhs->updated_time > d_rhs->updated_time);
        case EntryEnumP::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (d_lhs->updated_by < d_rhs->updated_by) : (d_lhs->updated_by > d_rhs->updated_by);
        case EntryEnumP::kCode:
            return (order == Qt::AscendingOrder) ? (d_lhs->code < d_rhs->code) : (d_lhs->code > d_rhs->code);
        case EntryEnumP::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_price < d_rhs->unit_price) : (d_lhs->unit_price > d_rhs->unit_price);
        case EntryEnumP::kDescription:
            return (order == Qt::AscendingOrder) ? (d_lhs->description < d_rhs->description) : (d_lhs->description > d_rhs->description);
        case EntryEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case EntryEnumP::kStatus:
            return (order == Qt::AscendingOrder) ? (d_lhs->status < d_rhs->status) : (d_lhs->status > d_rhs->status);
        case EntryEnumP::kExternalSku:
            return (order == Qt::AscendingOrder) ? (d_lhs->external_sku < d_rhs->external_sku) : (d_lhs->external_sku > d_rhs->external_sku);
        case EntryEnumP::kRhsNode:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_node < d_rhs->rhs_node) : (d_lhs->rhs_node > d_rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}

Qt::ItemFlags TableModelP::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumP column { index.column() };

    switch (column) {
    case EntryEnumP::kId:
    case EntryEnumP::kDocument:
    case EntryEnumP::kStatus:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool TableModelP::insertRows(int row, int, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));

    auto* entry { EntryPool::Instance().Allocate(section_) };
    entry->id = QUuid::createUuidV7();
    entry->lhs_node = lhs_id_;
    entry->issued_time = QDateTime::currentDateTimeUtc();

    beginInsertRows(parent, row, row);
    entry_list_.emplaceBack(entry);
    endInsertRows();

    emit SInsertEntry(entry);
    return true;
}
