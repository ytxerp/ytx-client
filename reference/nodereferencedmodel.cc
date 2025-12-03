#include "nodereferencedmodel.h"

#include "enum/nodeenum.h"
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
    const NodeReferencedEnum column { index.column() };

    switch (column) {
    case NodeReferencedEnum::kIssuedTime:
        return entry->issued_time;
    case NodeReferencedEnum::kNodeId:
        return entry->node_id;
    case NodeReferencedEnum::kSection:
        return entry->section;
    case NodeReferencedEnum::kOrderId:
        return entry->order_id;
    case NodeReferencedEnum::kExternalSku:
        return entry->external_sku;
    case NodeReferencedEnum::kkCount:
        return entry->count;
    case NodeReferencedEnum::kkMeasure:
        return entry->measure;
    case NodeReferencedEnum::kUnitPrice:
        return entry->unit_price;
    case NodeReferencedEnum::kUnitDiscount:
        return entry->unit_discount;
    case NodeReferencedEnum::kDescription:
        return entry->description;
    case NodeReferencedEnum::kInitial:
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
        const NodeReferencedEnum e_column { column };

        switch (e_column) {
        case NodeReferencedEnum::kExternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->external_sku < rhs->external_sku) : (lhs->external_sku > rhs->external_sku);
        case NodeReferencedEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case NodeReferencedEnum::kNodeId:
            return (order == Qt::AscendingOrder) ? (lhs->node_id < rhs->node_id) : (lhs->node_id > rhs->node_id);
        case NodeReferencedEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case NodeReferencedEnum::kkCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case NodeReferencedEnum::kkMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case NodeReferencedEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeReferencedEnum::kInitial:
            return (order == Qt::AscendingOrder) ? (lhs->initial < rhs->initial) : (lhs->initial > rhs->initial);
        case NodeReferencedEnum::kUnitDiscount:
            return (order == Qt::AscendingOrder) ? (lhs->unit_discount < rhs->unit_discount) : (lhs->unit_discount > rhs->unit_discount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
