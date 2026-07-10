#include "cashflowwrongmodel.h"

#include <QtCore/qjsonarray.h>

#include "cashflowstatementenum.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"
#include "websocket/jsongen.h"

CashFlowWrongModel::CashFlowWrongModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
{
}

CashFlowWrongModel::~CashFlowWrongModel() { }

QModelIndex CashFlowWrongModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex CashFlowWrongModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CashFlowWrongModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int CashFlowWrongModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant CashFlowWrongModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void CashFlowWrongModel::ResetModel(CJsonArray& node_array)
{
    const auto list { AddRowsList(node_array) };

    beginResetModel();
    list_ = list;
    endResetModel();
}

QList<CashFlowStatementWrongRow*> CashFlowWrongModel::AddRowsList(const CJsonArray& node_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    QList<CashFlowStatementWrongRow*> list {};

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        auto* node { ResourcePool<CashFlowStatementWrongRow>::Instance().Allocate() };
        node->ReadJson(obj);
        list.emplaceBack(node);
    }

    return list;
}

QVariant CashFlowWrongModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const CashFlowStatementWrongEnum column { index.column() };
    auto* entry { static_cast<CashFlowStatementWrongRow*>(index.internalPointer()) };

    switch (column) {
    case CashFlowStatementWrongEnum::kId:
        return entry->id;
    case CashFlowStatementWrongEnum::kIssuedTime:
        return entry->issued_time;
    case CashFlowStatementWrongEnum::kLhsNode:
        return entry->lhs_node;
    case CashFlowStatementWrongEnum::kLhsDebit:
        return entry->lhs_debit;
    case CashFlowStatementWrongEnum::kLhsCredit:
        return entry->lhs_credit;
    case CashFlowStatementWrongEnum::kDescription:
        return entry->description;
    case CashFlowStatementWrongEnum::kRhsCredit:
        return entry->rhs_credit;
    case CashFlowStatementWrongEnum::kRhsDebit:
        return entry->rhs_debit;
    case CashFlowStatementWrongEnum::kRhsNode:
        return entry->rhs_node;
    case CashFlowStatementWrongEnum::kCashKind:
        return std::to_underlying(entry->cash_kind);
    }
}

void CashFlowWrongModel::sort(int column, Qt::SortOrder order)
{
    const CashFlowStatementWrongEnum e_column { column };

    auto Compare = [e_column, order](const auto* lhs, const auto* rhs) -> bool {
        switch (e_column) {
        case CashFlowStatementWrongEnum::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::issued_time, order);
        case CashFlowStatementWrongEnum::kLhsNode:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::lhs_node, order);
        case CashFlowStatementWrongEnum::kLhsDebit:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::lhs_debit, order);
        case CashFlowStatementWrongEnum::kLhsCredit:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::lhs_credit, order);
        case CashFlowStatementWrongEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::description, order);
        case CashFlowStatementWrongEnum::kRhsCredit:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::rhs_credit, order);
        case CashFlowStatementWrongEnum::kRhsDebit:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::rhs_debit, order);
        case CashFlowStatementWrongEnum::kRhsNode:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::rhs_node, order);
        case CashFlowStatementWrongEnum::kCashKind:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementWrongRow::cash_kind, order);
        case CashFlowStatementWrongEnum::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
