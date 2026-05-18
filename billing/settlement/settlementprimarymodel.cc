#include "settlementprimarymodel.h"

#include <QJsonArray>

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "settlementenum.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementPrimaryModel::SettlementPrimaryModel(const QStringList& header, Section section, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , section_ { section }
{
}

SettlementPrimaryModel::~SettlementPrimaryModel() { ResourcePool<SettlementPrimary>::Instance().Recycle(list_); }

QModelIndex SettlementPrimaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex SettlementPrimaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementPrimaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SettlementPrimaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant SettlementPrimaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SettlementPrimaryEnum column { index.column() };
    auto* settlement { static_cast<SettlementPrimary*>(index.internalPointer()) };

    switch (column) {
    case SettlementPrimaryEnum::kId:
        return settlement->id;
    case SettlementPrimaryEnum::kVersion:
        return settlement->version;
    case SettlementPrimaryEnum::kIssuedTime:
        return settlement->issued_time;
    case SettlementPrimaryEnum::kDescription:
        return settlement->description;
    case SettlementPrimaryEnum::kStatus:
        return std::to_underlying(settlement->status);
    case SettlementPrimaryEnum::kAmount:
        return settlement->amount;
    case SettlementPrimaryEnum::kPartner:
        return settlement->partner_id;
    }
}

QVariant SettlementPrimaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void SettlementPrimaryModel::sort(int column, Qt::SortOrder order)
{
    const SettlementPrimaryEnum e_column { column };

    auto Compare = [e_column, order](const SettlementPrimary* lhs, const SettlementPrimary* rhs) -> bool {
        switch (e_column) {
        case SettlementPrimaryEnum::kPartner:
            return utils::CompareMember(lhs, rhs, &SettlementPrimary::partner_id, order);
        case SettlementPrimaryEnum::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &SettlementPrimary::issued_time, order);
        case SettlementPrimaryEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &SettlementPrimary::description, order);
        case SettlementPrimaryEnum::kStatus:
            return utils::CompareMember(lhs, rhs, &SettlementPrimary::status, order);
        case SettlementPrimaryEnum::kAmount:
            return utils::CompareMember(lhs, rhs, &SettlementPrimary::amount, order);
        case SettlementPrimaryEnum::kId:
        case SettlementPrimaryEnum::kVersion:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

bool SettlementPrimaryModel::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);
    auto* settlement { list_.takeAt(row) };
    endRemoveRows();

    QJsonObject message { JsonGen::SettlementDelete(section_, settlement->id, settlement->version) };
    WebSocket::Instance()->SendMessage(WsKey::kSettlementDelete, message);

    ResourcePool<SettlementPrimary>::Instance().Recycle(settlement);
    return true;
}

bool SettlementPrimaryModel::InsertSucceeded(SettlementPrimary* settlement)
{
    const int row { rowCount() };

    beginInsertRows(QModelIndex(), row, row);
    list_.emplaceBack(settlement);
    endInsertRows();

    return true;
}

void SettlementPrimaryModel::RecallSucceeded(const QUuid& settlement_id, const QJsonObject& update)
{
    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    if (!settlement)
        return;

    settlement->ReadJson(update);
    settlement->amount = 0.0;
}

void SettlementPrimaryModel::UpdateSucceeded(const QUuid& settlement_id, const QJsonObject& update)
{
    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    if (!settlement)
        return;

    settlement->ReadJson(update);
}

void SettlementPrimaryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<SettlementPrimary>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* settlement { ResourcePool<SettlementPrimary>::Instance().Allocate() };
        settlement->ReadJson(obj);

        list_.emplaceBack(settlement);
    }

    sort(static_cast<int>(SettlementPrimaryEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
