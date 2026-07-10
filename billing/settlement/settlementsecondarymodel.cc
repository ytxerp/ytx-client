#include "settlementsecondarymodel.h"

#include <QJsonArray>
#include <QTimer>

#include "component/constant.h"
#include "global/resourcepool.h"
#include "settlementenum.h"
#include "utils/templateutils.h"

SettlementSecondaryModel::SettlementSecondaryModel(const QStringList& header, SettlementStatus status, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , status_ { status }
{
}

SettlementSecondaryModel::~SettlementSecondaryModel() { ResourcePool<SettlementSecondary>::Instance().Recycle(list_cache_); }

QModelIndex SettlementSecondaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex SettlementSecondaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SettlementSecondaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SettlementSecondaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant SettlementSecondaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const SettlementSecondaryEnum column { index.column() };
    auto* settlement_node { static_cast<SettlementSecondary*>(index.internalPointer()) };

    switch (column) {
    case SettlementSecondaryEnum::kId:
        return settlement_node->id;
    case SettlementSecondaryEnum::kIssuedTime:
        return settlement_node->issued_time;
    case SettlementSecondaryEnum::kDescription:
        return settlement_node->description;
    case SettlementSecondaryEnum::kAmount:
        return settlement_node->amount;
    case SettlementSecondaryEnum::kEmployee:
        return settlement_node->employee_id;
    case SettlementSecondaryEnum::kIsSettled:
        return settlement_node->is_settled;
    }
}

bool SettlementSecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != std::to_underlying(SettlementSecondaryEnum::kIsSettled) || role != Qt::EditRole)
        return false;

    if (status_ == SettlementStatus::kReleased)
        return false;

    if (data(index, role) == value)
        return false;

    auto* settlement_node { static_cast<SettlementSecondary*>(index.internalPointer()) };

    const bool is_selected { value.toBool() };

    settlement_node->is_settled = is_selected;

    if (is_selected)
        pending_selected_.insert(settlement_node->id);
    else
        pending_selected_.remove(settlement_node->id);

    emit SSyncAmount(settlement_node->amount * (is_selected ? 1 : -1));

    return true;
}

QVariant SettlementSecondaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void SettlementSecondaryModel::sort(int column, Qt::SortOrder order)
{
    const SettlementSecondaryEnum e_column { column };

    auto Compare = [e_column, order](const SettlementSecondary* lhs, const SettlementSecondary* rhs) -> bool {
        switch (e_column) {
        case SettlementSecondaryEnum::kEmployee:
            return utils::CompareMember(lhs, rhs, &SettlementSecondary::employee_id, order);
        case SettlementSecondaryEnum::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &SettlementSecondary::issued_time, order);
        case SettlementSecondaryEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &SettlementSecondary::description, order);
        case SettlementSecondaryEnum::kAmount:
            return utils::CompareMember(lhs, rhs, &SettlementSecondary::amount, order);
        case SettlementSecondaryEnum::kIsSettled:
            return utils::CompareMember(lhs, rhs, &SettlementSecondary::is_settled, order);
        case SettlementSecondaryEnum::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void SettlementSecondaryModel::ResetModel(const QJsonArray& array)
{
    ResourcePool<SettlementSecondary>::Instance().Recycle(list_cache_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* settlement { ResourcePool<SettlementSecondary>::Instance().Allocate() };

        settlement->ReadJson(obj);

        list_cache_.emplaceBack(settlement);
    }

    {
        beginResetModel();

        list_.clear();

        for (auto* entry : std::as_const(list_cache_)) {
            if (status_ == SettlementStatus::kReleased && entry->is_settled)
                list_.emplaceBack(entry);

            if (status_ == SettlementStatus::kDraft)
                list_.emplaceBack(entry);
        }

        sort(std::to_underlying(SettlementSecondaryEnum::kIssuedTime), Qt::AscendingOrder);
        endResetModel();
    }
}

void SettlementSecondaryModel::UpdateStatus(SettlementStatus status)
{
    if (status_ == status)
        return;

    status_ = status;

    beginResetModel();

    if (status == SettlementStatus::kReleased) {
        list_.erase(std::remove_if(list_.begin(), list_.end(), [](const SettlementSecondary* node) { return !node->is_settled; }), list_.end());
    }

    if (status == SettlementStatus::kDraft) {
        for (auto* node : std::as_const(list_cache_))
            node->is_settled = false;

        list_ = list_cache_;
    }

    endResetModel();
}

void SettlementSecondaryModel::Finalize(QJsonObject& message)
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
