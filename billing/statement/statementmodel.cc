#include "statementmodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

StatementModel::StatementModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

StatementModel::~StatementModel() { ResourcePool<Statement>::Instance().Recycle(list_); }

QModelIndex StatementModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex StatementModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int StatementModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int StatementModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.statement_header.size();
}

QVariant StatementModel::data(const QModelIndex& index, int role) const
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

QVariant StatementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.statement_header.at(section);

    return QVariant();
}

void StatementModel::sort(int column, Qt::SortOrder order)
{
    const StatementEnum e_column { column };

    auto Compare = [e_column, order](const Statement* lhs, const Statement* rhs) -> bool {
        switch (e_column) {
        case StatementEnum::kPartner:
            return Utils::CompareMember(lhs, rhs, &Statement::partner_id, order);
        case StatementEnum::kPBalance:
            return Utils::CompareMember(lhs, rhs, &Statement::pbalance, order);
        case StatementEnum::kCAmount:
            return Utils::CompareMember(lhs, rhs, &Statement::camount, order);
        case StatementEnum::kCSettlement:
            return Utils::CompareMember(lhs, rhs, &Statement::csettlement, order);
        case StatementEnum::kCBalance:
            return Utils::CompareMember(lhs, rhs, &Statement::cbalance, order);
        case StatementEnum::kCCount:
            return Utils::CompareMember(lhs, rhs, &Statement::ccount, order);
        case StatementEnum::kCMeasure:
            return Utils::CompareMember(lhs, rhs, &Statement::cmeasure, order);
        case StatementEnum::kPlaceholder:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<Statement>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement { ResourcePool<Statement>::Instance().Allocate() };
        statement->ReadJson(obj);

        list_.emplaceBack(statement);
    }

    sort(std::to_underlying(StatementEnum::kPartner), Qt::AscendingOrder);
    endResetModel();
}
