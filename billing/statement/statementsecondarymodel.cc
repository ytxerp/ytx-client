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

    return createIndex(row, column, list_.at(row));
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

    const StatementSecondaryEnum column { index.column() };
    auto* statement { static_cast<StatementSecondary*>(index.internalPointer()) };

    switch (column) {
    case StatementSecondaryEnum::kDescription:
        return statement->description;
    case StatementSecondaryEnum::kCode:
        return statement->code;
    case StatementSecondaryEnum::kEmployee:
        return statement->employee_id;
    case StatementSecondaryEnum::kIssuedTime:
        return statement->issued_time;
    case StatementSecondaryEnum::kCount:
        return statement->count;
    case StatementSecondaryEnum::kMeasure:
        return statement->measure;
    case StatementSecondaryEnum::kStatus:
        return statement->status;
    case StatementSecondaryEnum::kAmount:
        return statement->amount;
    case StatementSecondaryEnum::kSettlement:
        return statement->settlement;
    }
}

bool StatementSecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementSecondaryEnum column { index.column() };
    auto* statement { static_cast<StatementSecondary*>(index.internalPointer()) };

    switch (column) {
    case StatementSecondaryEnum::kStatus:
        statement->status = value.toInt();
        break;
    case StatementSecondaryEnum::kIssuedTime:
    case StatementSecondaryEnum::kAmount:
    case StatementSecondaryEnum::kCount:
    case StatementSecondaryEnum::kDescription:
    case StatementSecondaryEnum::kMeasure:
    case StatementSecondaryEnum::kEmployee:
    case StatementSecondaryEnum::kSettlement:
    case StatementSecondaryEnum::kCode:
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
    const StatementSecondaryEnum e_column { column };

    auto Compare = [e_column, order](const StatementSecondary* lhs, const StatementSecondary* rhs) -> bool {
        switch (e_column) {
        case StatementSecondaryEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::description, order);
        case StatementSecondaryEnum::kCode:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::code, order);
        case StatementSecondaryEnum::kEmployee:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::employee_id, order);
        case StatementSecondaryEnum::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::issued_time, order);
        case StatementSecondaryEnum::kCount:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::count, order);
        case StatementSecondaryEnum::kMeasure:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::measure, order);
        case StatementSecondaryEnum::kStatus:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::status, order);
        case StatementSecondaryEnum::kSettlement:
            return utils::CompareMember(lhs, rhs, &StatementSecondary::settlement, order);
        case StatementSecondaryEnum::kAmount:
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

    sort(std::to_underlying(StatementSecondaryEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
