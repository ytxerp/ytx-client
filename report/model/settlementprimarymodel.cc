#include "settlementprimarymodel.h"

#include "component/enumclass.h"
#include "global/resourcepool.h"

SettlementPrimaryModel::SettlementPrimaryModel(EntryHub* dbhub, CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , dbhub_ { qobject_cast<EntryHubO*>(dbhub) }
    , info_ { info }
{
}

SettlementPrimaryModel::~SettlementPrimaryModel() { ResourcePool<Settlement>::Instance().Recycle(settlementList_list_); }

QModelIndex SettlementPrimaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SettlementPrimaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementPrimaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return settlementList_list_.size();
}

int SettlementPrimaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.settlement_primary_header.size();
}

QVariant SettlementPrimaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SettlementEnum kColumn { index.column() };
    auto* settlement { settlementList_list_.at(index.row()) };

    switch (kColumn) {
    case SettlementEnum::kId:
        return settlement->id;
    case SettlementEnum::kIssuedTime:
        return settlement->issued_time;
    case SettlementEnum::kDescription:
        return settlement->description;
    case SettlementEnum::kIsFinished:
        return settlement->is_finished ? settlement->is_finished : QVariant();
    case SettlementEnum::kInitialTotal:
        return settlement->initial_total == 0 ? QVariant() : settlement->initial_total;
    case SettlementEnum::kStakeholder:
        return settlement->employee.isNull() ? QVariant() : settlement->employee;
    default:
        return QVariant();
    }
}

bool SettlementPrimaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != std::to_underlying(SettlementEnum::kIsFinished) || role != Qt::EditRole || settlement_finished_)
        return false;

    auto* settlement { settlementList_list_.at(index.row()) };
    const bool check { value.toBool() };

    check ? dbhub_->AddSettlementPrimary(settlement->id, settlement_id_) : dbhub_->RemoveSettlementPrimary(settlement->id);

    settlement->is_finished = check;

    emit SSyncDouble(settlement_id_, std::to_underlying(SettlementEnum::kInitialTotal), check ? settlement->initial_total : -settlement->initial_total);
    return true;
}

QVariant SettlementPrimaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_primary_header.at(section);

    return QVariant();
}

void SettlementPrimaryModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.settlement_primary_header.size())
        return;

    auto Compare = [column, order](const Settlement* lhs, const Settlement* rhs) -> bool {
        const SettlementEnum kColumn { column };

        switch (kColumn) {
        case SettlementEnum::kStakeholder:
            return (order == Qt::AscendingOrder) ? (lhs->employee < rhs->employee) : (lhs->employee > rhs->employee);
        case SettlementEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case SettlementEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case SettlementEnum::kIsFinished:
            return (order == Qt::AscendingOrder) ? (lhs->is_finished < rhs->is_finished) : (lhs->is_finished > rhs->is_finished);
        case SettlementEnum::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(settlementList_list_.begin(), settlementList_list_.end(), Compare);
    emit layoutChanged();
}

void SettlementPrimaryModel::RemoveUnfinishedNode()
{
    if (settlementList_list_.isEmpty()) {
        return;
    }

    for (int i = settlementList_list_.size() - 1; i >= 0; --i) {
        if (!settlementList_list_[i]->is_finished) {
            beginRemoveRows(QModelIndex(), i, i);
            ResourcePool<Settlement>::Instance().Recycle(settlementList_list_.takeAt(i));
            endRemoveRows();
        }
    }
}

void SettlementPrimaryModel::UpdateSettlementInfo(const QUuid& party_id, const QUuid& settlement_id, bool settlement_finished)
{
    party_id_ = party_id;
    settlement_id_ = settlement_id;
    settlement_finished_ = settlement_finished;
}

void SettlementPrimaryModel::RSyncFinished(const QUuid& party_id, const QUuid& settlement_id, bool settlement_finished)
{
    UpdateSettlementInfo(party_id, settlement_id, settlement_finished);

    if (settlement_finished) {
        RemoveUnfinishedNode();
    } else {
        const bool is_empty { settlementList_list_.isEmpty() };

        const long long first_add { settlementList_list_.size() };
        dbhub_->ReadSettlementPrimary(settlementList_list_, party_id_, {}, true);
        const long long last_add { settlementList_list_.size() - 1 };

        if (last_add >= first_add) {
            beginInsertRows(QModelIndex(), first_add, last_add);
            endInsertRows();

            if (is_empty)
                sort(std::to_underlying(SettlementEnum::kIssuedTime), Qt::AscendingOrder);
        }
    }
}

void SettlementPrimaryModel::RResetModel(const QUuid& party_id, const QUuid& settlement_id, bool settlement_finished)
{
    UpdateSettlementInfo(party_id, settlement_id, settlement_finished);

    beginResetModel();
    if (!settlementList_list_.isEmpty())
        ResourcePool<Settlement>::Instance().Recycle(settlementList_list_);

    if (!party_id.isNull())
        dbhub_->ReadSettlementPrimary(settlementList_list_, party_id, settlement_id, settlement_finished);

    endResetModel();
}
