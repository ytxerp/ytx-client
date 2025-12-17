#include "statementmodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statementenum.h"
#include "global/resourcepool.h"

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
    default:
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
    if (column <= -1 || column >= info_.statement_header.size())
        return;

    auto Compare = [column, order](const Statement* lhs, const Statement* rhs) -> bool {
        const StatementEnum e_column { column };

        switch (e_column) {
        case StatementEnum::kPartner:
            return (order == Qt::AscendingOrder) ? (lhs->partner_id < rhs->partner_id) : (lhs->partner_id > rhs->partner_id);
        case StatementEnum::kPBalance:
            return (order == Qt::AscendingOrder) ? (lhs->pbalance < rhs->pbalance) : (lhs->pbalance > rhs->pbalance);
        case StatementEnum::kCAmount:
            return (order == Qt::AscendingOrder) ? (lhs->camount < rhs->camount) : (lhs->camount > rhs->camount);
        case StatementEnum::kCSettlement:
            return (order == Qt::AscendingOrder) ? (lhs->csettlement < rhs->csettlement) : (lhs->csettlement > rhs->csettlement);
        case StatementEnum::kCBalance:
            return (order == Qt::AscendingOrder) ? (lhs->cbalance < rhs->cbalance) : (lhs->cbalance > rhs->cbalance);
        case StatementEnum::kCCount:
            return (order == Qt::AscendingOrder) ? (lhs->ccount < rhs->ccount) : (lhs->ccount > rhs->ccount);
        case StatementEnum::kCMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->cmeasure < rhs->cmeasure) : (lhs->cmeasure > rhs->cmeasure);
        default:
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

    sort(static_cast<int>(StatementEnum::kPartner), Qt::AscendingOrder);
    endResetModel();
}
