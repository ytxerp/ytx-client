#include "statementsecondarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "global/resourcepool.h"
#include "statementenum.h"
#include "utils/templateutils.h"

StatementSecondaryModel::StatementSecondaryModel(const QStringList& header, const QUuid& partner_id, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , partner_id_ { partner_id }
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

int StatementSecondaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant StatementSecondaryModel::data(const QModelIndex& index, int role) const
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
    case StatementNodeEnum::kCode:
        return statement_primary->code;
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
    }
}

bool StatementSecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
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
    case StatementNodeEnum::kCode:
        return false;
    }

    return true;
}

QVariant StatementSecondaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void StatementSecondaryModel::sort(int column, Qt::SortOrder order)
{
    const StatementNodeEnum e_column { column };

    auto Compare = [e_column, order](const StatementSecondary* lhs, const StatementSecondary* rhs) -> bool {
        switch (e_column) {
        case StatementNodeEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::description, order);
        case StatementNodeEnum::kCode:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::code, order);
        case StatementNodeEnum::kEmployee:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::employee_id, order);
        case StatementNodeEnum::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::issued_time, order);
        case StatementNodeEnum::kCount:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::count, order);
        case StatementNodeEnum::kMeasure:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::measure, order);
        case StatementNodeEnum::kStatus:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::status, order);
        case StatementNodeEnum::kSettlement:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::settlement, order);
        case StatementNodeEnum::kAmount:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::amount, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementSecondaryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<StatementSecondary>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_primary { ResourcePool<StatementSecondary>::Instance().Allocate() };
        statement_primary->ReadJson(obj);

        list_.emplaceBack(statement_primary);
    }

    sort(std::to_underlying(StatementNodeEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
