#include "salesmodeli.h"

#include <QJsonArray>

#include "global/resourcepool.h"
#include "orderenum.h"
#include "utils/templateutils.h"

namespace history {

SalesModelI::SalesModelI(CSectionInfo& info, QObject* parent)
    : OrderModel { info, parent }
{
}

SalesModelI::~SalesModelI() { ResourcePool<OrderRow>::Instance().Recycle(list_); }

QVariant SalesModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SalesFieldI column { index.column() };
    auto* entry { static_cast<OrderRow*>(index.internalPointer()) };

    switch (column) {
    case SalesFieldI::kIssuedTime:
        return entry->issued_time;
    case SalesFieldI::kPartnerId:
        return entry->node_id;
    case SalesFieldI::kCount:
        return entry->count;
    case SalesFieldI::kMeasure:
        return entry->measure;
    case SalesFieldI::kUnitPrice:
        return entry->unit_price;
    case SalesFieldI::kDescription:
        return entry->description;
    case SalesFieldI::kInitial:
        return entry->initial;
    }
}

void SalesModelI::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_referenced_header.size() - 1)
        return;

    const SalesFieldI e_column { column };

    auto Compare = [e_column, order](const OrderRow* lhs, const OrderRow* rhs) -> bool {
        switch (e_column) {
        case SalesFieldI::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &OrderRow::issued_time, order);
        case SalesFieldI::kPartnerId:
            return utils::CompareMember(lhs, rhs, &OrderRow::node_id, order);
        case SalesFieldI::kUnitPrice:
            return utils::CompareMember(lhs, rhs, &OrderRow::unit_price, order);
        case SalesFieldI::kCount:
            return utils::CompareMember(lhs, rhs, &OrderRow::count, order);
        case SalesFieldI::kMeasure:
            return utils::CompareMember(lhs, rhs, &OrderRow::measure, order);
        case SalesFieldI::kDescription:
            return utils::CompareMember(lhs, rhs, &OrderRow::description, order);
        case SalesFieldI::kInitial:
            return utils::CompareMember(lhs, rhs, &OrderRow::initial, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}
}
