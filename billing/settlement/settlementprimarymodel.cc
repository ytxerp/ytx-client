#include "settlementprimarymodel.h"

#include <QJsonArray>

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "settlementenum.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

namespace settlement {

PrimaryModel::PrimaryModel(const QStringList& header, Section section, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , section_ { section }
{
}

PrimaryModel::~PrimaryModel() { ResourcePool<PrimaryRow>::Instance().Recycle(list_); }

QModelIndex PrimaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex PrimaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int PrimaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int PrimaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant PrimaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const PrimaryField column { index.column() };
    auto* settlement { static_cast<PrimaryRow*>(index.internalPointer()) };

    switch (column) {
    case PrimaryField::kId:
        return settlement->id;
    case PrimaryField::kVersion:
        return settlement->version;
    case PrimaryField::kIssuedTime:
        return settlement->issued_time;
    case PrimaryField::kDescription:
        return settlement->description;
    case PrimaryField::kStatus:
        return std::to_underlying(settlement->status);
    case PrimaryField::kAmount:
        return settlement->amount;
    case PrimaryField::kPartner:
        return settlement->partner_id;
    }
}

QVariant PrimaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void PrimaryModel::sort(int column, Qt::SortOrder order)
{
    const PrimaryField e_column { column };

    auto Compare = [e_column, order](const PrimaryRow* lhs, const PrimaryRow* rhs) -> bool {
        switch (e_column) {
        case PrimaryField::kPartner:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::partner_id, order);
        case PrimaryField::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::issued_time, order);
        case PrimaryField::kDescription:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::description, order);
        case PrimaryField::kStatus:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::status, order);
        case PrimaryField::kAmount:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::amount, order);
        case PrimaryField::kId:
        case PrimaryField::kVersion:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

bool PrimaryModel::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);
    auto* settlement { list_.takeAt(row) };
    endRemoveRows();

    QJsonObject message { JsonGen::SettlementDelete(section_, settlement->id, settlement->version) };
    WebSocket::Instance()->SendMessage(WsKey::kSettlementDelete, message);

    ResourcePool<PrimaryRow>::Instance().Recycle(settlement);
    return true;
}

bool PrimaryModel::InsertSucceeded(PrimaryRow* settlement)
{
    const int row { rowCount() };

    beginInsertRows(QModelIndex(), row, row);
    list_.emplaceBack(settlement);
    endInsertRows();

    return true;
}

void PrimaryModel::RecallSucceeded(const QUuid& settlement_id, const QJsonObject& update)
{
    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    if (!settlement)
        return;

    settlement->ReadJson(update);
    settlement->amount = 0.0;
}

void PrimaryModel::UpdateSucceeded(const QUuid& settlement_id, const QJsonObject& update)
{
    auto* settlement { FindSettlement(settlement_id) };
    Q_ASSERT_X(settlement != nullptr, "SettlementModel::RecallSettlement", "Settlement must exist for recalled operation");

    if (!settlement)
        return;

    settlement->ReadJson(update);
}

void PrimaryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<PrimaryRow>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* settlement { ResourcePool<PrimaryRow>::Instance().Allocate() };
        settlement->ReadJson(obj);

        list_.emplaceBack(settlement);
    }

    sort(static_cast<int>(PrimaryField::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
}
