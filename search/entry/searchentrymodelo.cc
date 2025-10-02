#include "searchentrymodelo.h"

#include "component/enumclass.h"

SearchEntryModelO::SearchEntryModelO(CSectionInfo& info, QObject* parent)
    : SearchEntryModel { info, parent }
{
}

QVariant SearchEntryModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryO>(entry_list_.at(index.row())) };
    const EntryEnumO kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumO::kId:
        return d_entry->id;
    case EntryEnumO::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumO::kUnitPrice:
        return d_entry->unit_price;
    case EntryEnumO::kDescription:
        return d_entry->description;
    case EntryEnumO::kExternalSku:
        return d_entry->external_sku;
    case EntryEnumO::kRhsNode:
        return d_entry->rhs_node;
    case EntryEnumO::kCount:
        return d_entry->count;
    case EntryEnumO::kMeasure:
        return d_entry->measure;
    case EntryEnumO::kDiscountPrice:
        return d_entry->discount_price;
    case EntryEnumO::kInitial:
        return d_entry->initial;
    case EntryEnumO::kDiscount:
        return d_entry->discount;
    case EntryEnumO::kFinal:
        return d_entry->final;
    default:
        return QVariant();
    }
}

void SearchEntryModelO::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](const Entry* lhs, const Entry* rhs) -> bool {
        const EntryEnumO kColumn { column };

        auto* d_lhs = DerivedPtr<EntryO>(lhs);
        auto* d_rhs = DerivedPtr<EntryO>(rhs);

        switch (kColumn) {
        case EntryEnumO::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case EntryEnumO::kCount:
            return (order == Qt::AscendingOrder) ? (d_lhs->count < d_rhs->count) : (d_lhs->count > d_rhs->count);
        case EntryEnumO::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_price < d_rhs->unit_price) : (d_lhs->unit_price > d_rhs->unit_price);
        case EntryEnumO::kMeasure:
            return (order == Qt::AscendingOrder) ? (d_lhs->measure < d_rhs->measure) : (d_lhs->measure > d_rhs->measure);
        case EntryEnumO::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case EntryEnumO::kExternalSku:
            return (order == Qt::AscendingOrder) ? (d_lhs->external_sku < d_rhs->external_sku) : (d_lhs->external_sku > d_rhs->external_sku);
        case EntryEnumO::kDiscountPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->discount_price < d_rhs->discount_price) : (d_lhs->discount_price > d_rhs->discount_price);
        case EntryEnumO::kInitial:
            return (order == Qt::AscendingOrder) ? (d_lhs->initial < d_rhs->initial) : (d_lhs->initial > d_rhs->initial);
        case EntryEnumO::kFinal:
            return (order == Qt::AscendingOrder) ? (d_lhs->final < d_rhs->final) : (d_lhs->final > d_rhs->final);
        case EntryEnumO::kDiscount:
            return (order == Qt::AscendingOrder) ? (d_lhs->discount < d_rhs->discount) : (d_lhs->discount > d_rhs->discount);
        case EntryEnumO::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
