#include "searchentrymodelp.h"

#include "enum/entryenum.h"
#include "utils/compareutils.h"

SearchEntryModelP::SearchEntryModelP(EntryHub* entry_hub, CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchEntryModel { info, tag_hash, parent }
    , entry_hub_p_ { static_cast<EntryHubP*>(entry_hub) }
{
}

QVariant SearchEntryModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry { DerivedPtr<EntryP>(entry_list_.at(index.row())) };
    const EntryEnumP column { index.column() };

    switch (column) {
    case EntryEnumP::kId:
        return d_entry->id;
    case EntryEnumP::kUpdateBy:
        return d_entry->updated_by;
    case EntryEnumP::kUpdateTime:
        return d_entry->updated_time;
    case EntryEnumP::kCreateTime:
        return d_entry->created_time;
    case EntryEnumP::kCreateBy:
        return d_entry->created_by;
    case EntryEnumP::kVersion:
        return d_entry->version;
    case EntryEnumP::kUserId:
        return d_entry->user_id;
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
    default:
        return QVariant();
    }
}

void SearchEntryModelP::sort(int column, Qt::SortOrder order)
{
    const EntryEnumP e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryP>(lhs) };
        auto* d_rhs { DerivedPtr<EntryP>(rhs) };

        switch (e_column) {
        case EntryEnumP::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &Entry::issued_time, order);
        case EntryEnumP::kCode:
            return Utils::CompareMember(lhs, rhs, &Entry::code, order);
        case EntryEnumP::kLhsNode:
            return Utils::CompareMember(lhs, rhs, &Entry::lhs_node, order);
        case EntryEnumP::kDescription:
            return Utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumP::kExternalSku:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryP::external_sku, order);
        case EntryEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case EntryEnumP::kStatus:
            return Utils::CompareMember(lhs, rhs, &Entry::status, order);
        case EntryEnumP::kTag:
            return Utils::CompareMember(lhs, rhs, &Entry::tag, order);
        case EntryEnumP::kUnitPrice:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryP::unit_price, order);
        case EntryEnumP::kRhsNode:
            return Utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case EntryEnumP::kId:
        case EntryEnumP::kUpdateBy:
        case EntryEnumP::kUpdateTime:
        case EntryEnumP::kCreateTime:
        case EntryEnumP::kCreateBy:
        case EntryEnumP::kVersion:
        case EntryEnumP::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}

void SearchEntryModelP::Search(const QString& text)
{
    // 1. Prepare the result list (do not modify the model yet)
    QList<Entry*> results {};

    if (!text.isEmpty()) {
        // Parse search input into text and tag set
        const SearchQuery query { ParseSearchQuery(text, tag_hash_) };

        if (!query.tags.isEmpty()) {
            // Tag search has higher priority
            entry_hub_p_->SearchTag(results, query.tags);
        } else if (!query.text.isEmpty()) {
            // Search by description text if no tags
            entry_hub_p_->SearchDescription(results, query.text);
        }
    }

    // 2. Update the model in one step
    beginResetModel();
    entry_list_ = std::move(results); // Move results to avoid copying
    endResetModel();
}
