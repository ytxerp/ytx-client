#include "searchentrymodeli.h"

#include "component/enumclass.h"

SearchEntryModelI::SearchEntryModelI(CSectionInfo& info, QObject* parent)
    : SearchEntryModel { info, parent }
{
}

QVariant SearchEntryModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryI>(entry_list_.at(index.row())) };
    const FullEntryEnumI column { index.column() };

    switch (column) {
    case FullEntryEnumI::kId:
        return d_entry->id;
    case FullEntryEnumI::kIssuedTime:
        return d_entry->issued_time;
    case FullEntryEnumI::kCode:
        return d_entry->code;
    case FullEntryEnumI::kLhsNode:
        return d_entry->lhs_node;
    case FullEntryEnumI::kLhsDebit:
        return d_entry->lhs_debit;
    case FullEntryEnumI::kUnitCost:
        return d_entry->unit_cost;
    case FullEntryEnumI::kLhsCredit:
        return d_entry->lhs_credit;
    case FullEntryEnumI::kDescription:
        return d_entry->description;
    case FullEntryEnumI::kDocument:
        return d_entry->document;
    case FullEntryEnumI::kStatus:
        return d_entry->status;
    case FullEntryEnumI::kRhsCredit:
        return d_entry->rhs_credit;
    case FullEntryEnumI::kRhsDebit:
        return d_entry->rhs_debit;
    case FullEntryEnumI::kRhsNode:
        return d_entry->rhs_node;
    default:
        return QVariant();
    }
}

void SearchEntryModelI::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](const Entry* lhs, const Entry* rhs) -> bool {
        const FullEntryEnumI e_column { column };

        auto* d_lhs = DerivedPtr<EntryI>(lhs);
        auto* d_rhs = DerivedPtr<EntryI>(rhs);

        switch (e_column) {
        case FullEntryEnumI::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case FullEntryEnumI::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case FullEntryEnumI::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case FullEntryEnumI::kUnitCost:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_cost < d_rhs->unit_cost) : (d_lhs->unit_cost > d_rhs->unit_cost);
        case FullEntryEnumI::kLhsDebit:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_debit < d_rhs->lhs_debit) : (d_lhs->lhs_debit > d_rhs->lhs_debit);
        case FullEntryEnumI::kLhsCredit:
            return (order == Qt::AscendingOrder) ? (d_lhs->lhs_credit < d_rhs->lhs_credit) : (d_lhs->lhs_credit > d_rhs->lhs_credit);
        case FullEntryEnumI::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case FullEntryEnumI::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case FullEntryEnumI::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        case FullEntryEnumI::kRhsCredit:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_credit < d_rhs->rhs_credit) : (d_lhs->rhs_credit > d_rhs->rhs_credit);
        case FullEntryEnumI::kRhsDebit:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_debit < d_rhs->rhs_debit) : (d_lhs->rhs_debit > d_rhs->rhs_debit);
        case FullEntryEnumI::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
