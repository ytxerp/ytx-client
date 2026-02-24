#include "salereferencemodeli.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

SaleReferenceModelI::SaleReferenceModelI(CSectionInfo& info, QObject* parent)
    : SaleReferenceModel { info, parent }
{
}

SaleReferenceModelI::~SaleReferenceModelI() { ResourcePool<SaleReference>::Instance().Recycle(list_); }

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

    auto Compare = [e_column, order](const SaleReference* lhs, const SaleReference* rhs) -> bool {
        switch (e_column) {
        case SaleReferenceEnumI::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &SaleReference::issued_time, order);
        case SaleReferenceEnumI::kPartnerId:
            return Utils::CompareMember(lhs, rhs, &SaleReference::node_id, order);
        case SaleReferenceEnumI::kUnitPrice:
            return Utils::CompareMember(lhs, rhs, &SaleReference::unit_price, order);
        case SaleReferenceEnumI::kCount:
            return Utils::CompareMember(lhs, rhs, &SaleReference::count, order);
        case SaleReferenceEnumI::kMeasure:
            return Utils::CompareMember(lhs, rhs, &SaleReference::measure, order);
        case SaleReferenceEnumI::kDescription:
            return Utils::CompareMember(lhs, rhs, &SaleReference::description, order);
        case SaleReferenceEnumI::kInitial:
            return Utils::CompareMember(lhs, rhs, &SaleReference::initial, order);
        case SaleReferenceEnumI::kOrderId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
