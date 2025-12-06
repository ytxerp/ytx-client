#include "salereferencemodel.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/resourcepool.h"

SaleReferenceModel::SaleReferenceModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

SaleReferenceModel::~SaleReferenceModel() { ResourcePool<SaleReference>::Instance().Recycle(list_); }

QModelIndex SaleReferenceModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SaleReferenceModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SaleReferenceModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SaleReferenceModel::columnCount(const QModelIndex& /*parent*/) const { return info_.node_referenced_header.size(); }

QVariant SaleReferenceModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* sale_reference { list_.at(index.row()) };
    const SaleReferenceEnum column { index.column() };

    switch (column) {
    case SaleReferenceEnum::kIssuedTime:
        return sale_reference->issued_time;
    case SaleReferenceEnum::kNodeId:
        return sale_reference->node_id;
    case SaleReferenceEnum::kOrderId:
        return sale_reference->order_id;
    case SaleReferenceEnum::kExternalSku:
        return sale_reference->external_sku;
    case SaleReferenceEnum::kCount:
        return sale_reference->count;
    case SaleReferenceEnum::kMeasure:
        return sale_reference->measure;
    case SaleReferenceEnum::kUnitPrice:
        return sale_reference->unit_price;
    case SaleReferenceEnum::kUnitDiscount:
        return sale_reference->unit_discount;
    case SaleReferenceEnum::kDescription:
        return sale_reference->description;
    case SaleReferenceEnum::kInitial:
        return sale_reference->initial;
    default:
        return QVariant();
    }
}

QVariant SaleReferenceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.node_referenced_header.at(section);

    return QVariant();
}

void SaleReferenceModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    auto Compare = [column, order](const SaleReference* lhs, const SaleReference* rhs) -> bool {
        const SaleReferenceEnum e_column { column };

        switch (e_column) {
        case SaleReferenceEnum::kExternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->external_sku < rhs->external_sku) : (lhs->external_sku > rhs->external_sku);
        case SaleReferenceEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case SaleReferenceEnum::kNodeId:
            return (order == Qt::AscendingOrder) ? (lhs->node_id < rhs->node_id) : (lhs->node_id > rhs->node_id);
        case SaleReferenceEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case SaleReferenceEnum::kCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case SaleReferenceEnum::kMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case SaleReferenceEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case SaleReferenceEnum::kInitial:
            return (order == Qt::AscendingOrder) ? (lhs->initial < rhs->initial) : (lhs->initial > rhs->initial);
        case SaleReferenceEnum::kUnitDiscount:
            return (order == Qt::AscendingOrder) ? (lhs->unit_discount < rhs->unit_discount) : (lhs->unit_discount > rhs->unit_discount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void SaleReferenceModel::ResetModel(const QJsonArray& entry_array)
{
    beginResetModel();

    ResourcePool<SaleReference>::Instance().Recycle(list_);

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* reference { ResourcePool<SaleReference>::Instance().Allocate() };
        reference->ReadJson(obj);

        list_.emplaceBack(reference);
    }

    endResetModel();
}
