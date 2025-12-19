#include "tablemodelsettlement.h"

#include <QJsonArray>
#include <QTimer>

#include "enum/settlementenum.h"
#include "global/resourcepool.h"

TableModelSettlement::TableModelSettlement(CSectionInfo& info, SettlementStatus status, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , status_ { status }
{
}

TableModelSettlement::~TableModelSettlement() { ResourcePool<SettlementItem>::Instance().Recycle(list_cache_); }

QModelIndex TableModelSettlement::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex TableModelSettlement::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TableModelSettlement::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int TableModelSettlement::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.settlement_item_header.size();
}

QVariant TableModelSettlement::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const SettlementItemEnum column { index.column() };

    auto* settlement_node { static_cast<SettlementItem*>(index.internalPointer()) };
    if (!settlement_node)
        return QVariant();

    switch (column) {
    case SettlementItemEnum::kId:
        return settlement_node->id;
    case SettlementItemEnum::kIssuedTime:
        return settlement_node->issued_time;
    case SettlementItemEnum::kDescription:
        return settlement_node->description;
    case SettlementItemEnum::kAmount:
        return settlement_node->amount;
    case SettlementItemEnum::kEmployee:
        return settlement_node->employee_id;
    case SettlementItemEnum::kIsSelected:
        return settlement_node->is_selected;
    default:
        return QVariant();
    }
}

bool TableModelSettlement::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != std::to_underlying(SettlementItemEnum::kIsSelected) || role != Qt::EditRole)
        return false;

    if (status_ == SettlementStatus::kReleased)
        return false;

    auto* settlement_node { static_cast<SettlementItem*>(index.internalPointer()) };
    if (!settlement_node)
        return false;

    const bool is_settled { value.toBool() };

    settlement_node->is_selected = is_settled;

    if (is_settled)
        pending_insert_.insert(settlement_node->id);
    else
        pending_delete_.insert(settlement_node->id);

    emit SSyncAmount(settlement_node->amount * (is_settled ? 1 : -1));

    return true;
}

QVariant TableModelSettlement::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.settlement_item_header.at(section);

    return QVariant();
}

void TableModelSettlement::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.settlement_item_header.size())
        return;

    auto Compare = [column, order](const SettlementItem* lhs, const SettlementItem* rhs) -> bool {
        const SettlementItemEnum e_column { column };

        switch (e_column) {
        case SettlementItemEnum::kEmployee:
            return (order == Qt::AscendingOrder) ? (lhs->employee_id < rhs->employee_id) : (lhs->employee_id > rhs->employee_id);
        case SettlementItemEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case SettlementItemEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case SettlementItemEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        case SettlementItemEnum::kIsSelected:
            return (order == Qt::AscendingOrder) ? (lhs->is_selected < rhs->is_selected) : (lhs->is_selected > rhs->is_selected);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void TableModelSettlement::ResetModel(const QJsonArray& array)
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
            if (status_ == SettlementStatus::kReleased && entry->is_selected)
                list_.emplaceBack(entry);

            if (status_ == SettlementStatus::kRecalled)
                list_.emplaceBack(entry);
        }

        sort(static_cast<int>(SettlementItemEnum::kIssuedTime), Qt::AscendingOrder);
        endResetModel();
    }
}

void TableModelSettlement::UpdateStatus(SettlementStatus status)
{
    if (status_ == status)
        return;

    status_ = status;

    {
        if (status == SettlementStatus::kReleased)
            for (int row = list_.size() - 1; row >= 0; --row) {
                auto* node = list_.at(row);
                if (!node->is_selected) {
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
                if (!node->is_selected) {
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

    sort(static_cast<int>(SettlementItemEnum::kIsSelected), Qt::DescendingOrder);
}

void TableModelSettlement::Finalize(QJsonObject& message)
{
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

void TableModelSettlement::NormalizeBuffer()
{
    for (auto it = pending_insert_.begin(); it != pending_insert_.end();) {
        if (pending_delete_.remove(*it)) {
            it = pending_insert_.erase(it);
        } else {
            ++it;
        }
    }
}
