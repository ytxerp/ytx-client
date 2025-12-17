#include "settlementitemmodel.h"

#include <QJsonArray>
#include <QTimer>

#include "enum/settlementenum.h"
#include "global/resourcepool.h"

SettlementItemModel::SettlementItemModel(CSectionInfo& info, SettlementStatus status, CUuid& settlement_id, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , settlement_id_ { settlement_id }
    , status_ { status }
{
}

SettlementItemModel::~SettlementItemModel() { ResourcePool<SettlementItem>::Instance().Recycle(list_cache_); }

QModelIndex SettlementItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex SettlementItemModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementItemModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SettlementItemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.settlement_item_header.size();
}

QVariant SettlementItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SettlementNodeEnum column { index.column() };

    auto* settlement_node { static_cast<SettlementItem*>(index.internalPointer()) };
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
    case SettlementNodeEnum::kIsSettled:
        return settlement_node->is_settled;
    default:
        return QVariant();
    }
}

bool SettlementItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != std::to_underlying(SettlementNodeEnum::kIsSettled) || role != Qt::EditRole)
        return false;

    if (status_ == SettlementStatus::kReleased)
        return false;

    auto* settlement_node { static_cast<SettlementItem*>(index.internalPointer()) };
    if (!settlement_node)
        return false;

    const bool is_settled { value.toBool() };

    settlement_node->is_settled = is_settled;

    if (is_settled)
        pending_insert_.insert(settlement_node->id);
    else
        pending_delete_.insert(settlement_node->id);

    emit SSyncAmount(settlement_node->amount * (is_settled ? 1 : -1));

    return true;
}

QVariant SettlementItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_item_header.at(section);

    return QVariant();
}

void SettlementItemModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.settlement_item_header.size())
        return;

    auto Compare = [column, order](const SettlementItem* lhs, const SettlementItem* rhs) -> bool {
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
        case SettlementNodeEnum::kIsSettled:
            return (order == Qt::AscendingOrder) ? (lhs->is_settled < rhs->is_settled) : (lhs->is_settled > rhs->is_settled);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void SettlementItemModel::ResetModel(const QJsonArray& array)
{
    ResourcePool<SettlementItem>::Instance().Recycle(list_cache_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* settlement { ResourcePool<SettlementItem>::Instance().Allocate() };

        settlement->ReadJson(obj);

        list_cache_.emplaceBack(settlement);
    }

    {
        beginResetModel();

        list_.clear();

        for (auto* entry : std::as_const(list_cache_)) {
            if (status_ == SettlementStatus::kReleased && entry->is_settled)
                list_.emplaceBack(entry);

            if (status_ == SettlementStatus::kRecalled)
                list_.emplaceBack(entry);
        }

        sort(static_cast<int>(SettlementNodeEnum::kIssuedTime), Qt::AscendingOrder);
        endResetModel();
    }
}

void SettlementItemModel::UpdateStatus(SettlementStatus status)
{
    if (status_ == status)
        return;

    status_ = status;

    {
        if (status == SettlementStatus::kReleased)
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
        if (status == SettlementStatus::kRecalled) {
            QList<SettlementItem*> to_add {};

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

void SettlementItemModel::Finalize(QJsonObject& message)
{
    {
        // Normalize diff buffers (inserted / deleted)
        // to ensure no conflict states remain before packaging.
        // e.g. inserted+deleted ⇒ remove both
        // NOTE: update caches are implicitly consistent: newly inserted never have update update.
        NormalizeBuffer();
    }

    // deleted
    {
        if (!pending_delete_.isEmpty()) {
            QJsonArray deleted_array {};
            for (const auto& id : std::as_const(pending_delete_)) {
                deleted_array.append(id.toString(QUuid::WithoutBraces));
            }

            message.insert(kSettlementItemDeleted, deleted_array);
        }
    }

    // insert
    {
        if (!pending_insert_.isEmpty()) {
            QJsonArray inserted_array {};
            for (const auto& id : std::as_const(pending_insert_)) {
                inserted_array.append(id.toString(QUuid::WithoutBraces));
            }

            message.insert(kSettlementItemInserted, inserted_array);
        }
    }

    pending_delete_.clear();
    pending_insert_.clear();
}

void SettlementItemModel::NormalizeBuffer()
{
    for (auto it = pending_insert_.begin(); it != pending_insert_.end();) {
        if (pending_delete_.remove(*it)) {
            it = pending_insert_.erase(it);
        } else {
            ++it;
        }
    }
}
