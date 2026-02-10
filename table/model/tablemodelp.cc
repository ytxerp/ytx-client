#include "tablemodelp.h"

#include <QDateTime>
#include <QtConcurrent>

#include "component/constant.h"
#include "enum/entryenum.h"
#include "global/entrypool.h"
#include "utils/compareutils.h"
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

void TableModelP::RDeleteOneEntry(const QUuid& entry_id)
{
    auto idx { GetIndex(entry_id) };
    if (!idx.isValid())
        return;

    int row { idx.row() };
    beginRemoveRows(QModelIndex(), row, row);
    entry_list_.remove(row);
    endRemoveRows();
}

void TableModelP::RAppendOneEntry(Entry* entry)
{
    auto row { entry_list_.size() };

    beginInsertRows(QModelIndex(), row, row);
    entry_list_.emplaceBack(entry);
    endInsertRows();

    if (entry_list_.size() == 1)
        emit SResizeColumnToContents(std::to_underlying(EntryEnum::kIssuedTime));
}

bool TableModelP::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent) - 1);

    auto* entry { entry_list_.at(row) };
    auto rhs_node_id { entry->rhs_node };

    beginRemoveRows(parent, row, row);
    entry_list_.removeAt(row);
    endRemoveRows();

    const auto entry_id { entry->id };

    if (!rhs_node_id.isNull()) {
        QJsonObject message { JsonGen::EntryDelete(section_, entry_id) };
        WebSocket::Instance()->SendMessage(kEntryDelete, message);

        emit SDeleteOneEntry(QUuid(), entry_id);
    } else {
        EntryPool::Instance().Recycle(entry, section_);
    }

    internal_sku_.remove(rhs_node_id);
    return true;
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

void TableModelP::ActionEntry(EntryAction action)
{
    if (entry_list_.isEmpty())
        return;

    QJsonObject message { JsonGen::EntryAction(section_, lhs_id_, std::to_underlying(action)) };
    WebSocket::Instance()->SendMessage(kEntryAction, message);

    auto Update = [action](Entry* entry) {
        switch (action) {
        case EntryAction::kMarkAll:
            entry->status = std::to_underlying(EntryStatus::kMarked);
            break;
        case EntryAction::kMarkNone:
            entry->status = std::to_underlying(EntryStatus::kUnmarked);
            break;
        case EntryAction::kMarkToggle:
            entry->status ^= std::to_underlying(EntryStatus::kMarked);
            break;
        default:
            break;
        }
    };

    auto future { QtConcurrent::map(entry_list_, Update) };
    auto* watcher { new QFutureWatcher<void>(this) };

    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
        const int column { std::to_underlying(EntryEnumP::kStatus) };
        emit dataChanged(index(0, column), index(rowCount() - 1, column));

        watcher->deleteLater();
    });

    watcher->setFuture(future);
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

    QJsonObject message { JsonGen::EntryLinkedNode(section_, entry_id) };

    if (old_node.isNull()) {
        message.insert(kEntry, entry->WriteJson());
        WebSocket::Instance()->SendMessage(kEntryInsert, message);

        emit SAppendOneEntry(entry);
    }

    if (!old_node.isNull()) {
        pending_updates_[entry_id].insert(kRhsNode, value.toString(QUuid::WithoutBraces));
        RestartTimer(entry_id);
    }

    return true;
}

QVariant TableModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* d_entry = DerivedPtr<EntryP>(entry_list_.at(index.row()));

    const EntryEnumP column { index.column() };

    switch (column) {
    case EntryEnumP::kId:
        return d_entry->id;
    case EntryEnumP::kUserId:
        return d_entry->user_id;
    case EntryEnumP::kCreateTime:
        return d_entry->created_time;
    case EntryEnumP::kCreateBy:
        return d_entry->created_by;
    case EntryEnumP::kUpdateTime:
        return d_entry->updated_time;
    case EntryEnumP::kVersion:
        return d_entry->version;
    case EntryEnumP::kUpdateBy:
        return d_entry->updated_by;
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
    default:
        return QVariant();
    }
}

bool TableModelP::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const EntryEnumP column { index.column() };

    auto* entry { entry_list_.at(index.row()) };
    auto* d_entry = DerivedPtr<EntryP>(entry);

    const QUuid id { entry->id };

    switch (column) {
    case EntryEnumP::kIssuedTime:
        Utils::UpdateIssuedTime(pending_updates_[id], entry, kIssuedTime, value.toDateTime(), &Entry::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kCode:
        Utils::UpdateField(pending_updates_[id], entry, kCode, value.toString(), &Entry::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kDocument:
        Utils::UpdateStringList(pending_updates_[id], entry, kDocument, value.toStringList(), &Entry::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kTag:
        Utils::UpdateStringList(pending_updates_[id], entry, kTag, value.toStringList(), &Entry::tag, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kRhsNode:
        UpdateInternalSku(d_entry, value.toUuid());
        break;
    case EntryEnumP::kUnitPrice:
        Utils::UpdateDouble(pending_updates_[id], d_entry, kUnitPrice, value.toDouble(), &EntryP::unit_price, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kDescription:
        Utils::UpdateField(pending_updates_[id], entry, kDescription, value.toString(), &Entry::description, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kStatus:
        Utils::UpdateField(pending_updates_[id], entry, kStatus, value.toInt(), &Entry::status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kExternalSku:
        Utils::UpdateUuid(pending_updates_[id], d_entry, kExternalSku, value.toUuid(), &EntryP::external_sku, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kId:
    case EntryEnumP::kUpdateBy:
    case EntryEnumP::kUpdateTime:
    case EntryEnumP::kCreateTime:
    case EntryEnumP::kCreateBy:
    case EntryEnumP::kVersion:
    case EntryEnumP::kUserId:
    case EntryEnumP::kLhsNode:
        return false;
    }

    emit SResizeColumnToContents(index.column());
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
            return Utils::CompareMember(lhs, rhs, &Entry::code, order);
        case EntryEnumP::kDescription:
            return Utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumP::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &Entry::issued_time, order);
        case EntryEnumP::kUnitPrice:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryP::unit_price, order);
        case EntryEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case EntryEnumP::kTag:
            return Utils::CompareMember(lhs, rhs, &EntryP::tag, order);
        case EntryEnumP::kStatus:
            return Utils::CompareMember(lhs, rhs, &Entry::status, order);
        case EntryEnumP::kExternalSku:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryP::external_sku, order);
        case EntryEnumP::kRhsNode:
            return Utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case EntryEnumP::kId:
        case EntryEnumP::kUpdateBy:
        case EntryEnumP::kUpdateTime:
        case EntryEnumP::kCreateTime:
        case EntryEnumP::kCreateBy:
        case EntryEnumP::kVersion:
        case EntryEnumP::kUserId:
        case EntryEnumP::kLhsNode:
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
    entry->lhs_node = lhs_id_;

    last_issued_ = last_issued_.isValid() ? last_issued_.addSecs(1) : QDateTime::currentDateTimeUtc();
    entry->issued_time = last_issued_;

    beginInsertRows(parent, row, row);
    entry_list_.emplaceBack(entry);
    endInsertRows();

    if (entry_list_.size() == 1)
        emit SResizeColumnToContents(std::to_underlying(EntryEnum::kIssuedTime));

    return true;
}
