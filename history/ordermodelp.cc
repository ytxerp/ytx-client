#include "ordermodelp.h"

#include <QJsonArray>

#include "global/partner_inventory_registry.h"
#include "global/resourcepool.h"
#include "orderenum.h"
#include "utils/templateutils.h"

namespace history {

OrderModelP::OrderModelP(CSectionInfo& info, const QUuid& partner_id, TreeModel* tree_model_i, QObject* parent)
    : OrderModel { info, parent }
    , tree_model_i_ { tree_model_i }
    , partner_id_ { partner_id }
{
}

OrderModelP::~OrderModelP() { ResourcePool<OrderRow>::Instance().Recycle(list_); }

QVariant OrderModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const OrderColumnP column { index.column() };
    auto* entry { static_cast<OrderRow*>(index.internalPointer()) };

    switch (column) {
    case OrderColumnP::kIssuedTime:
        return entry->issued_time;
    case OrderColumnP::kInternalSku:
        return entry->node_id;
    case OrderColumnP::kCount:
        return entry->count;
    case OrderColumnP::kMeasure:
        return entry->measure;
    case OrderColumnP::kUnitPrice:
        return entry->unit_price;
    case OrderColumnP::kDescription:
        return entry->description;
    case OrderColumnP::kInitial:
        return entry->initial;
    case OrderColumnP::kColor:
        return tree_model_i_->Color(entry->node_id);
    case OrderColumnP::kExternalSku:
        return PartnerInventoryRegistry::Instance().ExternalSku(partner_id_, entry->node_id);
    }
}

void OrderModelP::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const OrderColumnP e_column { column };

    auto Compare = [e_column, order](const OrderRow* lhs, const OrderRow* rhs) -> bool {
        switch (e_column) {
        case OrderColumnP::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &OrderRow::issued_time, order);
        case OrderColumnP::kInternalSku:
            return utils::CompareMember(lhs, rhs, &OrderRow::node_id, order);
        case OrderColumnP::kUnitPrice:
            return utils::CompareMember(lhs, rhs, &OrderRow::unit_price, order);
        case OrderColumnP::kCount:
            return utils::CompareMember(lhs, rhs, &OrderRow::count, order);
        case OrderColumnP::kMeasure:
            return utils::CompareMember(lhs, rhs, &OrderRow::measure, order);
        case OrderColumnP::kDescription:
            return utils::CompareMember(lhs, rhs, &OrderRow::description, order);
        case OrderColumnP::kInitial:
            return utils::CompareMember(lhs, rhs, &OrderRow::initial, order);
        case OrderColumnP::kExternalSku:
        case OrderColumnP::kColor:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}
}
