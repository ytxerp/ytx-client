#include "nodereferencedmodel.h"

#include "enum/entryenum.h"
#include "global/resourcepool.h"

NodeReferencedModel::NodeReferencedModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

NodeReferencedModel::~NodeReferencedModel() { ResourcePool<NodeReferenced>::Instance().Recycle(list_); }

QModelIndex NodeReferencedModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex NodeReferencedModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int NodeReferencedModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int NodeReferencedModel::columnCount(const QModelIndex& /*parent*/) const { return info_.node_referenced_header.size(); }

QVariant NodeReferencedModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry { list_.at(index.row()) };
    const EntryRefEnum column { index.column() };

    switch (column) {
    case EntryRefEnum::kIssuedTime:
        return entry->issued_time;
    case EntryRefEnum::kNodeId:
        return entry->node_id;
    case EntryRefEnum::kSection:
        return entry->section;
    case EntryRefEnum::kOrderId:
        return entry->order_id;
    case EntryRefEnum::kExternalSku:
        return entry->external_sku;
    case EntryRefEnum::kkCount:
        return entry->count;
    case EntryRefEnum::kkMeasure:
        return entry->measure;
    case EntryRefEnum::kUnitPrice:
        return entry->unit_price;
    case EntryRefEnum::kUnitDiscount:
        return entry->unit_discount;
    case EntryRefEnum::kDescription:
        return entry->description;
    case EntryRefEnum::kInitial:
        return entry->initial;
    default:
        return QVariant();
    }
}

QVariant NodeReferencedModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.node_referenced_header.at(section);

    return QVariant();
}

void NodeReferencedModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    auto Compare = [column, order](const NodeReferenced* lhs, const NodeReferenced* rhs) -> bool {
        const EntryRefEnum e_column { column };

        switch (e_column) {
        case EntryRefEnum::kExternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->external_sku < rhs->external_sku) : (lhs->external_sku > rhs->external_sku);
        case EntryRefEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case EntryRefEnum::kNodeId:
            return (order == Qt::AscendingOrder) ? (lhs->node_id < rhs->node_id) : (lhs->node_id > rhs->node_id);
        case EntryRefEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case EntryRefEnum::kkCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case EntryRefEnum::kkMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case EntryRefEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case EntryRefEnum::kInitial:
            return (order == Qt::AscendingOrder) ? (lhs->initial < rhs->initial) : (lhs->initial > rhs->initial);
        case EntryRefEnum::kUnitDiscount:
            return (order == Qt::AscendingOrder) ? (lhs->unit_discount < rhs->unit_discount) : (lhs->unit_discount > rhs->unit_discount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
