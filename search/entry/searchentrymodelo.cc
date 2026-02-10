#include "searchentrymodelo.h"

#include "enum/entryenum.h"
#include "global/entrypool.h"
#include "utils/compareutils.h"

SearchEntryModelO::SearchEntryModelO(CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchEntryModel { info, tag_hash, parent }
{
}

SearchEntryModelO::~SearchEntryModelO() { EntryPool::Instance().Recycle(entry_list_, info_.section); }

QVariant SearchEntryModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryO>(entry_list_.at(index.row())) };
    const EntryEnumO column { index.column() };

    switch (column) {
    case EntryEnumO::kId:
        return d_entry->id;
    case EntryEnumO::kUpdateBy:
        return d_entry->updated_by;
    case EntryEnumO::kUpdateTime:
        return d_entry->updated_time;
    case EntryEnumO::kCreateTime:
        return d_entry->created_time;
    case EntryEnumO::kCreateBy:
        return d_entry->created_by;
    case EntryEnumO::kVersion:
        return d_entry->version;
    case EntryEnumO::kUserId:
        return d_entry->user_id;
    case EntryEnumO::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumO::kUnitPrice:
        return d_entry->unit_price;
    case EntryEnumO::kDescription:
        return d_entry->description;
    case EntryEnumO::kExternalSku:
        return QVariant();
    case EntryEnumO::kRhsNode:
        return d_entry->rhs_node;
    case EntryEnumO::kCount:
        return d_entry->count;
    case EntryEnumO::kMeasure:
        return d_entry->measure;
    case EntryEnumO::kUnitDiscount:
        return d_entry->unit_discount;
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
    const EntryEnumO e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryO>(lhs) };
        auto* d_rhs { DerivedPtr<EntryO>(rhs) };

        switch (e_column) {
        case EntryEnumO::kLhsNode:
            return Utils::CompareMember(lhs, rhs, &Entry::lhs_node, order);
        case EntryEnumO::kRhsNode:
            return Utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case EntryEnumO::kDescription:
            return Utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumO::kCount:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::count, order);
        case EntryEnumO::kUnitPrice:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::unit_price, order);
        case EntryEnumO::kMeasure:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::measure, order);
        case EntryEnumO::kUnitDiscount:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::unit_discount, order);
        case EntryEnumO::kInitial:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::initial, order);
        case EntryEnumO::kFinal:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::final, order);
        case EntryEnumO::kDiscount:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::discount, order);
        case EntryEnumO::kId:
        case EntryEnumO::kUpdateBy:
        case EntryEnumO::kUpdateTime:
        case EntryEnumO::kCreateTime:
        case EntryEnumO::kCreateBy:
        case EntryEnumO::kVersion:
        case EntryEnumO::kUserId:
        case EntryEnumO::kExternalSku:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
