#include "settlementmodel.h"

#include "component/constant.h"
#include "component/enumclass.h"
#include "global/resourcepool.h"
#include "global/websocket.h"
#include "utils/jsongen.h"

SettlementModel::SettlementModel(EntryHub* dbhub, CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , dbhub_ { static_cast<EntryHubO*>(dbhub) }
    , info_ { info }
{
}

SettlementModel::~SettlementModel() { ResourcePool<Settlement>::Instance().Recycle(settlement_list_); }

void SettlementModel::RSyncDouble(const QUuid& settlement_id, int column, double delta1)
{
    if (delta1 == 0.0 || column != std::to_underlying(SettlementEnum::kInitialTotal))
        return;

    int row { 0 };

    for (auto* settlement : std::as_const(settlement_list_)) {
        if (settlement->id == settlement_id) {
            settlement->initial_total += delta1;

            emit SUpdateAmount(settlement->partner, 0.0, -delta1); // send to partner
            emit dataChanged(index(row, std::to_underlying(SettlementEnum::kInitialTotal)), index(row, std::to_underlying(SettlementEnum::kInitialTotal)));

            // dbhub_->WriteField(info_->settlement, kInitialTotal, settlement->initial_total, settlement->id);
            break;
        }

        ++row;
    }
}

QModelIndex SettlementModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SettlementModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return settlement_list_.size();
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

    const SettlementEnum kColumn { index.column() };
    auto* settlement { settlement_list_.at(index.row()) };

    switch (kColumn) {
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
        return settlement->status ? settlement->status : QVariant();
    case SettlementEnum::kInitialTotal:
        return settlement->initial_total == 0 ? QVariant() : settlement->initial_total;
    case SettlementEnum::kPartner:
        return settlement->partner.isNull() ? QVariant() : settlement->partner;
    default:
        return QVariant();
    }
}

bool SettlementModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const SettlementEnum kColumn { index.column() };
    const int kRow { index.row() };

    auto* settlement { settlement_list_.at(kRow) };
    QJsonObject cache {};

    switch (kColumn) {
    case SettlementEnum::kIssuedTime:
        NodeUtils::UpdateIssuedTime(cache, settlement, kIssuedTime, value.toDateTime(), &Settlement::issued_time);
        break;
    case SettlementEnum::kDescription:
        NodeUtils::UpdateField(update_cache_, settlement, kDescription, value.toString(), &Settlement::description);
        break;
    case SettlementEnum::kPartner:
        UpdatePartner(settlement, value.toUuid());
        break;
    case SettlementEnum::kStatus:
        UpdateFinished(settlement, value.toBool());
        break;
    default:
        return false;
    }

    if (!cache.isEmpty()) {
        const auto message { JsonGen::Update(info_.section_str, settlement->id, cache) };
        WebSocket::Instance()->SendMessage(kNodeUpdate, message);
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

QVariant SettlementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_header.at(section);

    return QVariant();
}

Qt::ItemFlags SettlementModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const SettlementEnum kColumn { index.column() };

    switch (kColumn) {
    case SettlementEnum::kIssuedTime:
    case SettlementEnum::kDescription:
    case SettlementEnum::kPartner:
        flags |= Qt::ItemIsEditable;
        break;
    default:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    const bool non_editable { index.siblingAtColumn(std::to_underlying(SettlementEnum::kStatus)).data().toBool() };
    if (non_editable)
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

void SettlementModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.settlement_header.size())
        return;

    auto Compare = [column, order](const Settlement* lhs, const Settlement* rhs) -> bool {
        const SettlementEnum kColumn { column };

        switch (kColumn) {
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
        case SettlementEnum::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(settlement_list_.begin(), settlement_list_.end(), Compare);
    emit layoutChanged();
}

bool SettlementModel::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);
    auto* settlement { settlement_list_.takeAt(row) };
    endRemoveRows();

    if (settlement->initial_total != 0.0)
        emit SUpdateAmount(settlement->partner, std::to_underlying(NodeEnumP::kFinalTotal), settlement->initial_total);

    dbhub_->RemoveSettlement(settlement->id);
    ResourcePool<Settlement>::Instance().Recycle(settlement);

    emit SResetModel({}, {}, false);
    return true;
}

bool SettlementModel::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    auto* settlement { ResourcePool<Settlement>::Instance().Allocate() };
    const bool is_empty { settlement_list_.isEmpty() };

    settlement->issued_time = QDateTime::currentDateTimeUtc();
    settlement->id = QUuid::createUuidV7();

    beginInsertRows(parent, row, row);
    settlement_list_.emplaceBack(settlement);
    endInsertRows();

    if (is_empty)
        emit SResizeColumnToContents(std::to_underlying(SettlementEnum::kIssuedTime));

    dbhub_->WriteSettlement(settlement);
    return true;
}

bool SettlementModel::UpdatePartner(Settlement* settlement, const QUuid& partner_id)
{
    if (dbhub_->SettlementReference(settlement->id) || settlement->partner == partner_id)
        return false;

    settlement->partner = partner_id;
    // dbhub_->WriteField(info_->settlement, kPartner, partner_id, settlement->id);

    emit SResetModel(partner_id, settlement->id, false);
    return true;
}

bool SettlementModel::UpdateFinished(Settlement* settlement, bool finished)
{
    if (!settlement->partner.isNull())
        return false;

    settlement->status = finished;
    // dbhub_->WriteField(info_->settlement, kIsFinished, finished, settlement->id);

    emit SSyncFinished(settlement->partner, settlement->id, finished);
    return true;
}

void SettlementModel::ResetModel(const QDateTime& start, const QDateTime& end)
{
    if (!start.isValid() || !end.isValid())
        return;

    beginResetModel();
    if (!settlement_list_.isEmpty())
        ResourcePool<Settlement>::Instance().Recycle(settlement_list_);

    dbhub_->ReadSettlement(settlement_list_, start.toUTC(), end.toUTC());
    endResetModel();
}
