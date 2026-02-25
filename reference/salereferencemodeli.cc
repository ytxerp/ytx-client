#include "salereferencemodeli.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

SaleReferenceModelI::SaleReferenceModelI(CSectionInfo& info, QObject* parent)
    : OrderReferenceModel { info, parent }
{
}

SaleReferenceModelI::~SaleReferenceModelI() { ResourcePool<OrderReference>::Instance().Recycle(list_); }

QVariant SaleReferenceModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* sale_reference { list_.at(index.row()) };
    const SaleReferenceEnumI column { index.column() };

    switch (column) {
    case SaleReferenceEnumI::kIssuedTime:
        return sale_reference->issued_time;
    case SaleReferenceEnumI::kPartnerId:
        return sale_reference->node_id;
    case SaleReferenceEnumI::kOrderId:
        return sale_reference->order_id;
    case SaleReferenceEnumI::kCount:
        return sale_reference->count;
    case SaleReferenceEnumI::kMeasure:
        return sale_reference->measure;
    case SaleReferenceEnumI::kUnitPrice:
        return sale_reference->unit_price;
    case SaleReferenceEnumI::kDescription:
        return sale_reference->description;
    case SaleReferenceEnumI::kInitial:
        return sale_reference->initial;
    default:
        return QVariant();
    }
}

void SaleReferenceModelI::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const SaleReferenceEnumI e_column { column };

    auto Compare = [e_column, order](const OrderReference* lhs, const OrderReference* rhs) -> bool {
        switch (e_column) {
        case SaleReferenceEnumI::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &OrderReference::issued_time, order);
        case SaleReferenceEnumI::kPartnerId:
            return Utils::CompareMember(lhs, rhs, &OrderReference::node_id, order);
        case SaleReferenceEnumI::kUnitPrice:
            return Utils::CompareMember(lhs, rhs, &OrderReference::unit_price, order);
        case SaleReferenceEnumI::kCount:
            return Utils::CompareMember(lhs, rhs, &OrderReference::count, order);
        case SaleReferenceEnumI::kMeasure:
            return Utils::CompareMember(lhs, rhs, &OrderReference::measure, order);
        case SaleReferenceEnumI::kDescription:
            return Utils::CompareMember(lhs, rhs, &OrderReference::description, order);
        case SaleReferenceEnumI::kInitial:
            return Utils::CompareMember(lhs, rhs, &OrderReference::initial, order);
        case SaleReferenceEnumI::kOrderId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
