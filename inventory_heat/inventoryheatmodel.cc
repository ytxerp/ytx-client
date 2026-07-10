#include "inventoryheatmodel.h"

#include <QJsonArray>

#include "global/resourcepool.h"
#include "inventoryheatenum.h"
#include "utils/templateutils.h"

InventoryHeatModel::InventoryHeatModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
}

InventoryHeatModel::~InventoryHeatModel() { ResourcePool<InventoryHeatRow>::Instance().Recycle(list_); }

QVariant InventoryHeatModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

QModelIndex InventoryHeatModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant InventoryHeatModel::data(const QModelIndex& index, int role) const
{
    // Basic validation to prevent out-of-bounds access
    if (!index.isValid() || index.row() >= list_.size()) {
        return {};
    }

    // Only respond to display and edit roles
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    const auto column { static_cast<InventoryHeatEnum>(index.column()) };
    auto* entry { static_cast<InventoryHeatRow*>(index.internalPointer()) };

    switch (column) {
    case InventoryHeatEnum::kInventoryNode:
        return entry->inventory_node.toString(QUuid::WithoutBraces);
    case InventoryHeatEnum::kOrderCount:
        return entry->order_count;
    case InventoryHeatEnum::kPartnerCount:
        return entry->partner_count;
    case InventoryHeatEnum::kActiveMonths:
        return entry->active_months;
    case InventoryHeatEnum::kActiveDays:
        return entry->active_days;
    case InventoryHeatEnum::kTotalQuantity:
        return entry->total_quantity;
    case InventoryHeatEnum::kHeatScore:
        return entry->heat_score;
    case InventoryHeatEnum::kPlaceholder:
        return {};
    }
}

void InventoryHeatModel::sort(int column, Qt::SortOrder order)
{ // Convert integer column to the structured enum using brace initialization
    const InventoryHeatEnum e_column { static_cast<InventoryHeatEnum>(column) };

    // Define a lambda for comparison based on the selected column and sort order
    auto Compare = [order, e_column](const InventoryHeatRow* lhs, const InventoryHeatRow* rhs) -> bool {
        switch (e_column) {
        case InventoryHeatEnum::kInventoryNode:
            return utils::CompareMember(lhs, rhs, &InventoryHeatRow::inventory_node, order);
        case InventoryHeatEnum::kOrderCount:
            return utils::CompareMember(lhs, rhs, &InventoryHeatRow::order_count, order);
        case InventoryHeatEnum::kPartnerCount:
            return utils::CompareMember(lhs, rhs, &InventoryHeatRow::partner_count, order);
        case InventoryHeatEnum::kActiveMonths:
            return utils::CompareMember(lhs, rhs, &InventoryHeatRow::active_months, order);
        case InventoryHeatEnum::kActiveDays:
            return utils::CompareMember(lhs, rhs, &InventoryHeatRow::active_days, order);
        case InventoryHeatEnum::kTotalQuantity:
            return utils::CompareMember(lhs, rhs, &InventoryHeatRow::total_quantity, order);
        case InventoryHeatEnum::kHeatScore:
            return utils::CompareMember(lhs, rhs, &InventoryHeatRow::heat_score, order);
        case InventoryHeatEnum::kPlaceholder:
            return false;
        }
        return false;
    };

    // Notify the view that the layout is about to change
    emit layoutAboutToBeChanged();

    // Perform the sort on the underlying data list
    std::sort(list_.begin(), list_.end(), Compare);

    // Notify the view that the layout has been updated
    emit layoutChanged();
}

void InventoryHeatModel::ResetModel(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty member array";
    }

    // Parse outside the reset block
    QList<InventoryHeatRow*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << Q_FUNC_INFO << "Invalid member data, expected object:" << value;
            continue;
        }

        auto* member { ResourcePool<InventoryHeatRow>::Instance().Allocate() };
        member->ReadJson(value.toObject());
        new_list.emplaceBack(member);
    }

    // Keep reset block as short as possible
    beginResetModel();
    ResourcePool<InventoryHeatRow>::Instance().Recycle(list_);
    list_ = std::move(new_list);
    sort(std::to_underlying(InventoryHeatEnum::kHeatScore), Qt::DescendingOrder);
    endResetModel();
}
