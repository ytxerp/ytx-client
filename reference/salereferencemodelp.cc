#include "salereferencemodelp.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

SaleReferenceModelP::SaleReferenceModelP(CSectionInfo& info, const QUuid& partner_id, TreeModel* tree_model_i, EntryHub* entry_hub_p, QObject* parent)
    : SaleReferenceModel { info, parent }
    , tree_model_i_ { tree_model_i }
    , entry_hub_p_ { static_cast<EntryHubP*>(entry_hub_p) }
    , partner_id_ { partner_id }
{
}

SaleReferenceModelP::~SaleReferenceModelP() { ResourcePool<SaleReference>::Instance().Recycle(list_); }

QVariant SaleReferenceModelP::data(const QModelIndex& index, int role) const
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

void SaleReferenceModelP::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const SaleReferenceEnumP e_column { column };

    auto Compare = [e_column, order](const SaleReference* lhs, const SaleReference* rhs) -> bool {
        switch (e_column) {
        case SaleReferenceEnumP::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &SaleReference::issued_time, order);
        case SaleReferenceEnumP::kInternalSku:
            return Utils::CompareMember(lhs, rhs, &SaleReference::node_id, order);
        case SaleReferenceEnumP::kUnitPrice:
            return Utils::CompareMember(lhs, rhs, &SaleReference::unit_price, order);
        case SaleReferenceEnumP::kCount:
            return Utils::CompareMember(lhs, rhs, &SaleReference::count, order);
        case SaleReferenceEnumP::kMeasure:
            return Utils::CompareMember(lhs, rhs, &SaleReference::measure, order);
        case SaleReferenceEnumP::kDescription:
            return Utils::CompareMember(lhs, rhs, &SaleReference::description, order);
        case SaleReferenceEnumP::kInitial:
            return Utils::CompareMember(lhs, rhs, &SaleReference::initial, order);
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
