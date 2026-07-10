#include "partnerheatmodel.h"

#include <QJsonArray>

#include "global/resourcepool.h"
#include "partnerheatenum.h"
#include "utils/templateutils.h"

PartnerHeatModel::PartnerHeatModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
}

PartnerHeatModel::~PartnerHeatModel() { ResourcePool<PartnerHeatRow>::Instance().Recycle(list_); }

QVariant PartnerHeatModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

QModelIndex PartnerHeatModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant PartnerHeatModel::data(const QModelIndex& index, int role) const
{
    // Basic validation to prevent out-of-bounds access
    if (!index.isValid() || index.row() >= list_.size()) {
        return {};
    }

    // Only respond to display and edit roles
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    const PartnerHeatEnum column { index.column() };
    auto* entry { static_cast<PartnerHeatRow*>(index.internalPointer()) };

    switch (column) {
    case PartnerHeatEnum::kPartnerNode:
        return entry->partner_node.toString(QUuid::WithoutBraces);
    case PartnerHeatEnum::kOrderCount:
        return entry->order_count;
    case PartnerHeatEnum::kInventoryDiversity:
        return entry->inventory_diversity;
    case PartnerHeatEnum::kActiveMonths:
        return entry->active_months;
    case PartnerHeatEnum::kActiveDays:
        return entry->active_days;
    case PartnerHeatEnum::kTotalQuantity:
        return entry->total_quantity;
    case PartnerHeatEnum::kHeatScore:
        return entry->heat_score;
    case PartnerHeatEnum::kPlaceholder:
        return {};
    }
}

void PartnerHeatModel::sort(int column, Qt::SortOrder order)
{
    // Convert integer column to the structured enum using brace initialization
    const PartnerHeatEnum e_column { column };

    // Define a lambda for comparison based on the selected column and sort order
    auto Compare = [order, e_column](const PartnerHeatRow* lhs, const PartnerHeatRow* rhs) -> bool {
        switch (e_column) {
        case PartnerHeatEnum::kPartnerNode:
            return utils::CompareMember(lhs, rhs, &PartnerHeatRow::partner_node, order);
        case PartnerHeatEnum::kOrderCount:
            return utils::CompareMember(lhs, rhs, &PartnerHeatRow::order_count, order);
        case PartnerHeatEnum::kInventoryDiversity:
            return utils::CompareMember(lhs, rhs, &PartnerHeatRow::inventory_diversity, order);
        case PartnerHeatEnum::kActiveMonths:
            return utils::CompareMember(lhs, rhs, &PartnerHeatRow::active_months, order);
        case PartnerHeatEnum::kActiveDays:
            return utils::CompareMember(lhs, rhs, &PartnerHeatRow::active_days, order);
        case PartnerHeatEnum::kTotalQuantity:
            return utils::CompareMember(lhs, rhs, &PartnerHeatRow::total_quantity, order);
        case PartnerHeatEnum::kHeatScore:
            return utils::CompareMember(lhs, rhs, &PartnerHeatRow::heat_score, order);
        case PartnerHeatEnum::kPlaceholder:
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

void PartnerHeatModel::ResetModel(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty member array";
    }

    // Parse outside the reset block
    QList<PartnerHeatRow*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << Q_FUNC_INFO << "Invalid member data, expected object:" << value;
            continue;
        }

        auto* member { ResourcePool<PartnerHeatRow>::Instance().Allocate() };
        member->ReadJson(value.toObject());
        new_list.emplaceBack(member);
    }

    // Keep reset block as short as possible
    beginResetModel();
    ResourcePool<PartnerHeatRow>::Instance().Recycle(list_);
    list_ = std::move(new_list);
    sort(std::to_underlying(PartnerHeatEnum::kHeatScore), Qt::DescendingOrder);
    endResetModel();
}
