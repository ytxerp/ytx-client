#include "settlementnodemodel.h"

#include "enum/settlementenum.h"
#include "global/resourcepool.h"

SettlementNodeModel::SettlementNodeModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

SettlementNodeModel::~SettlementNodeModel() { ResourcePool<SettlementNode>::Instance().Recycle(list_); }

QModelIndex SettlementNodeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex SettlementNodeModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementNodeModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SettlementNodeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.settlement_node_header.size();
}

QVariant SettlementNodeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SettlementNodeEnum column { index.column() };
    auto* settlement_node { list_.at(index.row()) };

    switch (column) {
    case SettlementNodeEnum::kId:
        return settlement_node->id;
    case SettlementNodeEnum::kSettlementId:
        return settlement_node->settlement_id;
    case SettlementNodeEnum::kIssuedTime:
        return settlement_node->issued_time;
    case SettlementNodeEnum::kDescription:
        return settlement_node->description;
    case SettlementNodeEnum::kAmount:
        return settlement_node->amount;
    case SettlementNodeEnum::kPartner:
        return settlement_node->partner;
    case SettlementNodeEnum::kEmployee:
        return settlement_node->employee;
    default:
        return QVariant();
    }
}

bool SettlementNodeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != std::to_underlying(SettlementNodeEnum::kSettlementId) || role != Qt::EditRole)
        return false;

    auto* settlement_node { list_.at(index.row()) };
    const bool check { value.toBool() };

    settlement_node->settlement_id = check ? settlement_id_ : QUuid();
    return true;
}

QVariant SettlementNodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_node_header.at(section);

    return QVariant();
}

void SettlementNodeModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.settlement_node_header.size())
        return;

    auto Compare = [column, order](const SettlementNode* lhs, const SettlementNode* rhs) -> bool {
        const SettlementNodeEnum e_column { column };

        switch (e_column) {
        case SettlementNodeEnum::kEmployee:
            return (order == Qt::AscendingOrder) ? (lhs->employee < rhs->employee) : (lhs->employee > rhs->employee);
        case SettlementNodeEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case SettlementNodeEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case SettlementNodeEnum::kPartner:
            return (order == Qt::AscendingOrder) ? (lhs->partner < rhs->partner) : (lhs->partner > rhs->partner);
        case SettlementNodeEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void SettlementNodeModel::UpdatePartner(const QUuid& partner_id) { partner_id_ = partner_id; }
