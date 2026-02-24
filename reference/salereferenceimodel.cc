#include "salereferenceimodel.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

SaleReferenceIModel::SaleReferenceIModel(CSectionInfo& info, QObject* parent)
    : SaleReferenceModel { info, parent }
{
}

SaleReferenceIModel::~SaleReferenceIModel() { ResourcePool<SaleReference>::Instance().Recycle(list_); }

QVariant SaleReferenceIModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* sale_reference { list_.at(index.row()) };
    const SaleReferenceIEnum column { index.column() };

    switch (column) {
    case SaleReferenceIEnum::kIssuedTime:
        return sale_reference->issued_time;
    case SaleReferenceIEnum::kPartnerId:
        return sale_reference->node_id;
    case SaleReferenceIEnum::kOrderId:
        return sale_reference->order_id;
    case SaleReferenceIEnum::kCount:
        return sale_reference->count;
    case SaleReferenceIEnum::kMeasure:
        return sale_reference->measure;
    case SaleReferenceIEnum::kUnitPrice:
        return sale_reference->unit_price;
    case SaleReferenceIEnum::kDescription:
        return sale_reference->description;
    case SaleReferenceIEnum::kInitial:
        return sale_reference->initial;
    default:
        return QVariant();
    }
}

void SaleReferenceIModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const SaleReferenceIEnum e_column { column };

    auto Compare = [e_column, order](const SaleReference* lhs, const SaleReference* rhs) -> bool {
        switch (e_column) {
        case SaleReferenceIEnum::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &SaleReference::issued_time, order);
        case SaleReferenceIEnum::kPartnerId:
            return Utils::CompareMember(lhs, rhs, &SaleReference::node_id, order);
        case SaleReferenceIEnum::kUnitPrice:
            return Utils::CompareMember(lhs, rhs, &SaleReference::unit_price, order);
        case SaleReferenceIEnum::kCount:
            return Utils::CompareMember(lhs, rhs, &SaleReference::count, order);
        case SaleReferenceIEnum::kMeasure:
            return Utils::CompareMember(lhs, rhs, &SaleReference::measure, order);
        case SaleReferenceIEnum::kDescription:
            return Utils::CompareMember(lhs, rhs, &SaleReference::description, order);
        case SaleReferenceIEnum::kInitial:
            return Utils::CompareMember(lhs, rhs, &SaleReference::initial, order);
        case SaleReferenceIEnum::kOrderId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
