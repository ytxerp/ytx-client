#include "searchentrymodelt.h"

#include "component/enumclass.h"

SearchEntryModelT::SearchEntryModelT(CSectionInfo& info, QObject* parent)
    : SearchEntryModel { info, parent }
{
}

QVariant SearchEntryModelT::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryT>(entry_list_.at(index.row())) };
    const FullEntryEnumT column { index.column() };

    switch (column) {
    case FullEntryEnumT::kId:
        return d_entry->id;
    case FullEntryEnumT::kIssuedTime:
        return d_entry->issued_time;
    case FullEntryEnumT::kCode:
        return d_entry->code;
    case FullEntryEnumT::kLhsNode:
        return d_entry->lhs_node;
    case FullEntryEnumT::kLhsDebit:
        return d_entry->lhs_debit;
    case FullEntryEnumT::kLhsRate:
        return d_entry->lhs_rate;
    case FullEntryEnumT::kRhsRate:
        return d_entry->rhs_rate;
    case FullEntryEnumT::kLhsCredit:
        return d_entry->lhs_credit;
    case FullEntryEnumT::kDescription:
        return d_entry->description;
    case FullEntryEnumT::kDocument:
        return d_entry->document;
    case FullEntryEnumT::kStatus:
        return d_entry->status;
    case FullEntryEnumT::kRhsCredit:
        return d_entry->rhs_credit;
    case FullEntryEnumT::kRhsDebit:
        return d_entry->rhs_debit;
    case FullEntryEnumT::kRhsNode:
        return d_entry->rhs_node;
    default:
        return QVariant();
    }
}

void SearchEntryModelT::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](const Entry* lhs, const Entry* rhs) -> bool {
        const FullEntryEnumT e_column { column };

        auto* d_lhs = DerivedPtr<EntryT>(lhs);
        auto* d_rhs = DerivedPtr<EntryT>(rhs);

        switch (e_column) {
        case FullEntryEnumT::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case FullEntryEnumT::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case FullEntryEnumT::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case FullEntryEnumT::kLhsRate:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_rate < d_rhs->lhs_rate) : (d_lhs->lhs_rate > d_rhs->lhs_rate);
        case FullEntryEnumT::kRhsRate:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_rate < d_rhs->rhs_rate) : (d_lhs->rhs_rate > d_rhs->rhs_rate);
        case FullEntryEnumT::kLhsDebit:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_debit < d_rhs->lhs_debit) : (d_lhs->lhs_debit > d_rhs->lhs_debit);
        case FullEntryEnumT::kLhsCredit:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_credit < d_rhs->lhs_credit) : (d_lhs->lhs_credit > d_rhs->lhs_credit);
        case FullEntryEnumT::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case FullEntryEnumT::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case FullEntryEnumT::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        case FullEntryEnumT::kRhsCredit:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_credit < d_rhs->rhs_credit) : (d_lhs->rhs_credit > d_rhs->rhs_credit);
        case FullEntryEnumT::kRhsDebit:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_debit < d_rhs->rhs_debit) : (d_lhs->rhs_debit > d_rhs->rhs_debit);
        case FullEntryEnumT::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
