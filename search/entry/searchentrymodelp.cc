#include "searchentrymodelp.h"

#include "enum/entryenum.h"

namespace search {

EntryModelP::EntryModelP(Section section, const QStringList& header, const QHash<QUuid, tag::Row>& tag_hash, QObject* parent)
    : EntryModel { section, header, tag_hash, parent }
{
}

QVariant EntryModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const EntryEnumP column { index.column() };
    auto* d_entry { static_cast<EntryP*>(index.internalPointer()) };

    switch (column) {
    case EntryEnumP::kIssuedTime:
        return d_entry->issued_time;
    case EntryEnumP::kTag:
        return d_entry->tag;
    case EntryEnumP::kCode:
        return d_entry->code;
    case EntryEnumP::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumP::kDescription:
        return d_entry->description;
    case EntryEnumP::kExternalSku:
        return d_entry->external_sku;
    case EntryEnumP::kDocument:
        return d_entry->document;
    case EntryEnumP::kStatus:
        return d_entry->status;
    case EntryEnumP::kUnitPrice:
        return d_entry->unit_price;
    case EntryEnumP::kRhsNode:
        return d_entry->rhs_node;
    case EntryEnumP::kIsLocked:
        return d_entry->is_locked;
    }
}

void EntryModelP::sort(int column, Qt::SortOrder order)
{
    const EntryEnumP e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryP>(lhs) };
        auto* d_rhs { DerivedPtr<EntryP>(rhs) };

        switch (e_column) {
        case EntryEnumP::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &Entry::issued_time, order);
        case EntryEnumP::kCode:
            return utils::CompareMember(lhs, rhs, &Entry::code, order);
        case EntryEnumP::kLhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_node, order);
        case EntryEnumP::kDescription:
            return utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumP::kExternalSku:
            return utils::CompareMember(d_lhs, d_rhs, &EntryP::external_sku, order);
        case EntryEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case EntryEnumP::kStatus:
            return utils::CompareMember(lhs, rhs, &Entry::status, order);
        case EntryEnumP::kTag:
            return utils::CompareMember(lhs, rhs, &Entry::tag, order);
        case EntryEnumP::kUnitPrice:
            return utils::CompareMember(d_lhs, d_rhs, &EntryP::unit_price, order);
        case EntryEnumP::kRhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case EntryEnumP::kIsLocked:
            return utils::CompareMember(lhs, rhs, &Entry::is_locked, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}

}
