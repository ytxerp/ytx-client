#include "statementsecondarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"

StatementSecondaryModel::StatementSecondaryModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

StatementSecondaryModel::~StatementSecondaryModel() { ResourcePool<StatementSecondary>::Instance().Recycle(list_); }

QModelIndex StatementSecondaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex StatementSecondaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int StatementSecondaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int StatementSecondaryModel::columnCount(const QModelIndex& /*parent*/) const { return info_.statement_secondary_header.size(); }

QVariant StatementSecondaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry { list_.at(index.row()) };
    const StatementSecondaryEnum column { index.column() };

    switch (column) {
    case StatementSecondaryEnum::kIssuedTime:
        return entry->issued_time;
    case StatementSecondaryEnum::kInternalSku:
        return entry->internal_sku;
    case StatementSecondaryEnum::kExternalSku:
        return entry->external_sku;
    case StatementSecondaryEnum::kCount:
        return entry->count;
    case StatementSecondaryEnum::kMeasure:
        return entry->measure;
    case StatementSecondaryEnum::kUnitPrice:
        return entry->unit_price;
    case StatementSecondaryEnum::kDescription:
        return entry->description;
    case StatementSecondaryEnum::kAmount:
        return entry->amount;
    case StatementSecondaryEnum::kStatus:
        return entry->status;
    default:
        return QVariant();
    }
}

bool StatementSecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementSecondaryEnum column { index.column() };
    const int kRow { index.row() };

    auto* entry { list_.at(kRow) };

    switch (column) {
    case StatementSecondaryEnum::kStatus:
        entry->status = value.toInt();
        break;
    default:
        return false;
    }

    return true;
}

QVariant StatementSecondaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.statement_secondary_header.at(section);

    return QVariant();
}

void StatementSecondaryModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.statement_secondary_header.size() - 1)
        return;

    auto Compare = [column, order](const StatementSecondary* lhs, const StatementSecondary* rhs) -> bool {
        const StatementSecondaryEnum e_column { column };

        switch (e_column) {
        case StatementSecondaryEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case StatementSecondaryEnum::kInternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->internal_sku < rhs->internal_sku) : (lhs->internal_sku > rhs->internal_sku);
        case StatementSecondaryEnum::kExternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->external_sku < rhs->external_sku) : (lhs->external_sku > rhs->external_sku);
        case StatementSecondaryEnum::kCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case StatementSecondaryEnum::kMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case StatementSecondaryEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case StatementSecondaryEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case StatementSecondaryEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        case StatementSecondaryEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementSecondaryModel::ResetModel(const QJsonArray& entry_array)
{
    beginResetModel();

    ResourcePool<StatementSecondary>::Instance().Recycle(list_);

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_secondary { ResourcePool<StatementSecondary>::Instance().Allocate() };
        statement_secondary->ReadJson(obj);

        list_.emplaceBack(statement_secondary);
    }

    endResetModel();
}
