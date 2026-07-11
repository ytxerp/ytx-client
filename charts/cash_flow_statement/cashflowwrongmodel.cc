#include "cashflowwrongmodel.h"

#include <QtCore/qjsonarray.h>

#include "cashflowstatementenum.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"
#include "websocket/jsongen.h"

namespace cash_flow {

WrongModel::WrongModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
{
}

WrongModel::~WrongModel() { }

QModelIndex WrongModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex WrongModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int WrongModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int WrongModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant WrongModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void WrongModel::Rebuild(CJsonArray& node_array)
{
    const auto list { AddRowsList(node_array) };

    beginResetModel();
    list_ = list;
    endResetModel();
}

QList<WrongRow*> WrongModel::AddRowsList(const CJsonArray& node_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    QList<WrongRow*> list {};

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        auto* node { ResourcePool<WrongRow>::Instance().Allocate() };
        node->ReadJson(obj);
        list.emplaceBack(node);
    }

    return list;
}

QVariant WrongModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const WrongRowField column { index.column() };
    auto* entry { static_cast<WrongRow*>(index.internalPointer()) };

    switch (column) {
    case WrongRowField::kId:
        return entry->id;
    case WrongRowField::kIssuedTime:
        return entry->issued_time;
    case WrongRowField::kLhsNode:
        return entry->lhs_node;
    case WrongRowField::kLhsDebit:
        return entry->lhs_debit;
    case WrongRowField::kLhsCredit:
        return entry->lhs_credit;
    case WrongRowField::kDescription:
        return entry->description;
    case WrongRowField::kRhsCredit:
        return entry->rhs_credit;
    case WrongRowField::kRhsDebit:
        return entry->rhs_debit;
    case WrongRowField::kRhsNode:
        return entry->rhs_node;
    case WrongRowField::kCashKind:
        return std::to_underlying(entry->cash_kind);
    }
}

void WrongModel::sort(int column, Qt::SortOrder order)
{
    const WrongRowField e_column { column };

    auto Compare = [e_column, order](const auto* lhs, const auto* rhs) -> bool {
        switch (e_column) {
        case WrongRowField::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &WrongRow::issued_time, order);
        case WrongRowField::kLhsNode:
            return utils::CompareMember(lhs, rhs, &WrongRow::lhs_node, order);
        case WrongRowField::kLhsDebit:
            return utils::CompareMember(lhs, rhs, &WrongRow::lhs_debit, order);
        case WrongRowField::kLhsCredit:
            return utils::CompareMember(lhs, rhs, &WrongRow::lhs_credit, order);
        case WrongRowField::kDescription:
            return utils::CompareMember(lhs, rhs, &WrongRow::description, order);
        case WrongRowField::kRhsCredit:
            return utils::CompareMember(lhs, rhs, &WrongRow::rhs_credit, order);
        case WrongRowField::kRhsDebit:
            return utils::CompareMember(lhs, rhs, &WrongRow::rhs_debit, order);
        case WrongRowField::kRhsNode:
            return utils::CompareMember(lhs, rhs, &WrongRow::rhs_node, order);
        case WrongRowField::kCashKind:
            return utils::CompareMember(lhs, rhs, &WrongRow::cash_kind, order);
        case WrongRowField::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}
}
