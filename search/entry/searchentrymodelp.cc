#include "searchentrymodelp.h"

#include "component/enumclass.h"

SearchEntryModelP::SearchEntryModelP(CSectionInfo& info, QObject* parent)
    : SearchEntryModel { info, parent }
{
}

QVariant SearchEntryModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryP>(entry_list_.at(index.row())) };
    const EntryEnumP kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumP::kId:
        return d_entry->id;
    case EntryEnumP::kIssuedTime:
        return d_entry->issued_time;
    case EntryEnumP::kCode:
        return d_entry->code;
    case EntryEnumP::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumP::kDescription:
        return d_entry->description;
    case EntryEnumP::kExternalSku:
        return d_entry->external_sku.isNull() ? QVariant() : d_entry->external_sku;
    case EntryEnumP::kDocument:
        return d_entry->document.isEmpty() ? QVariant() : d_entry->document.size();
    case EntryEnumP::kIsChecked:
        return d_entry->is_checked ? d_entry->is_checked : QVariant();
    case EntryEnumP::kUnitPrice:
        return d_entry->unit_price == 0 ? QVariant() : d_entry->unit_price;
    case EntryEnumP::kRhsNode:
        return d_entry->rhs_node;
    default:
        return QVariant();
    }
}

void SearchEntryModelP::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](const Entry* lhs, const Entry* rhs) -> bool {
        const EntryEnumP kColumn { column };

        auto* d_lhs = DerivedPtr<EntryP>(lhs);
        auto* d_rhs = DerivedPtr<EntryP>(rhs);

        switch (kColumn) {
        case EntryEnumP::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case EntryEnumP::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case EntryEnumP::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case EntryEnumP::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case EntryEnumP::kExternalSku:
            return (order == Qt::AscendingOrder) ? (d_lhs->external_sku < d_rhs->external_sku) : (d_lhs->external_sku > d_rhs->external_sku);
        case EntryEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case EntryEnumP::kIsChecked:
            return (order == Qt::AscendingOrder) ? (lhs->is_checked < rhs->is_checked) : (lhs->is_checked > rhs->is_checked);
        case EntryEnumP::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_price < d_rhs->unit_price) : (d_lhs->unit_price > d_rhs->unit_price);
        case EntryEnumP::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
