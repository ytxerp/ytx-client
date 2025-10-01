#include "leafmodelp.h"

#include <QDateTime>

#include "component/constant.h"
#include "global/entryshadowpool.h"
#include "global/websocket.h"
#include "utils/jsongen.h"

LeafModelP::LeafModelP(CLeafModelArg& arg, QObject* parent)
    : LeafModel { arg, parent }
{
}

bool LeafModelP::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);

    auto* entry_shadow { shadow_list_.at(row) };
    auto rhs_node_id { *entry_shadow->rhs_node };

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    if (!rhs_node_id.isNull()) {
        QJsonObject message {};
        message.insert(kSection, info_.section_str);
        message.insert(kSessionId, QString());
        message.insert(kEntryId, entry_shadow->id->toString(QUuid::WithoutBraces));

        WebSocket::Instance()->SendMessage(kEntryRemove, message);
    }

    EntryShadowPool::Instance().Recycle(entry_shadow, section_);
    return true;
}

bool LeafModelP::UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int /*row*/)
{
    if (value.isNull())
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowP>(entry_shadow);

    auto old_node { *d_shadow->rhs_node };
    if (old_node == value)
        return false;

    *d_shadow->rhs_node = value;

    const QUuid entry_id { *d_shadow->id };

    const QString old_node_id = old_node.toString(QUuid::WithoutBraces);
    const QString new_node_id = value.toString(QUuid::WithoutBraces);

    QJsonObject cache {};
    cache = d_shadow->WriteJson();

    QJsonObject message {};
    message.insert(kSection, info_.section_str);
    message.insert(kSessionId, QString());
    message.insert(kEntry, cache);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    if (old_node.isNull()) {
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

QVariant LeafModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_shadow = DerivedPtr<EntryShadowP>(shadow_list_.at(index.row()));

    const EntryEnumP kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumP::kId:
        return *d_shadow->id;
    case EntryEnumP::kUserId:
        return *d_shadow->user_id;
    case EntryEnumP::kCreateTime:
        return *d_shadow->created_time;
    case EntryEnumP::kCreateBy:
        return *d_shadow->created_by;
    case EntryEnumP::kUpdateTime:
        return *d_shadow->updated_time;
    case EntryEnumP::kUpdateBy:
        return *d_shadow->updated_by;
    case EntryEnumP::kLhsNode:
        return *d_shadow->lhs_node;
    case EntryEnumP::kIssuedTime:
        return *d_shadow->issued_time;
    case EntryEnumP::kCode:
        return *d_shadow->code;
    case EntryEnumP::kUnitPrice:
        return *d_shadow->unit_price == 0 ? QVariant() : *d_shadow->unit_price;
    case EntryEnumP::kDescription:
        return *d_shadow->description;
    case EntryEnumP::kDocument:
        return d_shadow->document->isEmpty() ? QVariant() : *d_shadow->document;
    case EntryEnumP::kMarkStatus:
        return *d_shadow->mark_status ? *d_shadow->mark_status : QVariant();
    case EntryEnumP::kRhsNode:
        return d_shadow->rhs_node->isNull() ? QVariant() : *d_shadow->rhs_node;
    case EntryEnumP::kExternalSku:
        return d_shadow->external_sku->isNull() ? QVariant() : *d_shadow->external_sku;
    default:
        return QVariant();
    }
}

bool LeafModelP::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const EntryEnumP kColumn { index.column() };
    const int kRow { index.row() };

    auto* shadow { shadow_list_.at(kRow) };
    auto* d_shadow = DerivedPtr<EntryShadowP>(shadow);

    const QUuid id { *shadow->id };

    switch (kColumn) {
    case EntryEnumP::kIssuedTime:
        EntryUtils::UpdateShadowIssuedTime(caches_[id], shadow, kIssuedTime, value.toDateTime(), &EntryShadow::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kCode:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kCode, value.toString(), &EntryShadow::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kDocument:
        EntryUtils::UpdateShadowDocument(caches_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kRhsNode:
        UpdateLinkedNode(shadow, value.toUuid(), kRow);
        break;
    case EntryEnumP::kUnitPrice:
        EntryUtils::UpdateShadowDouble(caches_[id], d_shadow, kUnitPrice, value.toDouble(), &EntryShadowP::unit_price, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kDescription:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kMarkStatus:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kMarkStatus, value.toBool(), &EntryShadow::mark_status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumP::kExternalSku:
        EntryUtils::UpdateShadowUuid(caches_[id], d_shadow, kExternalSku, value.toUuid(), &EntryShadowP::external_sku, [id, this]() { RestartTimer(id); });
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void LeafModelP::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0);
    if (column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](EntryShadow* lhs, EntryShadow* rhs) -> bool {
        const EntryEnumP kColumn { column };

        auto* d_lhs { DerivedPtr<EntryShadowP>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowP>(rhs) };

        switch (kColumn) {
        case EntryEnumP::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (*lhs->issued_time < *rhs->issued_time) : (*lhs->issued_time > *rhs->issued_time);
        case EntryEnumP::kUserId:
            return (order == Qt::AscendingOrder) ? (*lhs->user_id < *rhs->user_id) : (*lhs->user_id > *rhs->user_id);
        case EntryEnumP::kCreateTime:
            return (order == Qt::AscendingOrder) ? (*lhs->created_time < *rhs->created_time) : (*lhs->created_time > *rhs->created_time);
        case EntryEnumP::kCreateBy:
            return (order == Qt::AscendingOrder) ? (*lhs->created_by < *rhs->created_by) : (*lhs->created_by > *rhs->created_by);
        case EntryEnumP::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (*lhs->updated_time < *rhs->updated_time) : (*lhs->updated_time > *rhs->updated_time);
        case EntryEnumP::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (*lhs->updated_by < *rhs->updated_by) : (*lhs->updated_by > *rhs->updated_by);
        case EntryEnumP::kCode:
            return (order == Qt::AscendingOrder) ? (*lhs->code < *rhs->code) : (*lhs->code > *rhs->code);
        case EntryEnumP::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (*d_lhs->unit_price < *d_rhs->unit_price) : (*d_lhs->unit_price > *d_rhs->unit_price);
        case EntryEnumP::kDescription:
            return (order == Qt::AscendingOrder) ? (*lhs->description < *rhs->description) : (*lhs->description > *rhs->description);
        case EntryEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document->size() < rhs->document->size()) : (lhs->document->size() > rhs->document->size());
        case EntryEnumP::kMarkStatus:
            return (order == Qt::AscendingOrder) ? (*lhs->mark_status < *rhs->mark_status) : (*lhs->mark_status > *rhs->mark_status);
        case EntryEnumP::kExternalSku:
            return (order == Qt::AscendingOrder) ? (*d_lhs->external_sku < *d_rhs->external_sku) : (*d_lhs->external_sku > *d_rhs->external_sku);
        case EntryEnumP::kRhsNode:
            return (order == Qt::AscendingOrder) ? (*lhs->rhs_node < *rhs->rhs_node) : (*lhs->rhs_node > *rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(shadow_list_.begin(), shadow_list_.end(), Compare);
    emit layoutChanged();
}

Qt::ItemFlags LeafModelP::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumP kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumP::kId:
    case EntryEnumP::kDocument:
    case EntryEnumP::kMarkStatus:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}
