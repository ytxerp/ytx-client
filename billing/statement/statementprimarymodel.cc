#include "statementprimarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "global/resourcepool.h"
#include "statementenum.h"
#include "utils/templateutils.h"

StatementPrimaryModel::StatementPrimaryModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
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
    return header_.size();
}

QVariant StatementPrimaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const StatementEnum column { index.column() };
    auto* statement { list_.at(index.row()) };

    switch (column) {
    case StatementEnum::kPartner:
        return statement->partner_id;
    case StatementEnum::kPBalance:
        return statement->pbalance;
    case StatementEnum::kCAmount:
        return statement->camount;
    case StatementEnum::kCSettlement:
        return statement->csettlement;
    case StatementEnum::kCBalance:
        return statement->cbalance;
    case StatementEnum::kCCount:
        return statement->ccount;
    case StatementEnum::kCMeasure:
        return statement->cmeasure;
    case StatementEnum::kPlaceholder:
        return QVariant();
    }
}

QVariant StatementPrimaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void StatementPrimaryModel::sort(int column, Qt::SortOrder order)
{
    const StatementEnum e_column { column };

    auto Compare = [e_column, order](const StatementPrimary* lhs, const StatementPrimary* rhs) -> bool {
        switch (e_column) {
        case StatementEnum::kPartner:
            return utils::CompareMember(lhs, rhs, &StatementPrimary::partner_id, order);
        case StatementEnum::kPBalance:
            return utils::CompareMember(lhs, rhs, &StatementPrimary::pbalance, order);
        case StatementEnum::kCAmount:
            return utils::CompareMember(lhs, rhs, &StatementPrimary::camount, order);
        case StatementEnum::kCSettlement:
            return utils::CompareMember(lhs, rhs, &StatementPrimary::csettlement, order);
        case StatementEnum::kCBalance:
            return utils::CompareMember(lhs, rhs, &StatementPrimary::cbalance, order);
        case StatementEnum::kCCount:
            return utils::CompareMember(lhs, rhs, &StatementPrimary::ccount, order);
        case StatementEnum::kCMeasure:
            return utils::CompareMember(lhs, rhs, &StatementPrimary::cmeasure, order);
        case StatementEnum::kPlaceholder:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementPrimaryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<StatementPrimary>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement { ResourcePool<StatementPrimary>::Instance().Allocate() };
        statement->ReadJson(obj);

        list_.emplaceBack(statement);
    }

    sort(std::to_underlying(StatementEnum::kPartner), Qt::AscendingOrder);
    endResetModel();
}
