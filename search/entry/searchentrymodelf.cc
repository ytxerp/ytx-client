#include "searchentrymodelf.h"

#include "enum/entryenum.h"

SearchEntryModelF::SearchEntryModelF(CSectionInfo& info, QObject* parent)
    : SearchEntryModel { info, parent }
{
}

QVariant SearchEntryModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryF>(entry_list_.at(index.row())) };
    const FullEntryEnumF column { index.column() };

    switch (column) {
    case FullEntryEnumF::kId:
        return d_entry->id;
    case FullEntryEnumF::kIssuedTime:
        return d_entry->issued_time;
    case FullEntryEnumF::kCode:
        return d_entry->code;
    case FullEntryEnumF::kLhsNode:
        return d_entry->lhs_node;
    case FullEntryEnumF::kLhsRate:
        return d_entry->lhs_rate;
    case FullEntryEnumF::kLhsDebit:
        return d_entry->lhs_debit;
    case FullEntryEnumF::kLhsCredit:
        return d_entry->lhs_credit;
    case FullEntryEnumF::kDescription:
        return d_entry->description;
    case FullEntryEnumF::kDocument:
        return d_entry->document;
    case FullEntryEnumF::kStatus:
        return d_entry->status;
    case FullEntryEnumF::kRhsCredit:
        return d_entry->rhs_credit;
    case FullEntryEnumF::kRhsDebit:
        return d_entry->rhs_debit;
    case FullEntryEnumF::kRhsRate:
        return d_entry->rhs_rate;
    case FullEntryEnumF::kRhsNode:
        return d_entry->rhs_node;
    default:
        return QVariant();
    }
}

void SearchEntryModelF::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](const Entry* lhs, const Entry* rhs) -> bool {
        const FullEntryEnumF e_column { column };

        auto* d_lhs = DerivedPtr<EntryF>(lhs);
        auto* d_rhs = DerivedPtr<EntryF>(rhs);

        switch (e_column) {
        case FullEntryEnumF::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case FullEntryEnumF::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case FullEntryEnumF::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case FullEntryEnumF::kLhsRate:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_rate < d_rhs->lhs_rate) : (d_lhs->lhs_rate > d_rhs->lhs_rate);
        case FullEntryEnumF::kLhsDebit:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_debit < d_rhs->lhs_debit) : (d_lhs->lhs_debit > d_rhs->lhs_debit);
        case FullEntryEnumF::kLhsCredit:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_credit < d_rhs->lhs_credit) : (d_lhs->lhs_credit > d_rhs->lhs_credit);
        case FullEntryEnumF::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case FullEntryEnumF::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case FullEntryEnumF::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        case FullEntryEnumF::kRhsCredit:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_credit < d_rhs->rhs_credit) : (d_lhs->rhs_credit > d_rhs->rhs_credit);
        case FullEntryEnumF::kRhsDebit:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_debit < d_rhs->rhs_debit) : (d_lhs->rhs_debit > d_rhs->rhs_debit);
        case FullEntryEnumF::kRhsRate:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_rate < d_rhs->rhs_rate) : (d_lhs->rhs_rate > d_rhs->rhs_rate);
        case FullEntryEnumF::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
