#include "settlementmodel.h"

#include <QJsonArray>

#include "enum/settlementenum.h"
#include "global/resourcepool.h"

SettlementModel::SettlementModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

SettlementModel::~SettlementModel() { }

QModelIndex SettlementModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row).get());
}

QModelIndex SettlementModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SettlementModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.settlement_header.size();
}

QVariant SettlementModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SettlementEnum column { index.column() };
    auto& settlement { list_.at(index.row()) };

    switch (column) {
    case SettlementEnum::kId:
        return settlement->id;
    case SettlementEnum::kUserId:
        return settlement->user_id;
    case SettlementEnum::kCreateTime:
        return settlement->created_time;
    case SettlementEnum::kCreateBy:
        return settlement->created_by;
    case SettlementEnum::kUpdateTime:
        return settlement->updated_time;
    case SettlementEnum::kUpdateBy:
        return settlement->updated_by;
    case SettlementEnum::kIssuedTime:
        return settlement->issued_time;
    case SettlementEnum::kDescription:
        return settlement->description;
    case SettlementEnum::kStatus:
        return settlement->status;
    case SettlementEnum::kAmount:
        return settlement->amount;
    case SettlementEnum::kPartner:
        return settlement->partner;
    default:
        return QVariant();
    }
}

QVariant SettlementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_header.at(section);

    return QVariant();
}

void SettlementModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.settlement_header.size())
        return;

    auto Compare = [column, order](const auto& lhs, const auto& rhs) -> bool {
        const SettlementEnum e_column { column };

        switch (e_column) {
        case SettlementEnum::kPartner:
            return (order == Qt::AscendingOrder) ? (lhs->partner < rhs->partner) : (lhs->partner > rhs->partner);
        case SettlementEnum::kUserId:
            return (order == Qt::AscendingOrder) ? (lhs->user_id < rhs->user_id) : (lhs->user_id > rhs->user_id);
        case SettlementEnum::kCreateTime:
            return (order == Qt::AscendingOrder) ? (lhs->created_time < rhs->created_time) : (lhs->created_time > rhs->created_time);
        case SettlementEnum::kCreateBy:
            return (order == Qt::AscendingOrder) ? (lhs->created_by < rhs->created_by) : (lhs->created_by > rhs->created_by);
        case SettlementEnum::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (lhs->updated_time < rhs->updated_time) : (lhs->updated_time > rhs->updated_time);
        case SettlementEnum::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (lhs->updated_by < rhs->updated_by) : (lhs->updated_by > rhs->updated_by);
        case SettlementEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case SettlementEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case SettlementEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        case SettlementEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

bool SettlementModel::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);
    auto settlement { list_.takeAt(row) };
    endRemoveRows();

    return true;
}

bool SettlementModel::InsertNode(int row, const QModelIndex& parent, Settlement* node)
{
    beginInsertRows(parent, row, row);
    list_.emplaceBack(node);
    endInsertRows();

    return true;
}

void SettlementModel::ResetModel(const QJsonArray& entry_array)
{
    beginResetModel();

    list_.clear();

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        // Allocate a Settlement and wrap it in a shared_ptr with the deleter
        std::shared_ptr<Settlement> settlement(ResourcePool<Settlement>::Instance().Allocate(), kSettlementDeleter);

        settlement->ReadJson(obj);

        list_.emplaceBack(settlement);
    }

    sort(static_cast<int>(SettlementEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
