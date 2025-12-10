#include "settlementnodeeditmodel.h"

#include <QJsonArray>
#include <QTimer>

#include "enum/nodeenum.h"
#include "enum/settlementenum.h"

SettlementNodeEditModel::SettlementNodeEditModel(CSectionInfo& info, int status, CUuid& settlement_id, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , settlement_id_ { settlement_id }
    , status_ { status }
{
}

SettlementNodeEditModel::~SettlementNodeEditModel() { ResourcePool<SettlementNode>::Instance().Recycle(list_cache_); }

QModelIndex SettlementNodeEditModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex SettlementNodeEditModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementNodeEditModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SettlementNodeEditModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.settlement_node_header.size();
}

QVariant SettlementNodeEditModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SettlementNodeEnum column { index.column() };

    auto* settlement_node { static_cast<SettlementNode*>(index.internalPointer()) };
    if (!settlement_node)
        return QVariant();

    switch (column) {
    case SettlementNodeEnum::kId:
        return settlement_node->id;
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
    case SettlementNodeEnum::kStatus:
        return settlement_node->is_settled;
    default:
        return QVariant();
    }
}

bool SettlementNodeEditModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != std::to_underlying(SettlementNodeEnum::kStatus) || role != Qt::EditRole)
        return false;

    auto* settlement_node { static_cast<SettlementNode*>(index.internalPointer()) };
    if (!settlement_node)
        return false;

    settlement_node->is_settled = value.toBool();
    return true;
}

QVariant SettlementNodeEditModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_node_header.at(section);

    return QVariant();
}

void SettlementNodeEditModel::sort(int column, Qt::SortOrder order)
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
        case SettlementNodeEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->is_settled < rhs->is_settled) : (lhs->is_settled > rhs->is_settled);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void SettlementNodeEditModel::ResetModel(const QJsonArray& entry_array)
{
    ResourcePool<SettlementNode>::Instance().Recycle(list_cache_);

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* settlement { ResourcePool<SettlementNode>::Instance().Allocate() };

        settlement->ReadJson(obj);

        list_cache_.emplaceBack(settlement);
    }

    {
        beginResetModel();

        list_.clear();

        for (auto* entry : std::as_const(list_cache_)) {
            if (status_ == std::to_underlying(SettlementStatus::kReleased) && entry->is_settled)
                list_.emplaceBack(entry);

            if (status_ == std::to_underlying(SettlementStatus::kRecalled))
                list_.emplaceBack(entry);
        }

        sort(static_cast<int>(SettlementNodeEnum::kIssuedTime), Qt::AscendingOrder);
        endResetModel();
    }
}

void SettlementNodeEditModel::UpdateStatus(int status)
{
    if (status_ == status)
        return;

    status_ = status;

    {
        if (status == std::to_underlying(SettlementStatus::kReleased))
            for (int row = list_.size() - 1; row >= 0; --row) {
                auto* node = list_.at(row);
                if (!node->is_settled) {
                    beginRemoveRows(QModelIndex(), row, row);
                    list_.removeAt(row);
                    endRemoveRows();
                }
            }
    }

    {
        if (status == std::to_underlying(SettlementStatus::kRecalled)) {
            QList<SettlementNode*> to_add {};

            for (auto* node : std::as_const(list_cache_)) {
                if (!node->is_settled) {
                    to_add.append(node);
                }
            }

            if (!to_add.isEmpty()) {
                const qsizetype start_row { list_.size() };
                const qsizetype end_row { start_row + to_add.size() - 1 };

                beginInsertRows(QModelIndex(), start_row, end_row);
                list_.append(to_add);
                endInsertRows();
            }
        }
    }
}
