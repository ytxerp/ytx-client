#include "orderreferencemodelp.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/partner_inventory_registry.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

OrderReferenceModelP::OrderReferenceModelP(CSectionInfo& info, const QUuid& partner_id, TreeModel* tree_model_i, QObject* parent)
    : OrderReferenceModel { info, parent }
    , tree_model_i_ { tree_model_i }
    , partner_id_ { partner_id }
{
}

OrderReferenceModelP::~OrderReferenceModelP() { ResourcePool<OrderReference>::Instance().Recycle(list_); }

QVariant OrderReferenceModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SaleReferenceEnumP column { index.column() };
    auto* entry { static_cast<OrderReference*>(index.internalPointer()) };

    switch (column) {
    case SaleReferenceEnumP::kIssuedTime:
        return entry->issued_time;
    case SaleReferenceEnumP::kInternalSku:
        return entry->node_id;
    case SaleReferenceEnumP::kOrderId:
        return entry->order_id;
    case SaleReferenceEnumP::kCount:
        return entry->count;
    case SaleReferenceEnumP::kMeasure:
        return entry->measure;
    case SaleReferenceEnumP::kUnitPrice:
        return entry->unit_price;
    case SaleReferenceEnumP::kDescription:
        return entry->description;
    case SaleReferenceEnumP::kInitial:
        return entry->initial;
    case SaleReferenceEnumP::kColor:
        return tree_model_i_->Color(entry->node_id);
    case SaleReferenceEnumP::kExternalSku:
        return PartnerInventoryRegistry::Instance().ExternalSku(partner_id_, entry->node_id);
    }
}

void OrderReferenceModelP::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const SaleReferenceEnumP e_column { column };

    auto Compare = [e_column, order](const OrderReference* lhs, const OrderReference* rhs) -> bool {
        switch (e_column) {
        case SaleReferenceEnumP::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &OrderReference::issued_time, order);
        case SaleReferenceEnumP::kInternalSku:
            return utils::CompareMember(lhs, rhs, &OrderReference::node_id, order);
        case SaleReferenceEnumP::kUnitPrice:
            return utils::CompareMember(lhs, rhs, &OrderReference::unit_price, order);
        case SaleReferenceEnumP::kCount:
            return utils::CompareMember(lhs, rhs, &OrderReference::count, order);
        case SaleReferenceEnumP::kMeasure:
            return utils::CompareMember(lhs, rhs, &OrderReference::measure, order);
        case SaleReferenceEnumP::kDescription:
            return utils::CompareMember(lhs, rhs, &OrderReference::description, order);
        case SaleReferenceEnumP::kInitial:
            return utils::CompareMember(lhs, rhs, &OrderReference::initial, order);
        case SaleReferenceEnumP::kOrderId:
        case SaleReferenceEnumP::kExternalSku:
        case SaleReferenceEnumP::kColor:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
