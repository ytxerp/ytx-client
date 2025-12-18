#include "settlementmodel.h"

#include <QJsonArray>

#include "enum/settlementenum.h"
#include "global/resourcepool.h"

SettlementModel::SettlementModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

SettlementModel::~SettlementModel() { ResourcePool<Settlement>::Instance().Recycle(list_); }

QModelIndex SettlementModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
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
    auto* settlement { static_cast<Settlement*>(index.internalPointer()) };

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
        return settlement->partner_id;
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
            return (order == Qt::AscendingOrder) ? (lhs->partner_id < rhs->partner_id) : (lhs->partner_id > rhs->partner_id);
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
    auto* settlement { list_.takeAt(row) };
    ResourcePool<Settlement>::Instance().Recycle(settlement);
    endRemoveRows();

    return true;
}

bool SettlementModel::InsertSucceeded(Settlement* settlement, const QJsonObject& meta)
{
    Q_ASSERT_X(meta.contains(kUserId), "SettlementModel::InsertMeta", "Missing 'user_id' in meta");
    Q_ASSERT_X(meta.contains(kCreatedTime), "SettlementModel::InsertMeta", "Missing 'created_time' in meta");
    Q_ASSERT_X(meta.contains(kCreatedBy), "SettlementModel::InsertMeta", "Missing 'created_by' in meta");

    settlement->user_id = QUuid(meta[kUserId].toString());
    settlement->created_time = QDateTime::fromString(meta[kCreatedTime].toString(), Qt::ISODate);
    settlement->created_by = QUuid(meta[kCreatedBy].toString());

    const int row { rowCount() };

    beginInsertRows(QModelIndex(), row, row);
    list_.emplaceBack(settlement);
    endInsertRows();

    return true;
}

void SettlementModel::RecallSucceeded(const QUuid& settlement_id, const QJsonObject& meta)
{
    Q_ASSERT_X(meta.contains(kUpdatedBy), "SettlementModel::UpdateMeta", "Missing 'updated_by' in meta");
    Q_ASSERT_X(meta.contains(kUpdatedTime), "SettlementModel::UpdateMeta", "Missing 'updated_time' in meta");

    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    settlement->updated_time = QDateTime::fromString(meta[kUpdatedTime].toString(), Qt::ISODate);
    settlement->updated_by = QUuid(meta[kUpdatedBy].toString());
    settlement->status = std::to_underlying(SettlementStatus::kRecalled);
}

void SettlementModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<Settlement>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* settlement { ResourcePool<Settlement>::Instance().Allocate() };
        settlement->ReadJson(obj);

        list_.emplaceBack(settlement);
    }

    sort(static_cast<int>(SettlementEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
