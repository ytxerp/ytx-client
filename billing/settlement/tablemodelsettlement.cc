#include "tablemodelsettlement.h"

#include <QJsonArray>
#include <QTimer>

#include "enum/settlementenum.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

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
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
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
    case SettlementItemEnum::kIsSettled:
        return settlement_node->is_settled;
    }
}

bool TableModelSettlement::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != std::to_underlying(SettlementItemEnum::kIsSettled) || role != Qt::EditRole)
        return false;

    if (status_ == SettlementStatus::kSettled)
        return false;

    if (data(index, role) == value)
        return false;

    auto* settlement_node { static_cast<SettlementItem*>(index.internalPointer()) };
    if (!settlement_node)
        return false;

    const bool is_selected { value.toBool() };

    settlement_node->is_settled = is_selected;

    if (is_selected)
        pending_selected_.insert(settlement_node->id);
    else
        pending_selected_.remove(settlement_node->id);

    emit SSyncAmount(settlement_node->amount * (is_selected ? 1 : -1));

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
    const SettlementItemEnum e_column { column };

    auto Compare = [e_column, order](const SettlementItem* lhs, const SettlementItem* rhs) -> bool {
        switch (e_column) {
        case SettlementItemEnum::kEmployee:
            return Utils::CompareMember(lhs, rhs, &SettlementItem::employee_id, order);
        case SettlementItemEnum::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &SettlementItem::issued_time, order);
        case SettlementItemEnum::kDescription:
            return Utils::CompareMember(lhs, rhs, &SettlementItem::description, order);
        case SettlementItemEnum::kAmount:
            return Utils::CompareMember(lhs, rhs, &SettlementItem::amount, order);
        case SettlementItemEnum::kIsSettled:
            return Utils::CompareMember(lhs, rhs, &SettlementItem::is_settled, order);
        case SettlementItemEnum::kId:
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
            if (status_ == SettlementStatus::kSettled && entry->is_settled)
                list_.emplaceBack(entry);

            if (status_ == SettlementStatus::kUnsettled)
                list_.emplaceBack(entry);
        }

        sort(std::to_underlying(SettlementItemEnum::kIssuedTime), Qt::AscendingOrder);
        endResetModel();
    }
}

void TableModelSettlement::UpdateStatus(SettlementStatus status)
{
    if (status_ == status)
        return;

    status_ = status;

    beginResetModel();

    if (status == SettlementStatus::kSettled) {
        list_.erase(std::remove_if(list_.begin(), list_.end(), [](const SettlementItem* node) { return !node->is_settled; }), list_.end());
    }

    if (status == SettlementStatus::kUnsettled) {
        for (auto* node : std::as_const(list_cache_))
            node->is_settled = false;

        list_ = list_cache_;
    }

    endResetModel();
}

void TableModelSettlement::Finalize(QJsonObject& message)
{
    {
        QJsonArray selected_array {};
        for (const auto& id : std::as_const(pending_selected_)) {
            selected_array.append(id.toString(QUuid::WithoutBraces));
        }

        message.insert(kSettlementItemSelected, selected_array);
    }

    pending_selected_.clear();
}
