#include "salereferencepmodel.h"

#include <QJsonArray>

#include "enum/reference.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

SaleReferencePModel::SaleReferencePModel(CSectionInfo& info, TreeModel* tree_model_i, EntryHub* entry_hub_p, QObject* parent)
    : SaleReferenceModel { info, parent }
    , tree_model_i_ { tree_model_i }
    , entry_hub_p_ { static_cast<EntryHubP*>(entry_hub_p) }
{
}

SaleReferencePModel::~SaleReferencePModel() { ResourcePool<SaleReference>::Instance().Recycle(list_); }

QVariant SaleReferencePModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* sale_reference { list_.at(index.row()) };
    const SaleReferencePEnum column { index.column() };

    switch (column) {
    case SaleReferencePEnum::kIssuedTime:
        return sale_reference->issued_time;
    case SaleReferencePEnum::kInternalSku:
        return sale_reference->node_id;
    case SaleReferencePEnum::kOrderId:
        return sale_reference->order_id;
    case SaleReferencePEnum::kCount:
        return sale_reference->count;
    case SaleReferencePEnum::kMeasure:
        return sale_reference->measure;
    case SaleReferencePEnum::kUnitPrice:
        return sale_reference->unit_price;
    case SaleReferencePEnum::kDescription:
        return sale_reference->description;
    case SaleReferencePEnum::kInitial:
        return sale_reference->initial;
    case SaleReferencePEnum::kColor:
        return tree_model_i_->Color(sale_reference->node_id);
    case SaleReferencePEnum::kExternalSku:
        return entry_hub_p_->ExternalSku(sale_reference->order_id, sale_reference->node_id);
    }
}

void SaleReferencePModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const SaleReferencePEnum e_column { column };

    auto Compare = [e_column, order](const SaleReference* lhs, const SaleReference* rhs) -> bool {
        switch (e_column) {
        case SaleReferencePEnum::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &SaleReference::issued_time, order);
        case SaleReferencePEnum::kInternalSku:
            return Utils::CompareMember(lhs, rhs, &SaleReference::node_id, order);
        case SaleReferencePEnum::kUnitPrice:
            return Utils::CompareMember(lhs, rhs, &SaleReference::unit_price, order);
        case SaleReferencePEnum::kCount:
            return Utils::CompareMember(lhs, rhs, &SaleReference::count, order);
        case SaleReferencePEnum::kMeasure:
            return Utils::CompareMember(lhs, rhs, &SaleReference::measure, order);
        case SaleReferencePEnum::kDescription:
            return Utils::CompareMember(lhs, rhs, &SaleReference::description, order);
        case SaleReferencePEnum::kInitial:
            return Utils::CompareMember(lhs, rhs, &SaleReference::initial, order);
        case SaleReferencePEnum::kOrderId:
        case SaleReferencePEnum::kExternalSku:
        case SaleReferencePEnum::kColor:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
