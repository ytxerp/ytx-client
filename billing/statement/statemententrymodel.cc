#include "statemententrymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

StatementEntryModel::StatementEntryModel(EntryHubP* entry_hub_p, CSectionInfo& info, CUuid& partner_id, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , entry_hub_p_ { entry_hub_p }
    , partner_id_ { partner_id }
{
}

StatementEntryModel::~StatementEntryModel() { ResourcePool<StatementEntry>::Instance().Recycle(list_); }

QModelIndex StatementEntryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex StatementEntryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int StatementEntryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int StatementEntryModel::columnCount(const QModelIndex& /*parent*/) const { return info_.statement_entry_header.size(); }

QVariant StatementEntryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* entry { list_.at(index.row()) };
    const StatementEntryEnum column { index.column() };

    switch (column) {
    case StatementEntryEnum::kIssuedTime:
        return entry->issued_time;
    case StatementEntryEnum::kInternalSku:
        return entry->internal_sku;
    case StatementEntryEnum::kExternalSku:
        return entry_hub_p_->ExternalSku(partner_id_, entry->internal_sku);
    case StatementEntryEnum::kCount:
        return entry->count;
    case StatementEntryEnum::kMeasure:
        return entry->measure;
    case StatementEntryEnum::kUnitPrice:
        return entry->unit_price;
    case StatementEntryEnum::kDescription:
        return entry->description;
    case StatementEntryEnum::kCode:
        return entry->code;
    case StatementEntryEnum::kAmount:
        return entry->amount;
    case StatementEntryEnum::kStatus:
        return entry->status;
    }
}

bool StatementEntryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const StatementEntryEnum column { index.column() };
    const int kRow { index.row() };

    auto* entry { list_.at(kRow) };

    switch (column) {
    case StatementEntryEnum::kStatus:
        entry->status = value.toInt();
        break;
    case StatementEntryEnum::kIssuedTime:
    case StatementEntryEnum::kAmount:
    case StatementEntryEnum::kCount:
    case StatementEntryEnum::kDescription:
    case StatementEntryEnum::kMeasure:
    case StatementEntryEnum::kUnitPrice:
    case StatementEntryEnum::kInternalSku:
    case StatementEntryEnum::kExternalSku:
    case StatementEntryEnum::kCode:
        return false;
    }

    return true;
}

QVariant StatementEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.statement_entry_header.at(section);

    return QVariant();
}

void StatementEntryModel::sort(int column, Qt::SortOrder order)
{
    const StatementEntryEnum e_column { column };

    auto Compare = [e_column, order](const StatementEntry* lhs, const StatementEntry* rhs) -> bool {
        switch (e_column) {
        case StatementEntryEnum::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::issued_time, order);
        case StatementEntryEnum::kInternalSku:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::internal_sku, order);
        case StatementEntryEnum::kExternalSku:
            return false;
        case StatementEntryEnum::kCount:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::count, order);
        case StatementEntryEnum::kMeasure:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::measure, order);
        case StatementEntryEnum::kUnitPrice:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::unit_price, order);
        case StatementEntryEnum::kDescription:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::description, order);
        case StatementEntryEnum::kCode:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::code, order);
        case StatementEntryEnum::kAmount:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::amount, order);
        case StatementEntryEnum::kStatus:
            return Utils::CompareMember(lhs, rhs, &StatementEntry::status, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementEntryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<StatementEntry>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_secondary { ResourcePool<StatementEntry>::Instance().Allocate() };
        statement_secondary->ReadJson(obj);

        list_.emplaceBack(statement_secondary);
    }

    sort(std::to_underlying(StatementEntryEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}

void StatementEntryModel::ActionEntry(EntryAction action)
{
    if (list_.isEmpty())
        return;

    for (auto* entry : std::as_const(list_)) {
        switch (action) {
        case EntryAction::kMarkAll:
            entry->status = std::to_underlying(EntryStatus::kMarked);
            break;
        case EntryAction::kMarkNone:
            entry->status = std::to_underlying(EntryStatus::kUnmarked);
            break;
        case EntryAction::kMarkToggle:
            entry->status ^= std::to_underlying(EntryStatus::kMarked);
            break;
        }
    }

    const int column { std::to_underlying(StatementEntryEnum::kStatus) };

    const QModelIndex top_left { index(0, column) };
    const QModelIndex bottom_right { index(rowCount() - 1, column) };

    emit dataChanged(top_left, bottom_right, QList<int> { Qt::DisplayRole, Qt::EditRole });
}
