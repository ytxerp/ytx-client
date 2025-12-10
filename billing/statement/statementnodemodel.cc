#include "statementnodemodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"

StatementNodeModel::StatementNodeModel(CSectionInfo& info, const QUuid& partner_id, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , partner_id_ { partner_id }
{
}

StatementNodeModel::~StatementNodeModel() { ResourcePool<StatementNode>::Instance().Recycle(list_); }

QModelIndex StatementNodeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex StatementNodeModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int StatementNodeModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int StatementNodeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.statement_node_header.size();
}

QVariant StatementNodeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* statement_primary { list_.at(index.row()) };
    const StatementNodeEnum column { index.column() };

    switch (column) {
    case StatementNodeEnum::kDescription:
        return statement_primary->description;
    case StatementNodeEnum::kEmployee:
        return statement_primary->employee;
    case StatementNodeEnum::kIssuedTime:
        return statement_primary->issued_time;
    case StatementNodeEnum::kCount:
        return statement_primary->count;
    case StatementNodeEnum::kMeasure:
        return statement_primary->measure;
    case StatementNodeEnum::kStatus:
        return statement_primary->status;
    case StatementNodeEnum::kAmount:
        return statement_primary->amount;
    case StatementNodeEnum::kSettlement:
        return statement_primary->settlement;
    default:
        return QVariant();
    }
}

bool StatementNodeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementNodeEnum column { index.column() };
    auto* node { list_.at(index.row()) };

    switch (column) {
    case StatementNodeEnum::kStatus:
        node->status = value.toInt();
        break;
    default:
        return false;
    }

    return true;
}

QVariant StatementNodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.statement_node_header.at(section);

    return QVariant();
}

void StatementNodeModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.statement_node_header.size())
        return;

    auto Compare = [column, order](const StatementNode* lhs, const StatementNode* rhs) -> bool {
        const StatementNodeEnum e_column { column };

        switch (e_column) {
        case StatementNodeEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case StatementNodeEnum::kEmployee:
            return (order == Qt::AscendingOrder) ? (lhs->employee < rhs->employee) : (lhs->employee > rhs->employee);
        case StatementNodeEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case StatementNodeEnum::kCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case StatementNodeEnum::kMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case StatementNodeEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        case StatementNodeEnum::kSettlement:
            return (order == Qt::AscendingOrder) ? (lhs->settlement < rhs->settlement) : (lhs->settlement > rhs->settlement);
        case StatementNodeEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementNodeModel::ResetModel(const QJsonArray& entry_array)
{
    beginResetModel();

    ResourcePool<StatementNode>::Instance().Recycle(list_);

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_primary { ResourcePool<StatementNode>::Instance().Allocate() };
        statement_primary->ReadJson(obj);

        list_.emplaceBack(statement_primary);
    }

    sort(static_cast<int>(StatementNodeEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
