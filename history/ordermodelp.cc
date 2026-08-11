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

    const OrderFieldP column { index.column() };
    auto* entry { static_cast<OrderRow*>(index.internalPointer()) };

    switch (column) {
    case OrderFieldP::kIssuedTime:
        return entry->issued_time;
    case OrderFieldP::kInternalSku:
        return entry->node_id;
    case OrderFieldP::kCount:
        return entry->count;
    case OrderFieldP::kMeasure:
        return entry->measure;
    case OrderFieldP::kUnitPrice:
        return entry->unit_price;
    case OrderFieldP::kDescription:
        return entry->description;
    case OrderFieldP::kInitial:
        return entry->initial;
    case OrderFieldP::kColor:
        return tree_model_i_->Color(entry->node_id);
    case OrderFieldP::kExternalSku:
        return PartnerInventoryRegistry::Instance().ExternalSku(partner_id_, entry->node_id);
    }
}

void OrderModelP::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const OrderFieldP e_column { column };

    auto Compare = [e_column, order](const OrderRow* lhs, const OrderRow* rhs) -> bool {
        switch (e_column) {
        case OrderFieldP::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &OrderRow::issued_time, order);
        case OrderFieldP::kInternalSku:
            return utils::CompareMember(lhs, rhs, &OrderRow::node_id, order);
        case OrderFieldP::kUnitPrice:
            return utils::CompareMember(lhs, rhs, &OrderRow::unit_price, order);
        case OrderFieldP::kCount:
            return utils::CompareMember(lhs, rhs, &OrderRow::count, order);
        case OrderFieldP::kMeasure:
            return utils::CompareMember(lhs, rhs, &OrderRow::measure, order);
        case OrderFieldP::kDescription:
            return utils::CompareMember(lhs, rhs, &OrderRow::description, order);
        case OrderFieldP::kInitial:
            return utils::CompareMember(lhs, rhs, &OrderRow::initial, order);
        case OrderFieldP::kExternalSku:
        case OrderFieldP::kColor:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}
}
