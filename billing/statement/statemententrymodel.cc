#include "statemententrymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"

StatementEntryModel::StatementEntryModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
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
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry { list_.at(index.row()) };
    const StatementEntryEnum column { index.column() };

    switch (column) {
    case StatementEntryEnum::kIssuedTime:
        return entry->issued_time;
    case StatementEntryEnum::kInternalSku:
        return entry->internal_sku;
    case StatementEntryEnum::kExternalSku:
        return entry->external_sku;
    case StatementEntryEnum::kCount:
        return entry->count;
    case StatementEntryEnum::kMeasure:
        return entry->measure;
    case StatementEntryEnum::kUnitPrice:
        return entry->unit_price;
    case StatementEntryEnum::kDescription:
        return entry->description;
    case StatementEntryEnum::kAmount:
        return entry->amount;
    case StatementEntryEnum::kStatus:
        return entry->status;
    default:
        return QVariant();
    }
}

bool StatementEntryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementEntryEnum column { index.column() };
    const int kRow { index.row() };

    auto* entry { list_.at(kRow) };

    switch (column) {
    case StatementEntryEnum::kStatus:
        entry->status = value.toInt();
        break;
    default:
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
    if (column <= -1 || column >= info_.statement_entry_header.size() - 1)
        return;

    auto Compare = [column, order](const StatementEntry* lhs, const StatementEntry* rhs) -> bool {
        const StatementEntryEnum e_column { column };

        switch (e_column) {
        case StatementEntryEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case StatementEntryEnum::kInternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->internal_sku < rhs->internal_sku) : (lhs->internal_sku > rhs->internal_sku);
        case StatementEntryEnum::kExternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->external_sku < rhs->external_sku) : (lhs->external_sku > rhs->external_sku);
        case StatementEntryEnum::kCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case StatementEntryEnum::kMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case StatementEntryEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case StatementEntryEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case StatementEntryEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        case StatementEntryEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementEntryModel::ResetModel(const QJsonArray& entry_array)
{
    beginResetModel();

    ResourcePool<StatementEntry>::Instance().Recycle(list_);

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_secondary { ResourcePool<StatementEntry>::Instance().Allocate() };
        statement_secondary->ReadJson(obj);

        list_.emplaceBack(statement_secondary);
    }

    sort(static_cast<int>(StatementEntryEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
