#include "statementprimarymodel.h"

#include <QTime>

#include "component/enumclass.h"
#include "global/resourcepool.h"

StatementPrimaryModel::StatementPrimaryModel(EntryHub* dbhub, CSectionInfo& info, const QUuid& party_id, QObject* parent)
    : QAbstractItemModel { parent }
    , dbhub_ { qobject_cast<EntryHubO*>(dbhub) }
    , info_ { info }
    , party_id_ { party_id }
{
}

StatementPrimaryModel::~StatementPrimaryModel() { ResourcePool<StatementPrimary>::Instance().Recycle(statement_primary_list_); }

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
    return statement_primary_list_.size();
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

    auto* node { statement_primary_list_.at(index.row()) };
    const StatementPrimaryEnum kColumn { index.column() };

    switch (kColumn) {
    case StatementPrimaryEnum::kDescription:
        return node->description;
    case StatementPrimaryEnum::kEmployee:
        return node->employee.isNull() ? QVariant() : node->employee;
    case StatementPrimaryEnum::kIssuedTime:
        return node->issued_time;
    case StatementPrimaryEnum::kFirst:
        return node->first == 0 ? QVariant() : node->first;
    case StatementPrimaryEnum::kSecond:
        return node->second == 0 ? QVariant() : node->second;
    case StatementPrimaryEnum::kIsChecked:
        return node->is_checked ? node->is_checked : QVariant();
    case StatementPrimaryEnum::kInitialTotal:
        return node->initial_total == 0 ? QVariant() : node->initial_total;
    case StatementPrimaryEnum::kFinalTotal:
        return node->final_total == 0 ? QVariant() : node->final_total;
    default:
        return QVariant();
    }
}

bool StatementPrimaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementPrimaryEnum kColumn { index.column() };
    auto* node { statement_primary_list_.at(index.row()) };

    switch (kColumn) {
    case StatementPrimaryEnum::kIsChecked:
        node->is_checked = value.toBool();
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
        const StatementPrimaryEnum kColumn { column };

        switch (kColumn) {
        case StatementPrimaryEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case StatementPrimaryEnum::kEmployee:
            return (order == Qt::AscendingOrder) ? (lhs->employee < rhs->employee) : (lhs->employee > rhs->employee);
        case StatementPrimaryEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case StatementPrimaryEnum::kFirst:
            return (order == Qt::AscendingOrder) ? (lhs->first < rhs->first) : (lhs->first > rhs->first);
        case StatementPrimaryEnum::kSecond:
            return (order == Qt::AscendingOrder) ? (lhs->second < rhs->second) : (lhs->second > rhs->second);
        case StatementPrimaryEnum::kIsChecked:
            return (order == Qt::AscendingOrder) ? (lhs->is_checked < rhs->is_checked) : (lhs->is_checked > rhs->is_checked);
        case StatementPrimaryEnum::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        case StatementPrimaryEnum::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(statement_primary_list_.begin(), statement_primary_list_.end(), Compare);
    emit layoutChanged();
}

void StatementPrimaryModel::RResetModel(int unit, const QDateTime& start, const QDateTime& end)
{
    if (party_id_.isNull() || !start.isValid() || !end.isValid())
        return;

    beginResetModel();

    if (!statement_primary_list_.isEmpty())
        ResourcePool<StatementPrimary>::Instance().Recycle(statement_primary_list_);

    dbhub_->ReadStatementPrimary(statement_primary_list_, party_id_, unit, start.toUTC(), end.toUTC());
    endResetModel();
}
