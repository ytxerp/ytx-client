#include "treemodelsettlement.h"

#include <QJsonArray>

#include "component/constantwebsocket.h"
#include "enum/settlementenum.h"
#include "global/resourcepool.h"
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
    const SettlementEnum e_column { column };

    auto Compare = [e_column, order](const Settlement* lhs, const Settlement* rhs) -> bool {
        switch (e_column) {
        case SettlementEnum::kPartner:
            return utils::CompareMember(lhs, rhs, &Settlement::partner_id, order);
        case SettlementEnum::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &Settlement::issued_time, order);
        case SettlementEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &Settlement::description, order);
        case SettlementEnum::kStatus:
            return utils::CompareMember(lhs, rhs, &Settlement::status, order);
        case SettlementEnum::kAmount:
            return utils::CompareMember(lhs, rhs, &Settlement::amount, order);
        case SettlementEnum::kId:
        case SettlementEnum::kVersion:
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

    QJsonObject message { JsonGen::SettlementDelete(info_.section, settlement->id, settlement->version) };
    WebSocket::Instance()->SendMessage(WsKey::kSettlementDelete, message);

    ResourcePool<Settlement>::Instance().Recycle(settlement);
    return true;
}

bool TreeModelSettlement::InsertSucceeded(Settlement* settlement)
{
    const int row { rowCount() };

    beginInsertRows(QModelIndex(), row, row);
    list_.emplaceBack(settlement);
    endInsertRows();

    return true;
}

void TreeModelSettlement::RecallSucceeded(const QUuid& settlement_id, const QJsonObject& update)
{
    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    if (!settlement)
        return;

    settlement->ReadJson(update);
    settlement->amount = 0.0;
}

void TreeModelSettlement::UpdateSucceeded(const QUuid& settlement_id, const QJsonObject& update)
{
    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    if (!settlement)
        return;

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
