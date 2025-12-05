#include "statementprimarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"

StatementPrimaryModel::StatementPrimaryModel(CSectionInfo& info, const QUuid& partner_id, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , partner_id_ { partner_id }
{
}

StatementPrimaryModel::~StatementPrimaryModel() { ResourcePool<StatementPrimary>::Instance().Recycle(list_); }

QModelIndex StatementPrimaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex StatementPrimaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int StatementPrimaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int StatementPrimaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.statement_primary_header.size();
}

QVariant StatementPrimaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* statement_primary { list_.at(index.row()) };
    const StatementPrimaryEnum column { index.column() };

    switch (column) {
    case StatementPrimaryEnum::kDescription:
        return statement_primary->description;
    case StatementPrimaryEnum::kEmployee:
        return statement_primary->employee;
    case StatementPrimaryEnum::kIssuedTime:
        return statement_primary->issued_time;
    case StatementPrimaryEnum::kCount:
        return statement_primary->count;
    case StatementPrimaryEnum::kMeasure:
        return statement_primary->measure;
    case StatementPrimaryEnum::kStatus:
        return statement_primary->status;
    case StatementPrimaryEnum::kAmount:
        return statement_primary->amount;
    case StatementPrimaryEnum::kSettlement:
        return statement_primary->settlement;
    default:
        return QVariant();
    }
}

bool StatementPrimaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementPrimaryEnum column { index.column() };
    auto* node { list_.at(index.row()) };

    switch (column) {
    case StatementPrimaryEnum::kStatus:
        node->status = value.toInt();
        break;
    default:
        return false;
    }

    return true;
}

QVariant StatementPrimaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.statement_primary_header.at(section);

    return QVariant();
}

void StatementPrimaryModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.statement_primary_header.size())
        return;

    auto Compare = [column, order](const StatementPrimary* lhs, const StatementPrimary* rhs) -> bool {
        const StatementPrimaryEnum e_column { column };

        switch (e_column) {
        case StatementPrimaryEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case StatementPrimaryEnum::kEmployee:
            return (order == Qt::AscendingOrder) ? (lhs->employee < rhs->employee) : (lhs->employee > rhs->employee);
        case StatementPrimaryEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case StatementPrimaryEnum::kCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case StatementPrimaryEnum::kMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case StatementPrimaryEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        case StatementPrimaryEnum::kSettlement:
            return (order == Qt::AscendingOrder) ? (lhs->settlement < rhs->settlement) : (lhs->settlement > rhs->settlement);
        case StatementPrimaryEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementPrimaryModel::ResetModel(const QJsonArray& entry_array)
{
    beginResetModel();

    ResourcePool<StatementPrimary>::Instance().Recycle(list_);

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_primary { ResourcePool<StatementPrimary>::Instance().Allocate() };
        statement_primary->ReadJson(obj);

        list_.emplaceBack(statement_primary);
    }

    endResetModel();
}
