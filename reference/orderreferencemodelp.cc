#include "orderreferencemodelp.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

OrderReferenceModelP::OrderReferenceModelP(CSectionInfo& info, const QUuid& partner_id, TreeModel* tree_model_i, EntryHub* entry_hub_p, QObject* parent)
    : OrderReferenceModel { info, parent }
    , tree_model_i_ { tree_model_i }
    , entry_hub_p_ { static_cast<EntryHubP*>(entry_hub_p) }
    , partner_id_ { partner_id }
{
}

OrderReferenceModelP::~OrderReferenceModelP() { ResourcePool<OrderReference>::Instance().Recycle(list_); }

QVariant OrderReferenceModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* sale_reference { list_.at(index.row()) };
    const SaleReferenceEnumP column { index.column() };

    switch (column) {
    case SaleReferenceEnumP::kIssuedTime:
        return sale_reference->issued_time;
    case SaleReferenceEnumP::kInternalSku:
        return sale_reference->node_id;
    case SaleReferenceEnumP::kOrderId:
        return sale_reference->order_id;
    case SaleReferenceEnumP::kCount:
        return sale_reference->count;
    case SaleReferenceEnumP::kMeasure:
        return sale_reference->measure;
    case SaleReferenceEnumP::kUnitPrice:
        return sale_reference->unit_price;
    case SaleReferenceEnumP::kDescription:
        return sale_reference->description;
    case SaleReferenceEnumP::kInitial:
        return sale_reference->initial;
    case SaleReferenceEnumP::kColor:
        return tree_model_i_->Color(sale_reference->node_id);
    case SaleReferenceEnumP::kExternalSku:
        return entry_hub_p_->ExternalSku(partner_id_, sale_reference->node_id);
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
