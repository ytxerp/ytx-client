#include "periodclosemodel.h"

#include "enum/entryenum.h"
#include "utils/templateutils.h"

namespace period_close {

Model::Model(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel(parent)
    , info_ { info }
{
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.full_entry_header.at(section);

    return QVariant();
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex Model::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int Model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int Model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.full_entry_header.size();
}

void Model::sort(int column, Qt::SortOrder order)
{
    const FullEntryEnumF e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { utils::DerivedPtr<EntryF>(lhs) };
        auto* d_rhs { utils::DerivedPtr<EntryF>(rhs) };

        switch (e_column) {
        case FullEntryEnumF::kLhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_node, order);
        case FullEntryEnumF::kLhsRate:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_rate, order);
        case FullEntryEnumF::kLhsDebit:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_debit, order);
        case FullEntryEnumF::kLhsCredit:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_credit, order);
        case FullEntryEnumF::kRhsCredit:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_credit, order);
        case FullEntryEnumF::kRhsDebit:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_debit, order);
        case FullEntryEnumF::kRhsRate:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_rate, order);
        case FullEntryEnumF::kRhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case FullEntryEnumF::kCashKind:
            return utils::CompareMember(d_lhs, d_rhs, &EntryF::cash_kind, order);
        case FullEntryEnumF::kId:
        case FullEntryEnumF::kIssuedTime:
        case FullEntryEnumF::kCode:
        case FullEntryEnumF::kDescription:
        case FullEntryEnumF::kDocument:
        case FullEntryEnumF::kTag:
        case FullEntryEnumF::kStatus:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const FullEntryEnumF column { index.column() };
    auto* entry { static_cast<EntryF*>(index.internalPointer()) };

    switch (column) {
    case FullEntryEnumF::kId:
        return entry->id;
    case FullEntryEnumF::kIssuedTime:
        return entry->issued_time;
    case FullEntryEnumF::kCode:
        return entry->code;
    case FullEntryEnumF::kTag:
        return entry->tag;
    case FullEntryEnumF::kLhsNode:
        return entry->lhs_node;
    case FullEntryEnumF::kLhsRate:
        return entry->lhs_rate;
    case FullEntryEnumF::kLhsDebit:
        return entry->lhs_debit;
    case FullEntryEnumF::kLhsCredit:
        return entry->lhs_credit;
    case FullEntryEnumF::kDescription:
        return entry->description;
    case FullEntryEnumF::kDocument:
        return entry->document;
    case FullEntryEnumF::kStatus:
        return entry->status;
    case FullEntryEnumF::kRhsCredit:
        return entry->rhs_credit;
    case FullEntryEnumF::kRhsDebit:
        return entry->rhs_debit;
    case FullEntryEnumF::kRhsRate:
        return entry->rhs_rate;
    case FullEntryEnumF::kRhsNode:
        return entry->rhs_node;
    case FullEntryEnumF::kCashKind:
        return std::to_underlying(entry->cash_kind);
    }
}

void Model::Rebuild(const QList<Entry*>& list)
{
    beginResetModel();
    list_ = list;
    endResetModel();
}
}