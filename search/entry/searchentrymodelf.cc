#include "searchentrymodelf.h"

#include "enum/entryenum.h"

SearchEntryModelF::SearchEntryModelF(CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchEntryModel { info, tag_hash, parent }
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
    case FullEntryEnumF::kVersion:
        return d_entry->version;
    case FullEntryEnumF::kIssuedTime:
        return d_entry->issued_time;
    case FullEntryEnumF::kCode:
        return d_entry->code;
    case FullEntryEnumF::kTag:
        return d_entry->tag;
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
    case FullEntryEnumF::kCashKind:
        return std::to_underlying(d_entry->cash_kind);
    }
}

void SearchEntryModelF::sort(int column, Qt::SortOrder order)
{
    const FullEntryEnumF e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryF>(lhs) };
        auto* d_rhs { DerivedPtr<EntryF>(rhs) };

        switch (e_column) {
        case FullEntryEnumF::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &Entry::issued_time, order);
        case FullEntryEnumF::kCode:
            return utils::CompareMember(lhs, rhs, &Entry::code, order);
        case FullEntryEnumF::kLhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_node, order);
        case FullEntryEnumF::kLhsRate:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_rate, order);
        case FullEntryEnumF::kLhsDebit:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_debit, order);
        case FullEntryEnumF::kLhsCredit:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_credit, order);
        case FullEntryEnumF::kDescription:
            return utils::CompareMember(lhs, rhs, &Entry::description, order);
        case FullEntryEnumF::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case FullEntryEnumF::kTag:
            return utils::CompareMember(lhs, rhs, &Entry::tag, order);
        case FullEntryEnumF::kStatus:
            return utils::CompareMember(lhs, rhs, &Entry::status, order);
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
        case FullEntryEnumF::kVersion:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
