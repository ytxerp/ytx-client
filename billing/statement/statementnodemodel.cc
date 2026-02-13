#include "statementnodemodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

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
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* statement_primary { list_.at(index.row()) };
    const StatementNodeEnum column { index.column() };

    switch (column) {
    case StatementNodeEnum::kDescription:
        return statement_primary->description;
    case StatementNodeEnum::kEmployee:
        return statement_primary->employee_id;
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
    case StatementNodeEnum::kIssuedTime:
    case StatementNodeEnum::kAmount:
    case StatementNodeEnum::kCount:
    case StatementNodeEnum::kDescription:
    case StatementNodeEnum::kMeasure:
    case StatementNodeEnum::kEmployee:
    case StatementNodeEnum::kSettlement:
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
    const StatementNodeEnum e_column { column };

    auto Compare = [e_column, order](const StatementNode* lhs, const StatementNode* rhs) -> bool {
        switch (e_column) {
        case StatementNodeEnum::kDescription:
            return Utils::CompareMember(lhs, rhs, &StatementNode::description, order);
        case StatementNodeEnum::kEmployee:
            return Utils::CompareMember(lhs, rhs, &StatementNode::employee_id, order);
        case StatementNodeEnum::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &StatementNode::issued_time, order);
        case StatementNodeEnum::kCount:
            return Utils::CompareMember(lhs, rhs, &StatementNode::count, order);
        case StatementNodeEnum::kMeasure:
            return Utils::CompareMember(lhs, rhs, &StatementNode::measure, order);
        case StatementNodeEnum::kStatus:
            return Utils::CompareMember(lhs, rhs, &StatementNode::status, order);
        case StatementNodeEnum::kSettlement:
            return Utils::CompareMember(lhs, rhs, &StatementNode::settlement, order);
        case StatementNodeEnum::kAmount:
            return Utils::CompareMember(lhs, rhs, &StatementNode::amount, order);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementNodeModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<StatementNode>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_primary { ResourcePool<StatementNode>::Instance().Allocate() };
        statement_primary->ReadJson(obj);

        list_.emplaceBack(statement_primary);
    }

    sort(std::to_underlying(StatementNodeEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
