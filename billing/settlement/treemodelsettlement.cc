#include "treemodelsettlement.h"

#include <QJsonArray>

#include "enum/settlementenum.h"
#include "global/resourcepool.h"
#include "utils/compareutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeModelSettlement::TreeModelSettlement(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

TreeModelSettlement::~TreeModelSettlement() { ResourcePool<Settlement>::Instance().Recycle(list_); }

QModelIndex TreeModelSettlement::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex TreeModelSettlement::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TreeModelSettlement::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int TreeModelSettlement::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.settlement_header.size();
}

QVariant TreeModelSettlement::data(const QModelIndex& index, int role) const
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
    case SettlementEnum::kVersion:
        return settlement->version;
    case SettlementEnum::kIssuedTime:
        return settlement->issued_time;
    case SettlementEnum::kDescription:
        return settlement->description;
    case SettlementEnum::kStatus:
        return std::to_underlying(settlement->status);
    case SettlementEnum::kAmount:
        return settlement->amount;
    case SettlementEnum::kPartner:
        return settlement->partner_id;
    default:
        return QVariant();
    }
}

QVariant TreeModelSettlement::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_header.at(section);

    return QVariant();
}

void TreeModelSettlement::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < info_.settlement_header.size());

    const SettlementEnum e_column { column };

    auto Compare = [e_column, order](const Settlement* lhs, const Settlement* rhs) -> bool {
        switch (e_column) {
        case SettlementEnum::kPartner:
            return Utils::CompareMember(lhs, rhs, &Settlement::partner_id, order);
        case SettlementEnum::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &Settlement::issued_time, order);
        case SettlementEnum::kDescription:
            return Utils::CompareMember(lhs, rhs, &Settlement::description, order);
        case SettlementEnum::kStatus:
            return Utils::CompareMember(lhs, rhs, &Settlement::status, order);
        case SettlementEnum::kAmount:
            return Utils::CompareMember(lhs, rhs, &Settlement::amount, order);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

bool TreeModelSettlement::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);
    auto* settlement { list_.takeAt(row) };
    endRemoveRows();

    QJsonObject message { JsonGen::SettlementRemoved(info_.section, settlement->id) };
    WebSocket::Instance()->SendMessage(kSettlementRemoved, message);

    ResourcePool<Settlement>::Instance().Recycle(settlement);
    return true;
}

bool TreeModelSettlement::InsertSucceeded(Settlement* settlement, const QJsonObject& meta)
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

void TreeModelSettlement::RecallSucceeded(const QUuid& settlement_id, const QJsonObject& update, const QJsonObject& meta)
{
    Q_ASSERT_X(meta.contains(kUpdatedBy), "SettlementModel::UpdateMeta", "Missing 'updated_by' in meta");
    Q_ASSERT_X(meta.contains(kUpdatedTime), "SettlementModel::UpdateMeta", "Missing 'updated_time' in meta");

    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    settlement->updated_time = QDateTime::fromString(meta[kUpdatedTime].toString(), Qt::ISODate);
    settlement->updated_by = QUuid(meta[kUpdatedBy].toString());
    settlement->ReadJson(update);
}

void TreeModelSettlement::UpdateSucceeded(const QUuid& settlement_id, const QJsonObject& update, const QJsonObject& meta)
{
    Q_ASSERT_X(meta.contains(kUpdatedBy), "SettlementModel::UpdateMeta", "Missing 'updated_by' in meta");
    Q_ASSERT_X(meta.contains(kUpdatedTime), "SettlementModel::UpdateMeta", "Missing 'updated_time' in meta");

    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    if (!settlement)
        return;

    settlement->updated_time = QDateTime::fromString(meta[kUpdatedTime].toString(), Qt::ISODate);
    settlement->updated_by = QUuid(meta[kUpdatedBy].toString());
    settlement->ReadJson(update);
}

void TreeModelSettlement::ResetModel(const QJsonArray& array)
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
