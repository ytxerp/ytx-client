#include "searchentrymodels.h"

#include "component/enumclass.h"

SearchEntryModelS::SearchEntryModelS(CSectionInfo& info, QObject* parent)
    : SearchEntryModel { info, parent }
{
}

QVariant SearchEntryModelS::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryS>(entry_list_.at(index.row())) };
    const EntryEnumS kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumS::kId:
        return d_entry->id;
    case EntryEnumS::kIssuedTime:
        return d_entry->issued_time;
    case EntryEnumS::kCode:
        return d_entry->code;
    case EntryEnumS::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumS::kDescription:
        return d_entry->description;
    case EntryEnumS::kExternalItem:
        return d_entry->external_item.isNull() ? QVariant() : d_entry->external_item;
    case EntryEnumS::kDocument:
        return d_entry->document.isEmpty() ? QVariant() : d_entry->document.size();
    case EntryEnumS::kIsChecked:
        return d_entry->is_checked ? d_entry->is_checked : QVariant();
    case EntryEnumS::kUnitPrice:
        return d_entry->unit_price == 0 ? QVariant() : d_entry->unit_price;
    case EntryEnumS::kRhsNode:
        return d_entry->rhs_node;
    default:
        return QVariant();
    }
}

void SearchEntryModelS::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](const Entry* lhs, const Entry* rhs) -> bool {
        const EntryEnumS kColumn { column };

        auto* d_lhs = DerivedPtr<EntryS>(lhs);
        auto* d_rhs = DerivedPtr<EntryS>(rhs);

        switch (kColumn) {
        case EntryEnumS::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case EntryEnumS::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case EntryEnumS::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case EntryEnumS::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case EntryEnumS::kExternalItem:
            return (order == Qt::AscendingOrder) ? (d_lhs->external_item < d_rhs->external_item) : (d_lhs->external_item > d_rhs->external_item);
        case EntryEnumS::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case EntryEnumS::kIsChecked:
            return (order == Qt::AscendingOrder) ? (lhs->is_checked < rhs->is_checked) : (lhs->is_checked > rhs->is_checked);
        case EntryEnumS::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_price < d_rhs->unit_price) : (d_lhs->unit_price > d_rhs->unit_price);
        case EntryEnumS::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
