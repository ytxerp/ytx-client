#include "searchentrymodelr.h"

#include "enum/entryenum.h"

namespace search {
EntryModelR::EntryModelR(Section section, const QStringList& header, const QHash<QUuid, tag::Row>& tag_hash, QObject* parent)
    : EntryModel { section, header, tag_hash, parent }
{
}

QVariant EntryModelR::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const EntryEnumR column { index.column() };
    auto* d_entry { static_cast<EntryR*>(index.internalPointer()) };

    switch (column) {
    case EntryEnumR::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumR::kDescription:
        return d_entry->description;
    case EntryEnumR::kTag:
        return d_entry->tag;
    case EntryEnumR::kInitial:
        return d_entry->initial;
    case EntryEnumR::kDebitNode:
        return d_entry->debit_node;
    case EntryEnumR::kDocument:
        return d_entry->document;
    case EntryEnumR::kIssuedTime:
        return d_entry->issued_time;
    case EntryEnumR::kCreditNode:
        return d_entry->credit_node;
    }
}

void EntryModelR::sort(int column, Qt::SortOrder order)
{
    const EntryEnumR e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryR>(lhs) };
        auto* d_rhs { DerivedPtr<EntryR>(rhs) };

        switch (e_column) {
        case EntryEnumR::kLhsNode:
            return utils::CompareMember(lhs, rhs, &EntryR::lhs_node, order);
        case EntryEnumR::kDescription:
            return utils::CompareMember(lhs, rhs, &EntryR::description, order);
        case EntryEnumR::kInitial:
            return utils::CompareMember(d_lhs, d_rhs, &EntryR::initial, order);
        case EntryEnumR::kTag:
            return utils::CompareMember(lhs, rhs, &EntryR::tag, order);
        case EntryEnumR::kDocument:
            return utils::CompareMember(lhs, rhs, &EntryR::document, order);
        case EntryEnumR::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &EntryR::issued_time, order);
        case EntryEnumR::kDebitNode:
            return utils::CompareMember(d_lhs, d_rhs, &EntryR::debit_node, order);
        case EntryEnumR::kCreditNode:
            return utils::CompareMember(d_lhs, d_rhs, &EntryR::credit_node, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}
}
