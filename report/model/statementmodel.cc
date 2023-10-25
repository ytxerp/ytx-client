#include "statementmodel.h"

#include <QTime>

#include "component/enumclass.h"
#include "global/resourcepool.h"

StatementModel::StatementModel(EntryHub* dbhub, CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , dbhub_ { static_cast<EntryHubO*>(dbhub) }
    , info_ { info }
{
}

StatementModel::~StatementModel() { ResourcePool<Statement>::Instance().Recycle(statement_list_); }

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
    return statement_list_.size();
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

    const StatementEnum kColumn { index.column() };
    auto* statement { statement_list_.at(index.row()) };

    switch (kColumn) {
    case StatementEnum::kParty:
        return statement->party;
    case StatementEnum::kPBalance:
        return statement->pbalance == 0 ? QVariant() : statement->pbalance;
    case StatementEnum::kCGrossAmount:
        return statement->cgross_amount == 0 ? QVariant() : statement->cgross_amount;
    case StatementEnum::kCSettlement:
        return statement->csettlement == 0 ? QVariant() : statement->csettlement;
    case StatementEnum::kCBalance:
        return statement->cbalance == 0 ? QVariant() : statement->cbalance;
    case StatementEnum::kCFirst:
        return statement->cfirst == 0 ? QVariant() : statement->cfirst;
    case StatementEnum::kCSecond:
        return statement->csecond == 0 ? QVariant() : statement->csecond;
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
        const StatementEnum kColumn { column };

        switch (kColumn) {
        case StatementEnum::kParty:
            return (order == Qt::AscendingOrder) ? (lhs->party < rhs->party) : (lhs->party > rhs->party);
        case StatementEnum::kPBalance:
            return (order == Qt::AscendingOrder) ? (lhs->pbalance < rhs->pbalance) : (lhs->pbalance > rhs->pbalance);
        case StatementEnum::kCGrossAmount:
            return (order == Qt::AscendingOrder) ? (lhs->cgross_amount < rhs->cgross_amount) : (lhs->cgross_amount > rhs->cgross_amount);
        case StatementEnum::kCSettlement:
            return (order == Qt::AscendingOrder) ? (lhs->csettlement < rhs->csettlement) : (lhs->csettlement > rhs->csettlement);
        case StatementEnum::kCBalance:
            return (order == Qt::AscendingOrder) ? (lhs->cbalance < rhs->cbalance) : (lhs->cbalance > rhs->cbalance);
        case StatementEnum::kCFirst:
            return (order == Qt::AscendingOrder) ? (lhs->cfirst < rhs->cfirst) : (lhs->cfirst > rhs->cfirst);
        case StatementEnum::kCSecond:
            return (order == Qt::AscendingOrder) ? (lhs->csecond < rhs->csecond) : (lhs->csecond > rhs->csecond);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(statement_list_.begin(), statement_list_.end(), Compare);
    emit layoutChanged();
}

void StatementModel::RResetModel(int unit, const QDateTime& start, const QDateTime& end)
{
    if (!start.isValid() || !end.isValid())
        return;

    beginResetModel();
    if (!statement_list_.isEmpty())
        ResourcePool<Statement>::Instance().Recycle(statement_list_);

    dbhub_->ReadStatement(statement_list_, unit, start.toUTC(), end.toUTC());
    endResetModel();
}
