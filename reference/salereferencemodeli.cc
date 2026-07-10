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

    const SaleReferenceEnumI column { index.column() };
    auto* entry { static_cast<OrderReference*>(index.internalPointer()) };

    switch (column) {
    case SaleReferenceEnumI::kIssuedTime:
        return entry->issued_time;
    case SaleReferenceEnumI::kPartnerId:
        return entry->node_id;
    case SaleReferenceEnumI::kOrderId:
        return entry->order_id;
    case SaleReferenceEnumI::kCount:
        return entry->count;
    case SaleReferenceEnumI::kMeasure:
        return entry->measure;
    case SaleReferenceEnumI::kUnitPrice:
        return entry->unit_price;
    case SaleReferenceEnumI::kDescription:
        return entry->description;
    case SaleReferenceEnumI::kInitial:
        return entry->initial;
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
            return utils::CompareMember(lhs, rhs, &OrderReference::issued_time, order);
        case SaleReferenceEnumI::kPartnerId:
            return utils::CompareMember(lhs, rhs, &OrderReference::node_id, order);
        case SaleReferenceEnumI::kUnitPrice:
            return utils::CompareMember(lhs, rhs, &OrderReference::unit_price, order);
        case SaleReferenceEnumI::kCount:
            return utils::CompareMember(lhs, rhs, &OrderReference::count, order);
        case SaleReferenceEnumI::kMeasure:
            return utils::CompareMember(lhs, rhs, &OrderReference::measure, order);
        case SaleReferenceEnumI::kDescription:
            return utils::CompareMember(lhs, rhs, &OrderReference::description, order);
        case SaleReferenceEnumI::kInitial:
            return utils::CompareMember(lhs, rhs, &OrderReference::initial, order);
        case SaleReferenceEnumI::kOrderId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
